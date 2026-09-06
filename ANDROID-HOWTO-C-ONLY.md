# Android How-To — C-Only Build & Run (mulle-nanovg)

> The working guide. Everything below reproduces what `ANDROID-MATCHPLAN.md`
> reports done as of 2026-09-06: a **C-only** Android app — NativeActivity +
> EGL + GLES3, no Java, no Gradle, no mulle-sde — that renders the nanovg demo,
> built and run from inside **Docker**.
>
> Companion docs: [ANDROID.md](ANDROID.md) (research),
> [ANDROID-MATCHPLAN.md](ANDROID-MATCHPLAN.md) (plan + completion notes).

---

## 1. What you get / what this is

```
┌────────────── host ──────────────┐      ┌─────────── docker ────────────┐
│ editor, git, docker, adb(opt.)   │      │ nanovg-android  (image)       │
│                                  │      │  ├ SDK cmdline-tools, platform │
│ repo  ── bind mount ──▶ /workspace ──▶  │  ├ NDK r29 (toolchain)        │
│   android/* (sources+scripts)    │      │  ├ emulator + system image    │
│   android/build/ (APK, .so) ◀────┘      │  └ /dev/kvm, /dev/bus/usb     │
│                                  │      │ nanovg-emu  (container, -d)   │
│ docker exec adb install/shell ◀───────▶ │  └ boots emulator headless    │
└──────────────────────────────────┘      └───────────────────────────────┘
```

- **C only**: `main-android.c` (NativeActivity + EGL + GLES3), the nanovg
  sources, `demo.c`. The APK is assembled by hand (`aapt2` → `zip` →
  `zipalign` → `apksigner`). No Gradle, no Java code, no `R.java`.
- **Docker-first** (not host cross-compile): one container holds SDK + NDK +
  emulator + tools. The host only needs `docker` (and optionally `adb`).
- **Why C-only packaging instead of Gradle?** For a demo it is dramatically
  simpler: no AGP version matrix, no wrapper downloads, ~200 lines of shell.
  Revisit only if you need Play-ready packaging (icons-per-density, AAB, …).

---

## 2. Host prerequisites

| Requirement | Notes |
|---|---|
| Linux with Docker (daemon running) | any recent Docker is fine |
| `/dev/kvm` | needed for a usable in-Docker emulator. Check: `ls -l /dev/kvm`. Without it the emulator is ~unusably slow → fall back to a physical device or a host-side emulator |
| ~15 GB free in Docker's storage | image holds SDK+NDK+emulator+system image. Check `docker info \| grep "Docker Root"` and `df -h` there |
| USB access for physical devices (optional) | `--device /dev/bus/usb` (already in `docker-run.sh`) |
| Physical device (optional) | if you have one, adb sees it in the container; otherwise use the emulator |

The repo tree you need:

```
MulleUI/mulle-nanovg/
├── src/                  # nanovg core + header-only backends (nanovg_gl.h …)
├── demo/                 # demo.c, main-linux-gles.c, fonts, images
├── ANDROID.md            # background research
├── ANDROID-MATCHPLAN.md  # plan + what is done
└── android/              # ← everything for the Android build lives here
    ├── Dockerfile
    ├── docker-run.sh     # interactive container (dev shell)
    ├── build.sh          # cross-compile + package APK
    ├── run.sh            # emulator | boot-wait | install | logcat | all
    ├── AndroidManifest.xml
    ├── res/              # strings.xml + launcher icon (mipmaps)
    ├── jni/
    │   ├── CMakeLists.txt
    │   ├── main-android.c        # app entry: NativeActivity+EGL+GLES3+loop
    │   └── android_assets.{c,h}  # AAssetManager wrapper
    ├── screenshots/      # proof frames from the emulator
    └── assets/           # staged from demo/ by build.sh (gitignored)
```

Two files outside `android/` were touched (both guarded so the desktop build is
unchanged):

- `demo/src/demo.c` — `NVG_ANDROID` branches in `loadDemoData` (loads via
  `AAssetManager` + `nvgCreateImageMem`/`nvgCreateFontMem`), GLFW/iconv
  includes guarded. Desktop path is byte-identical inside `#else`.
