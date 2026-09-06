#!/usr/bin/env bash
# Build the Android APK: cross-compile lib+app with the NDK toolchain for one
# or more ABIs, package with aapt2, align and sign. Designed to run inside the
# container (docker-run.sh) but also works standalone if ANDROID_HOME/NDK set.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Environment (present inside the Docker image)
[ -f /etc/profile.d/android-env.sh ] && source /etc/profile.d/android-env.sh
: "${ANDROID_HOME:?ANDROID_HOME not set (run inside the container or export it)}"
: "${ANDROID_NDK_HOME:?ANDROID_NDK_HOME not set (run inside the container or export it)}"

ABIS="${ABIS:-arm64-v8a armeabi-v7a}"

# Cross-compile freetype + harfbuzz first (skips ABIs already built)
echo "==> Building third-party libs (freetype + harfbuzz) if needed"
bash "${SCRIPT_DIR}/build-libs.sh"

BUILD_TOOLS="${ANDROID_HOME}/build-tools/34.0.0"
PLATFORM_JAR="${ANDROID_HOME}/platforms/android-34/android.jar"
PACKAGE="com.mulle.nanovg.demo"
LIB_NAME="nanovg-demo"
OUT="${SCRIPT_DIR}/build"
APK_DIR="${OUT}/apk"
KEYSTORE="${SCRIPT_DIR}/debug.keystore"

echo "==> Cross-compiling for ABIs: ${ABIS}"
rm -rf "${APK_DIR}"
mkdir -p "${APK_DIR}/lib"

for ABI in ${ABIS}; do
    echo "==> Configuring CMake (${ABI})"
    cmake -S "${SCRIPT_DIR}/jni" -B "${OUT}/cmake-${ABI}" \
        -DCMAKE_TOOLCHAIN_FILE="${ANDROID_NDK_HOME}/build/cmake/android.toolchain.cmake" \
        -DANDROID_ABI="${ABI}" \
        -DANDROID_PLATFORM=android-21 \
        -DCMAKE_BUILD_TYPE=Release \
        -DFREETYPE_DIR="${SCRIPT_DIR}/third_party/prefix/freetype/${ABI}" \
        -DHARFBUZZ_DIR="${SCRIPT_DIR}/third_party/prefix/harfbuzz/${ABI}" \
        -G Ninja

    echo "==> Building (${ABI})"
    cmake --build "${OUT}/cmake-${ABI}"

    mkdir -p "${APK_DIR}/lib/${ABI}"
    cp "${OUT}/cmake-${ABI}/lib${LIB_NAME}.so" "${APK_DIR}/lib/${ABI}/"
done

echo "==> Staging assets"
rm -rf "${SCRIPT_DIR}/assets"
mkdir -p "${SCRIPT_DIR}/assets/images"
cp "${SCRIPT_DIR}/../demo/images/"*.jpg "${SCRIPT_DIR}/assets/images/"
cp "${SCRIPT_DIR}/../demo/entypo.ttf" \
   "${SCRIPT_DIR}/../demo/Roboto-Regular.ttf" \
   "${SCRIPT_DIR}/../demo/Roboto-Bold.ttf" \
   "${SCRIPT_DIR}/../demo/NotoEmoji-Regular.ttf" \
   "${SCRIPT_DIR}/assets/"

echo "==> Packaging resources"
rm -f "${OUT}/res.zip" "${OUT}/app-unsigned.apk"
"${BUILD_TOOLS}/aapt2" compile --dir "${SCRIPT_DIR}/res" -o "${OUT}/res.zip"
"${BUILD_TOOLS}/aapt2" link \
    -o "${OUT}/app-unsigned.apk" \
    -I "${PLATFORM_JAR}" \
    --manifest "${SCRIPT_DIR}/AndroidManifest.xml" \
    -A "${SCRIPT_DIR}/assets" \
    "${OUT}/res.zip"

echo "==> Adding native libraries"
( cd "${APK_DIR}" && zip -qr "${OUT}/app-unsigned.apk" lib )

echo "==> Aligning and signing"
if [ ! -f "${KEYSTORE}" ]; then
    echo "==> Generating debug keystore"
    keytool -genkeypair -keystore "${KEYSTORE}" -alias androiddebugkey \
        -keyalg RSA -keysize 2048 -validity 10000 \
        -storepass android -keypass android \
        -dname "CN=Android Debug,O=Android,C=US"
fi
"${BUILD_TOOLS}/zipalign" -f 4 "${OUT}/app-unsigned.apk" "${OUT}/app-aligned.apk"
"${BUILD_TOOLS}/apksigner" sign \
    --ks "${KEYSTORE}" --ks-pass pass:android --key-pass pass:android \
    --out "${OUT}/app-debug.apk" "${OUT}/app-aligned.apk"

echo "==> Done: ${OUT}/app-debug.apk"
"${BUILD_TOOLS}/apksigner" verify "${OUT}/app-debug.apk"
unzip -l "${OUT}/app-debug.apk" | grep -E "lib/|assets/" | head -20