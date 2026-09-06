#!/usr/bin/env bash
# Build the Android image (if needed) and drop into a container with
# KVM (for the emulator) and USB (for physical devices) passthrough.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"

IMAGE="${IMAGE:-nanovg-android}"

docker build -t "${IMAGE}" "${SCRIPT_DIR}"

# --device /dev/kvm : hardware-accelerated emulator (KVM_OK verified on host)
# --device /dev/bus/usb : physical device access via adb
exec docker run -it --rm \
    --name nanovg-android \
    --device /dev/kvm \
    --device /dev/bus/usb \
    -e KVM=1 \
    -v "${REPO_ROOT}":/workspace \
    -w /workspace \
    "${IMAGE}" \
    bash -lc 'source /etc/profile.d/android-env.sh && exec bash'