- `ANDROID-MATCHPLAN.md` — this workflow's status/log.

---

## 3. One-time setup

### 3.1 Build the Docker image

```bash
cd MulleUI/mulle-nanovg/android
docker build -t nanovg-android .      # ~5-15 min first time (downloads SDK/NDK)
```

This installs (pinned, verified via `sdkmanager --list`):

- Ubuntu 24.04, JDK 17, cmake, ninja, plus X11/GL/audio libs the emulator
  binary needs even headless
- `platform-tools`, `platforms;android-34`, `build-tools;34.0.0`
- NDK `29.0.14206865` (stable r29), emulator, `system-images;android-34;google_apis;x86_64`
- `/etc/profile.d/android-env.sh` exporting `ANDROID_HOME` / `ANDROID_NDK_HOME`

> If the build is interrupted: each SDK component is its own RUN layer, so
> re-running resumes from the last completed layer (Docker cache).

### 3.2 (Optional) interactive dev shell

```bash
./docker-run.sh
```

This execs into an interactive container with the repo mounted at
`/workspace`, KVM + USB forwarded, and the SDK env sourced. From here you can
`build.sh`, `run.sh`, or any adb/ndk command by hand. When you exit, the
container is removed (`--rm`) — the **image stays**, so the next shell is
instant.

Everything below also works as one-shot `docker run` commands (that is how
this guide runs them), no interactive shell needed.

---

## 4. Building the APK

From inside the container (or `docker run --rm`), one command:

```bash
bash /workspace/MulleUI/mulle-nanovg/android/build.sh
```

What `build.sh` does, step by step:

0. Runs `build-libs.sh` first: cross-compiles **static freetype + harfbuzz**
   per ABI into `android/third_party/prefix/<lib>/<abi>/` unless already
   built (first run downloads ~15 MB of pinned sources from GitHub — needs
   network, then a few minutes of compiling). Freetype is built with
   subpixel rendering enabled: `fontstash.h` hard-requires
   `FT_CONFIG_OPTION_SUBPIXEL_RENDERING` unless you define
   `NANOVG_NO_CLEARTYPE`. Delete `android/third_party/prefix/<lib>` to force
   a rebuild after bumping versions/flags.
1. `cmake` configure with the NDK toolchain file, per ABI:
   `-DCMAKE_TOOLCHAIN_FILE=${ANDROID_NDK_HOME}/build/cmake/android.toolchain.cmake`
   `-DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-21` (repeats for each ABI in `${ABIS}`, default `arm64-v8a armeabi-v7a`)
2. `ninja` builds `libnanovg-demo.so` (only changed objects → incremental)
3. Stages demo fonts/images into `android/assets/`
4. `aapt2 compile` + `aapt2 link` the resources/manifest into a base APK
   (`-A assets` embeds the assets)
5. Zips each ABI's `.so` under `lib/<abi>/`
6. `zipalign` + `apksigner` with a debug keystore (auto-generated once at
   `android/debug.keystore`, gitignored)

Output: **`android/build/app-debug.apk`** (multi-ABI, signed).

Run it as a one-shot container:

```bash
cd <repo root>   # the directory that contains MulleUI/
docker run --rm -v "$PWD":/workspace -w /workspace nanovg-android \
  bash -lc 'source /etc/profile.d/android-env.sh && bash /workspace/MulleUI/mulle-nanovg/android/build.sh'
```

> The build dirs and the keystore live in the **host** repo
> (`android/build/`, gitignored). They survive container churn, which is what
> makes rebuilds fast.

---

## 5. Running on the emulator

### 5.1 Start the emulator as a persistent container

```bash
docker rm -f nanovg-emu 2>/dev/null          # idempotent restart
docker run -d --name nanovg-emu \
  --device /dev/kvm -e KVM=1 \
  -v "$PWD":/workspace \
  nanovg-android bash -lc 'source /etc/profile.d/android-env.sh && /workspace/MulleUI/mulle-nanovg/android/run.sh emulator'
```

`run.sh emulator` creates the AVD on first run (`nanovg`, android-34 x86_64)
and boots the emulator **headless** (`-no-window -gpu swiftshader_indirect`).
Because it is the container's main process, the container stays up as long as
the emulator runs.

