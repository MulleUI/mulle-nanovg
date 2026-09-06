#!/usr/bin/env bash
# Emulator orchestration for the mulle-nanovg demo. Runs inside the container.
#
#   run.sh emulator     create AVD (if needed) and start the headless emulator (long-running)
#   run.sh boot-wait    wait until the emulator has fully booted
#   run.sh install      install the APK and launch the activity
#   run.sh logcat       tail logcat filtered to the demo
#   run.sh all          emulator (backgrounded) + boot-wait + install + logcat
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
[ -f /etc/profile.d/android-env.sh ] && source /etc/profile.d/android-env.sh
: "${ANDROID_HOME:?ANDROID_HOME not set (run inside the container or export it)}"

AVD_NAME="${AVD_NAME:-nanovg}"
SYS_IMAGE="system-images;android-34;google_apis;x86_64"
APK="${SCRIPT_DIR}/build/app-debug.apk"
PACKAGE="com.mulle.nanovg.demo"
ACTIVITY="android.app.NativeActivity"

ensure_avd() {
    if ! "${ANDROID_HOME}/cmdline-tools/latest/bin/avdmanager" list avd 2>/dev/null | grep -q "Name: ${AVD_NAME}"; then
        echo "==> Creating AVD '${AVD_NAME}'"
        echo no | "${ANDROID_HOME}/cmdline-tools/latest/bin/avdmanager" create avd \
            -n "${AVD_NAME}" -k "${SYS_IMAGE}" --force >/dev/null
    fi
}

cmd_emulator() {
    ensure_avd
    echo "==> Starting emulator '${AVD_NAME}' (headless, swiftshader)"
    exec "${ANDROID_HOME}/emulator/emulator" \
        -avd "${AVD_NAME}" \
        -no-window -gpu swiftshader_indirect -no-audio -no-boot-anim -no-snapshot \
        -accel on
}

cmd_boot_wait() {
    echo "==> Waiting for device"
    adb wait-for-device
    echo "==> Waiting for boot completion"
    until [ "$(adb shell getprop sys.boot_completed 2>/dev/null | tr -d '\r')" = "1" ]; do
        sleep 2
    done
    echo "==> Booted: $(adb shell getprop ro.build.version.release | tr -d '\r') (API $(adb shell getprop ro.build.version.sdk | tr -d '\r'))"
}

cmd_install() {
    [ -f "${APK}" ] || { echo "APK not found: ${APK} (run build.sh first)"; exit 1; }
    echo "==> Installing ${APK}"
    adb install -r "${APK}"
    echo "==> Launching ${PACKAGE}"
    adb shell am start -n "${PACKAGE}/${ACTIVITY}"
}

cmd_logcat() {
    adb logcat -s nanovg
}

cmd_all() {
    ensure_avd
    nohup "${ANDROID_HOME}/emulator/emulator" \
        -avd "${AVD_NAME}" \
        -no-window -gpu swiftshader_indirect -no-audio -no-boot-anim -no-snapshot \
        -accel on > /tmp/emulator.log 2>&1 &
    cmd_boot_wait
    cmd_install
    cmd_logcat
}

case "${1:-all}" in
    emulator)  cmd_emulator ;;
    boot-wait) cmd_boot_wait ;;
    install)   cmd_install ;;
    logcat)    cmd_logcat ;;
    all)       cmd_all ;;
    *) echo "usage: $0 {emulator|boot-wait|install|logcat|all}"; exit 1 ;;
esac