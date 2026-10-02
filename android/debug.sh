#!/usr/bin/env bash
# Attach LLDB (from the NDK) to the running demo on the emulator.
#
# Why x86_64? The emulator is an x86_64 image running the arm64 build under
# ARM translation; lldb-server (and ptrace) from translated ARM code SIGSEGVs,
# so the debuggable build is the x86_64 ABI, which runs natively. Real-device
# debugging would use the device's ABI instead.
#
# Requires a running, booted emulator (run.sh emulator) and the app installed.
# Run inside the container (docker-run.sh).
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
[ -f /etc/profile.d/android-env.sh ] && source /etc/profile.d/android-env.sh
: "${ANDROID_HOME:?ANDROID_HOME not set (run inside the container or export it)}"
: "${ANDROID_NDK_HOME:?ANDROID_NDK_HOME not set}"

PACKAGE="com.mulle.nanovg.demo"
ACTIVITY="android.app.NativeActivity"
ABI="${DEBUG_ABI:-x86_64}"                     # native ABI on this emulator
SOCK="/data/local/tmp/nanovg-debug.sock"       # device-side platform socket
LLDB_ABI_DIR="${ANDROID_NDK_HOME}/toolchains/llvm/prebuilt/linux-x86_64"

# --- 1. Build + install a native (x86_64) debuggable APK -------------------
echo "==> Building ${ABI} debug APK"
( cd "${SCRIPT_DIR}" && ABIS="${ABI}" ./build.sh >/dev/null )
cd "${SCRIPT_DIR}"
echo "==> Installing + relaunching"
adb install -r build/app-debug.apk >/dev/null
adb shell am force-stop "${PACKAGE}" >/dev/null 2>&1 || true
adb shell am start -n "${PACKAGE}/${ACTIVITY}" >/dev/null
sleep 3
PID="$(adb shell pidof ${PACKAGE} | tr -d '\r')"
echo "==> App PID: ${PID} (primaryCpuAbi=$(adb shell pm dump ${PACKAGE} 2>/dev/null | grep -m1 primaryCpuAbi | tr -d '\r'))"

# --- 2. Install native lldb-server where the platform spawns it ------------
LS="${LLDB_ABI_DIR}/lib/clang/21/lib/linux/${ABI}/lldb-server"
echo "==> Pushing lldb-server (${ABI})"
adb push "${LS}" /data/local/tmp/lldb-server >/dev/null
adb shell chmod 755 /data/local/tmp/lldb-server

# --- 3. Start the platform server ------------------------------------------
echo "==> Starting platform server"
adb shell "pkill -f lldb-server" 2>/dev/null || true
sleep 1
adb shell "nohup /data/local/tmp/lldb-server platform --server \
    --listen unix-abstract://${SOCK} >/dev/null 2>&1 &"

# --- 4. Attach LLDB --------------------------------------------------------
export LD_LIBRARY_PATH="${LLDB_ABI_DIR}/lib:${LLDB_ABI_DIR}/python3/lib"
export PYTHONHOME="${LLDB_ABI_DIR}/python3"
export PYTHONPATH="${LLDB_ABI_DIR}/python3/lib/python3.11"

echo "==> Launching LLDB (attach to PID ${PID}). Try: bt, breakpoint set -n renderDemo, continue"
exec "${LLDB_ABI_DIR}/bin/lldb" \
    -o "platform select remote-android" \
    -o "platform connect unix-abstract-connect://${SOCK}" \
    -o "process attach --pid ${PID}"