### 5.2 Wait for boot, install, launch

```bash
# wait until Android has booted (30–120 s)
docker exec nanovg-emu bash -lc 'source /etc/profile.d/android-env.sh && /workspace/MulleUI/mulle-nanovg/android/run.sh boot-wait'

# install the APK and start the activity
docker exec nanovg-emu bash -lc 'source /etc/profile.d/android-env.sh && /workspace/MulleUI/mulle-nanovg/android/run.sh install'
```

Expected output: `Success` + `Starting: Intent { cmp=com.mulle.nanovg.demo/android.app.NativeActivity }`.

### 5.3 Verify it actually renders

```bash
# our logs (init + demo data)
docker exec nanovg-emu bash -lc 'adb logcat -d -s nanovg'

# process alive?
docker exec nanovg-emu bash -lc 'adb shell pidof com.mulle.nanovg.demo'

# grab a frame and check it is not a flat color
docker exec nanovg-emu bash -lc 'adb exec-out screencap -p > /tmp/frame.png'
docker cp nanovg-emu:/tmp/frame.png ./frame.png
identify -format '%wx%h mean=%[fx:mean] stddev=%[fx:standard_deviation]\n' frame.png   # stddev > ~0.05 ⇒ real content
```

A healthy logcat looks like:

```
I nanovg  : android_main entered
I nanovg  : APP_CMD_INIT_WINDOW
I nanovg  : EGL init OK, window 320x640
I nanovg  : NanoVG GLES3 context created successfully
I nanovg  : Demo data loaded (12 images, 4 fonts)
I nanovg  : APP_CMD_GAINED_FOCUS
```

Touch works too — a held finger toggles the demo's magnified "blowup" view:

```bash
docker exec nanovg-emu bash -lc 'adb shell input swipe 160 320 160 320 1500 & sleep 0.8; adb exec-out screencap -p > /tmp/blowup.png; wait'
```

---

## 6. Running on a physical device

No hardware was available when this was written, so this path is plumbed but
**not verified** — treat it as documented configuration:

```bash
# start the emulator container with USB forwarded (docker-run.sh does this too)
docker run -d --name nanovg-dev --device /dev/kvm --device /dev/bus/usb \
  -v "$PWD":/workspace nanovg-android bash -lc 'sleep infinity'

# inside that container, adb sees the phone
docker exec nanovg-dev bash -lc 'source /etc/profile.d/android-env.sh && adb devices'
# → List of devices attached
#   <serial>    device

docker exec nanovg-dev bash -lc 'source /etc/profile.d/android-env.sh && adb install -r /workspace/MulleUI/mulle-nanovg/android/build/app-debug.apk && adb shell am start -n com.mulle.nanovg.demo/android.app.NativeActivity'
```

The APK contains `arm64-v8a` and `armeabi-v7a`, so it installs on virtually
any modern phone. (Enable *Developer options → USB debugging* on the phone.)

---

## 7. The development loop (edit → build → run)

This is the part to internalize: **the host is your editor; the container is
your toolchain; the source is shared, not copied.**

### How the mount works

- The repo is bind-mounted **read-write** into the container at `/workspace`.
- You edit `demo.c` / `main-android.c` on the host. The container sees the
  change immediately (no copy, no sync).
- The container writes build products back into the host repo
  (`android/build/`, `android/debug.keystore`). They are gitignored and they
  **persist** — that is what makes incremental builds work across container
  invocations.
- Nothing stateful lives inside a build container; every `docker run --rm` is
  fresh but cheap (image is built; only changed `.c` files recompile).
- The **emulator container is the one long-lived piece** — rebooting Android
  takes a minute, so it stays up and you redeploy with fast `adb install`s.

### The fast iteration recipe

```bash
# 1. edit a source file on the host (any editor)

# 2. rebuild (ninja recompiles only what changed → seconds)
docker run --rm -v "$PWD":/workspace -w /workspace nanovg-android \
  bash -lc 'source /etc/profile.d/android-env.sh && bash /workspace/MulleUI/mulle-nanovg/android/build.sh'

# 3. redeploy (emulator already booted → also seconds)
docker exec nanovg-emu bash -lc 'source /etc/profile.d/android-env.sh && adb install -r /workspace/MulleUI/mulle-nanovg/android/build/app-debug.apk && adb shell am start -n com.mulle.nanovg.demo/android.app.NativeActivity'

# 4. watch logs
docker exec nanovg-emu adb logcat -s nanovg
```

