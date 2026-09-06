#!/usr/bin/env bash
# Cross-compile static freetype + harfbuzz for the Android ABIs using the NDK
# toolchain. Output goes to third_party/prefix/<name>/<abi>/ (libs + headers),
# on the host mount so it persists between containers.
#
# Usage:
#   bash build-libs.sh            # build whatever is missing (called by build.sh)
#   bash build-libs.sh freetype   # only freetype
#   bash build-libs.sh harfbuzz arm64-v8a   # one lib, one ABI
#
# Incremental: a lib/ABI is skipped when its .a already exists. To force a
# rebuild (e.g. after changing flags here), delete the prefix dir or source:
#   rm -rf third_party/prefix/<name>  third_party/build/<name>-*
#
# Sources/versions (pinned):
#   freetype  2.13.3  https://github.com/freetype/freetype/tree/VER-2-13-3
#   harfbuzz 14.4.0   https://github.com/harfbuzz/harfbuzz/tree/14.4.0
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Environment (present inside the Docker image)
[ -f /etc/profile.d/android-env.sh ] && source /etc/profile.d/android-env.sh
: "${ANDROID_NDK_HOME:?ANDROID_NDK_HOME not set (run inside the container or export it)}"

ABIS="${ABIS:-arm64-v8a armeabi-v7a}"
TOOLCHAIN="${ANDROID_NDK_HOME}/build/cmake/android.toolchain.cmake"
TP="${SCRIPT_DIR}/third_party"
SRC_ROOT="${TP}/src"
PREFIX_ROOT="${TP}/prefix"
BUILD_ROOT="${TP}/build"
FT_SRC="${SRC_ROOT}/freetype-2.13.3"
HB_SRC="${SRC_ROOT}/harfbuzz-14.4.0"

# --- freetype: fontstash.h #errors unless subpixel rendering is enabled.
# Desktop distro freetype builds enable it; enable it here for parity (and so
# the ClearType code path stays available). Idempotent.
FTOPT="${FT_SRC}/include/freetype/config/ftoption.h"
if grep -q '/\* #define FT_CONFIG_OPTION_SUBPIXEL_RENDERING \*/' "${FTOPT}"; then
    echo "==> Enabling FT_CONFIG_OPTION_SUBPIXEL_RENDERING in ${FTOPT}"
    sed -i 's|/\* #define FT_CONFIG_OPTION_SUBPIXEL_RENDERING \*/|#define FT_CONFIG_OPTION_SUBPIXEL_RENDERING|' "${FTOPT}"
fi

build_lib()
{
    # $1 = name, $2 = source dir, rest = extra cmake args
    local name="$1"
    local src="$2"
    shift 2

    for abi in ${ABIS}; do
        local prefix="${PREFIX_ROOT}/${name}/${abi}"
        local lib="${prefix}/lib/lib${name}.a"

        if [ -f "${lib}" ]; then
            echo "==> ${name} ${abi}: already built (${lib}); delete third_party/prefix/${name} to rebuild"
            continue
        fi

        echo "==> Configuring ${name} (${abi})"
        cmake -S "${src}" -B "${BUILD_ROOT}/${name}-${abi}" \
            -DCMAKE_TOOLCHAIN_FILE="${TOOLCHAIN}" \
            -DANDROID_ABI="${abi}" \
            -DANDROID_PLATFORM=android-21 \
            -DCMAKE_BUILD_TYPE=Release \
            -DCMAKE_INSTALL_PREFIX="${prefix}" \
            -DBUILD_SHARED_LIBS=OFF \
            "$@" \
            -G Ninja

        echo "==> Building ${name} (${abi})"
        cmake --build "${BUILD_ROOT}/${name}-${abi}"

        echo "==> Installing ${name} (${abi}) -> ${prefix}"
        cmake --install "${BUILD_ROOT}/${name}-${abi}"
    done
}

# Filter args: optional [name] [abi]
if [ "$#" -ge 1 ]; then
    case "$1" in
        freetype|harfbuzz) ;;
        *) echo "Unknown lib '$1' (want freetype or harfbuzz)" >&2; exit 1 ;;
    esac
    [ "$#" -ge 2 ] && ABIS="$2"
    [ "$#" -gt 2 ] && { echo "Too many args" >&2; exit 1; }
    [ "$1" = freetype ] && build_lib freetype "${FT_SRC}" \
        -DFT_DISABLE_ZLIB=TRUE -DFT_DISABLE_BZIP2=TRUE -DFT_DISABLE_PNG=TRUE \
        -DFT_DISABLE_HARFBUZZ=TRUE -DFT_DISABLE_BROTLI=TRUE
    [ "$1" = harfbuzz ] && build_lib harfbuzz "${HB_SRC}" \
        -DHB_BUILD_UTILS=OFF -DHB_BUILD_TESTS=OFF -DHB_BUILD_BENCHMARK=OFF \
        -DHB_BUILD_SUBSET=OFF -DHB_BUILD_RASTER=OFF -DHB_BUILD_VECTOR=OFF -DHB_BUILD_GPU=OFF \
        -DHB_HAVE_FREETYPE=OFF -DHB_HAVE_GLIB=OFF -DHB_HAVE_ICU=OFF \
        -DHB_HAVE_GRAPHITE2=OFF -DHB_HAVE_GOBJECT=OFF -DCMAKE_CXX_STANDARD=20
    exit 0
fi

# Full build (default): freetype first, then harfbuzz, for all ABIs.
build_lib freetype "${FT_SRC}" \
    -DFT_DISABLE_ZLIB=TRUE -DFT_DISABLE_BZIP2=TRUE -DFT_DISABLE_PNG=TRUE \
    -DFT_DISABLE_HARFBUZZ=TRUE -DFT_DISABLE_BROTLI=TRUE
build_lib harfbuzz "${HB_SRC}" \
    -DHB_BUILD_UTILS=OFF -DHB_BUILD_TESTS=OFF -DHB_BUILD_BENCHMARK=OFF \
    -DHB_BUILD_SUBSET=OFF -DHB_BUILD_RASTER=OFF -DHB_BUILD_VECTOR=OFF -DHB_BUILD_GPU=OFF \
    -DHB_HAVE_FREETYPE=OFF -DHB_HAVE_GLIB=OFF -DHB_HAVE_ICU=OFF \
    -DHB_HAVE_GRAPHITE2=OFF -DHB_HAVE_GOBJECT=OFF -DCMAKE_CXX_STANDARD=20

echo "==> Done. Libs in ${PREFIX_ROOT}"
ls -la "${PREFIX_ROOT}"/freetype/*/lib/libfreetype.a "${PREFIX_ROOT}"/harfbuzz/*/lib/libharfbuzz.a
