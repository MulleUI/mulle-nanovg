# mulle-nanovg Android build environment
# SDK + NDK + emulator for building, running, and debugging the demo on Android.
# Versions verified against `sdkmanager --list` on 2026-09-06.
#
# Each SDK component is installed in its own RUN layer: if a build is
# interrupted (foreground builds have time limits here), re-running resumes
# from the last completed layer via Docker's cache.
FROM ubuntu:24.04

ARG NDK_VERSION=ndk;29.0.14206865
ARG SYS_IMAGE=system-images;android-34;google_apis;x86_64
ARG BUILD_TOOLS=34.0.0
ARG PLATFORM=android-34

# Layer 1: base tools
# (X11/GL/audio libs are needed by the emulator binary even in headless mode)
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
        openjdk-17-jdk-headless \
        cmake \
        ninja-build \
        wget \
        unzip \
        git \
        file \
        zip \
        pkg-config \
        libx11-6 \
        libxext6 \
        libxdamage1 \
        libxfixes3 \
        libxrandr2 \
        libxcomposite1 \
        libxcursor1 \
        libxi6 \
        libxtst6 \
        libnss3 \
        libnspr4 \
        libasound2t64 \
        libpulse0 \
        libgl1 \
        libglib2.0-0 \
        libdbus-1-3 \
        libpng16-16 \
        libxkbfile1 \
    && rm -rf /var/lib/apt/lists/*

# Layer 2: SDK cmdline-tools
ENV ANDROID_HOME=/opt/android-sdk
RUN mkdir -p ${ANDROID_HOME}/cmdline-tools && \
    wget -q https://dl.google.com/android/repository/commandlinetools-linux-11076708_latest.zip -O /tmp/clt.zip && \
    unzip -q /tmp/clt.zip -d ${ANDROID_HOME}/cmdline-tools && \
    mv ${ANDROID_HOME}/cmdline-tools/cmdline-tools ${ANDROID_HOME}/cmdline-tools/latest && \
    rm /tmp/clt.zip

ENV PATH=${ANDROID_HOME}/cmdline-tools/latest/bin:${ANDROID_HOME}/platform-tools:${ANDROID_HOME}/emulator:${PATH}

# Layer 3: licenses + small components (platform-tools, platform, build-tools)
RUN yes | sdkmanager --licenses > /dev/null 2>&1; \
    sdkmanager "platform-tools" \
              "platforms;${PLATFORM}" \
              "build-tools;${BUILD_TOOLS}"

# Layer 4: NDK (large)
RUN sdkmanager "${NDK_VERSION}"

# Layer 5: emulator (large)
RUN sdkmanager "emulator"

# Layer 6: system image (largest)
RUN sdkmanager "${SYS_IMAGE}"

# Derive ANDROID_NDK_HOME from the ARG (ENV cannot do shell expansion).
# Written to a profile script so every shell (docker exec, scripts) sees it.
RUN NDK_DIR="${ANDROID_HOME}/ndk/$(echo "${NDK_VERSION}" | cut -d';' -f2)" && \
    printf 'export ANDROID_NDK_HOME=%s\nexport ANDROID_HOME=%s\n' "${NDK_DIR}" "${ANDROID_HOME}" \
        > /etc/profile.d/android-env.sh && \
    chmod +x /etc/profile.d/android-env.sh

WORKDIR /workspace

# adb server inside container
EXPOSE 5037