Or, inside the interactive shell from `docker-run.sh`, the same loop reads:

```bash
./build.sh            # 2
./run.sh install      # 3
adb logcat -s nanovg  # 4   (run.sh logcat also works)
```

Typical round-trip after the first full build: **a few seconds** for the .so +
repackage, then an instant `adb install`.

### What is *not* in the loop (important)

- **No `mulle-sde craft`.** The Android target is built with plain CMake + the
  NDK toolchain on purpose: mulle-sde assumes a Unix dev environment and was
  never pointed at Android (see Decision 6 in the matchplan). Desktop dev on
  the host keeps using mulle-sde; the Android path simply bypasses it.
- **No Gradle.** Packaging is a shell script (aapt2/zip/zipalign/apksigner).
- **No Java in the app.** `android:hasCode="false"`, pure native activity.

---

## 8. Where the code lives / what to edit

| File | Role | When you edit it |
|---|---|---|
| `android/jni/main-android.c` | App entry: `android_main`, EGL init, render loop, touch input, ClearType demo strip | windowing/EGL/input/frame pacing |
| `android/jni/android_assets.{c,h}` | `AAssetManager` read helper | asset loading |
| `demo/src/demo.c` | The actual demo scene (shared with desktop) | drawing content |
| `android/jni/CMakeLists.txt` | NDK build graph (`NVG_ANDROID`; freetype headers + libs via `FREETYPE_DIR`/`HARFBUZZ_DIR`) | adding sources/flags |
| `android/build-libs.sh` | cross-compiles static freetype + harfbuzz per ABI (auto-run by build.sh, incremental) | lib versions, build flags |
| `android/build.sh` | cross-compile + packaging pipeline | ABIs, keystore, assets |
| `android/run.sh` | emulator orchestration subcommands | AVD/emulator flags |
| `android/Dockerfile` | the toolchain image | SDK/NDK versions, system packages |
| `android/AndroidManifest.xml` | app metadata (`uses-sdk`, activity, icon) | permissions, SDK levels |

Source notes that matter when editing:

- `main-android.c` is a 1:1 port of `demo/src/main-linux-gles.c` — if you
  change the demo's render loop there, mirror it here.
- `demo.c` has `NVG_ANDROID` branches only around *asset loading* (fonts and
  images via memory APIs). The drawing code itself is shared unchanged.
- Fonts run on **freetype**, the mulle-nanovg README default: `fontstash.h`
  picks `FONS_USE_FREETYPE` when neither it nor `FONS_USE_STB_TRUETYPE` is
  defined — same path as desktop. Freetype **2.13.3** (subpixel rendering
  enabled) and harfbuzz **14.4.0** are cross-compiled per ABI by
  `build-libs.sh`. harfbuzz has **no consumer code yet** in this fork — it is
  built and linked so it is ready when text shaping lands; the linker drops
  the static archive from the .so until then.
- **ClearType** (freetype LCD subpixel AA) is on: `main-android.c` creates the
  context with `NVG_CLEARTYPE` and overlays a bottom strip comparing
  "Regular" vs "ClearType" text on a solid panel (screenshot
  `android/screenshots/emulator-gles3-cleartype.png`). Rules baked in there:
  `nvgClearType(vg, 1)` selects the second RGBA/LCD font context; the text
  background color must match what is *behind* the text and be set **after**
  `nvgTextColor` (which resets it); set `nvgClearType` before `nvgFontFace`
  (font lookup follows the clearType state); keep x integer. No
  `NANOVG_CLEARTYPE` define is needed — the GL shader's `type == 4` branch
  ships unconditionally; only GLES3 `dFdx`, freetype subpixel rendering, and
  the `NVG_CLEARTYPE` create flag are required. Toggle off: remove the flag
  and the `draw_cleartype_strip` call in `main-android.c`.
- Rendering backends are header-instantiated: `main-android.c` does
  `#define NANOVG_GLES3_IMPLEMENTATION` + `#include <nanovg_gl.h>` exactly
  once (see gotcha 2 below).

---

## 9. Troubleshooting / gotchas (all hit and solved here)

1. **`app_dummy()` is load-bearing.** Removing it breaks the link: the static
   glue object (which defines `ANativeActivity_onCreate`) is never pulled in,
   and the app dies at load with
   `UnsatisfiedLinkError … undefined symbol: ANativeActivity_onCreate`.
   Keep the `app_dummy()` call (deprecated warning is cosmetic).
2. **`nanovg_gl.h`'s implementation sits *outside* its include guard.** Include
   it exactly once per translation unit, and include `demo.h` (which pulls the
   core headers) *before* the `NANOVG_GLES3_IMPLEMENTATION` include — same
   ordering as `main-linux-gles.c`.
3. **`<uses-sdk>` is mandatory.** Android 14+ rejects an APK with no targetSdk
   (`INSTALL_FAILED_DEPRECATED_SDK_VERSION: … found 0`). Manifest declares
   `minSdkVersion=21 targetSdkVersion=34`.
4. **The emulator needs X11/GL/audio system libs even headless.** Symptoms:
   `emulator: error while loading shared libraries: libX11.so.6 …`. Find all
   missing ones with
   `ldd $ANDROID_HOME/emulator/qemu/linux-x86_64/qemu-system-x86_64` (set
   `LD_LIBRARY_PATH=…/emulator/lib64:…/lib64/qt/lib` first) and `apt-get
   install` them. The Dockerfile already lists the full set.
5. **Docker `ENV` cannot do shell expansion.** `ANDROID_NDK_HOME` is derived
   from the pinned NDK version in a `RUN` step into
   `/etc/profile.d/android-env.sh`.
6. **No background processes across tool calls** (if you are driving this from
   an agent): long builds must be resumable (split RUN layers / Docker cache)
   and the emulator must run as a daemon container (`docker run -d`).
7. **First `emulator -version` may fail on libpng/libxkbfile** → covered by the
   same package list as #4.
8. **AVD creation asks questions** — `run.sh` pipes `echo no` into
   `avdmanager create avd` and uses `--force`, so it is non-interactive.

---

## 10. Cleaning up

```bash
docker stop nanovg-emu && docker rm nanovg-emu   # stop the emulator
docker rmi nanovg-android                        # drop the ~10 GB image
rm -rf MulleUI/mulle-nanovg/android/build        # drop build products (APK/.so)
rm -rf MulleUI/mulle-nanovg/android/third_party  # drop freetype/harfbuzz sources + libs (re-fetched by next build.sh)
```

Everything reproducible is in the repo; nothing else is needed to start over
at section 3.

---

## 11. Full from-scratch reproduction (cheat sheet)

```bash
# host, from the repo root that contains MulleUI/
cd MulleUI/mulle-nanovg/android

# 1. image (once, ~10 min)
docker build -t nanovg-android .

# 2. APK (incremental afterwards)
docker run --rm -v "$PWD/../..":/workspace -w /workspace nanovg-android \
  bash -lc 'source /etc/profile.d/android-env.sh && bash /workspace/MulleUI/mulle-nanovg/android/build.sh'

# 3. emulator (persistent)
docker run -d --name nanovg-emu --device /dev/kvm -e KVM=1 \
  -v "$PWD/../..":/workspace nanovg-android \
  bash -lc 'source /etc/profile.d/android-env.sh && /workspace/MulleUI/mulle-nanovg/android/run.sh emulator'

# 4. boot → install → launch → logs
docker exec nanovg-emu bash -lc 'source /etc/profile.d/android-env.sh && /workspace/MulleUI/mulle-nanovg/android/run.sh boot-wait'
docker exec nanovg-emu bash -lc 'source /etc/profile.d/android-env.sh && /workspace/MulleUI/mulle-nanovg/android/run.sh install'
docker exec nanovg-emu bash -lc 'adb logcat -d -s nanovg'

# 5. dev loop after editing a source file: re-run step 2, then step 4's install line
```

Expected end state: logcat shows `Demo data loaded (12 images, 4 fonts)` and
`screencap` frames look like `android/screenshots/emulator-gles3-demo.png`.
