# Android Matchplan (mulle-nanovg)

> Companion to [ANDROID.md](ANDROID.md) — this is the *working plan*: what we
> actually do next, in what order, and how we know each step is done.
> ANDROID.md is the background research; this file is the playbook.
>
> **Reproducing this from scratch?** Use
> [ANDROID-HOWTO-C-ONLY.md](ANDROID-HOWTO-C-ONLY.md) — a step-by-step
> C-only build/run/dev-loop guide.

## Status — 2026-09-06: phases 0–4 DONE ✅, step 2 (fonts) DONE ✅, step 3 (ClearType) DONE ✅

The mulle-nanovg demo **compiles and runs on Android**. A signed, multi-ABI
APK (`android/build/app-debug.apk`) is built in Docker (SDK + NDK r29 +
emulator) and runs on an emulator inside that container: full demo rendering
(shapes, text, the 12 images, icons, emoji fallback), touch input (drag =
mouse, hold = blowup), ~40 fps on software GL, no GL errors in logcat.

Evidence: `android/screenshots/emulator-gles3-demo.png` and
`emulator-gles3-blowup.png` (320x640, from `adb exec-out screencap`).
Reproduce: `cd android && docker-run.sh`, then `build.sh`, then
`run.sh all` (or the emulator + boot-wait + install subcommands).

**Fonts follow-up (step 2, done same day):** the demo now rasterizes text with
**freetype 2.13.3** — the README's default and the only "supported"
configuration — instead of the bootstrap stb_truetype build. **harfbuzz
14.4.0** is cross-compiled and linked too, ready for future text shaping.
See "Phase 4.5 — Fonts" below.

Phase 5 (Vulkan, gdb, CI, release signing) is intentionally **deferred** —
see below.

## Goal

A project that **compiles and runs on Android**: the mulle-nanovg demo (or a
minimal GLES3 renderer built from it) shipped as an installable APK, built,
run, and debugged from inside **Docker** (preferred over host cross-compile),
on an **emulator inside that container** (KVM is available on this host) or a
physical device via adb.

**Definition of done**: `adb shell am start` launches a NativeActivity app
that renders the full nanovg demo (shapes, text, images, emoji) at interactive
frame rate, with no GL errors in logcat, and we can rebuild/redeploy with one
command.

## Environment facts (checked 2026-09-06)

| Thing | State |
|-------|-------|
| Docker | 29.1.3 on host ✅ |
| `/dev/kvm` | present ✅ (emulator-in-container is viable) |
| adb | on host, daemon works, **no devices attached** → emulator is the primary target |
| Host resources | 64 cores / 121 GB RAM / ~23 GB free disk |
| cmake (host) | 4.2.3 |
| mulle-sde | installed at `/home/src/srcS/mulle-sde` (host dev only, see Decision 6) |
| Demo assets | `demo/images/image1..12.jpg` (~151K), fonts `Roboto-*`, `entypo`, `NotoEmoji-Regular`, `NotoColorEmoji` (11 MB) |
| Asset loaders | fork has `nvgCreateFontMem` / `nvgCreateImageMem` / `nvgCreateFontMemAtIndex` ✅ (no filesystem needed) |

## Decisions (locked in, revisit only if blocked)

| # | Topic | Decision | Why |
|---|-------|----------|-----|
| 1 | Delivery | **Docker-first**: one container holds SDK + NDK + emulator + build tooling. Host only runs `docker` and `adb`. | User preference; no host pollution; reproducible |
| 2 | Backend | **GLES 3.0** via `NANOVG_GLES3_IMPLEMENTATION` → `nvgCreateGLES3` | Works on API 21+; `main-linux-gles.c` already proves this backend; Vulkan (API 24+) is a stretch goal only |
| 3 | Windowing | **NativeActivity + `android_native_app_glue` + EGL**. No GLFW (no Android support). | Standard NDK path; EGL is the GLES entry on Android |
| 4 | Assets | Bundle fonts/images in APK `assets/`, load via **`AAssetManager` + `nvgCreateFontMem` / `nvgCreateImageMem`** | demo.c uses relative paths (`./entypo.ttf`, `images/imageN.jpg`) that don't exist on Android; mem loaders avoid temp files |
| 5 | Packaging | **Manual** `aapt2` + `zipalign` + `apksigner` script. **No Gradle.** | It's a demo; Gradle/AGP adds huge download + version pain for zero benefit here. Revisit only if we need Play-ready packaging |
| 6 | Build system | Android target built with **plain CMake + NDK toolchain** (bypasses mulle-sde). Host dev keeps using mulle-sde per AGENTS.md. | mulle-sde expects a Unix env; ANDROID.md already flags this as a challenge. Keep host workflow untouched, don't fight mulle-sde for the cross-compile |
| 7 | Fonts | **freetype (the README default)** — cross-compiled static per ABI by `android/build-libs.sh` (2.13.3, subpixel rendering enabled). harfbuzz (14.4.0) cross-compiled too, for future shaping. | fontstash.h defaults to `FONS_USE_FREETYPE` when neither it nor `FONS_USE_STB_TRUETYPE` is defined — same path as desktop. stb_truetype was only a bootstrap to avoid the dep. ClearType code now compiles (needs freetype) but stays off in the demo. harfbuzz is declared optional in the mulle sourcetree but has **no consumer code yet**; it is linked only so the dependency is ready |
| 8 | ABI | **arm64-v8a only** to start (emulator default). `armeabi-v7a` added later if we touch old devices | Keep the loop fast |
| 9 | API level | minSdk **21** (GLES3 guaranteed), compile/target **34** | Matches ANDROID.md research |
| 10 | AGENTS.md / vibecoding | AGENTS.md read and honored for host-side conventions, but **`mulle-sde vibecoding on` is NOT run** (explicit user instruction) | User override |

## Directory layout to create

Everything new lives under `MulleUI/mulle-nanovg/android/` so the host project
stays untouched:

Actual layout as built (2026-09-06):

```
android/
├── Dockerfile                  # SDK + NDK r29 + emulator + build tools (pinned)
├── docker-run.sh               # build image + run container with KVM/usb passthrough
├── build.sh                    # cross-compile (multi-ABI) + package + sign APK
├── build-libs.sh               # cross-compile static freetype + harfbuzz (auto-run by build.sh, incremental)
├── run.sh                      # emulator | boot-wait | install | logcat | all
├── jni/
│   ├── CMakeLists.txt          # NDK toolchain build (nanovg lib + app target)
│   ├── main-android.c          # NativeActivity + EGL + render loop (port of main-linux-gles.c)
│   └── android_assets.{c,h}    # AAssetManager wrapper for fonts/images
├── AndroidManifest.xml         # minSdk 21 / target 34, icon, NativeActivity
├── res/
│   ├── values/strings.xml
│   └── mipmap-{m,h,x,xx}dpi/ic_launcher.png
├── screenshots/                # emulator proof: demo + blowup + freetype frames
├── third_party/                # freetype/harfbuzz sources + per-ABI prefixes (gitignored)
└── assets/                     # staged from demo/ by build.sh (gitignored)
```

Source changes to the demo (deliberate, all guarded by `NVG_ANDROID`):

- `android/jni/main-android.c` — new Android main mirroring `main-linux-gles.c`
  (lives in `android/` so the host demo target is untouched).
- `demo/src/demo.c` — `NVG_ANDROID` branch in `loadDemoData` using
  `AAssetManager` + `nvgCreateImageMem` / `nvgCreateFontMem`; desktop path
  byte-identical in `#else`; GLFW/iconv includes guarded.

## Phases

Each phase ends with a **checkpoint** — concrete evidence it worked. Do not
start the next phase until the checkpoint passes.

### Phase 0 — Preflight ✅ done 2026-09-06

- [x] docker, /dev/kvm, adb, cmake verified
- [x] demo asset inventory (12 jpgs, 5 ttf), mem loaders confirmed in `src/nanovg.h`
- [x] `docker run --device /dev/kvm` verified (KVM_OK); Docker storage on a 1.8 TB partition (disk space was a non-issue)

### Phase 1 — Docker image with Android toolchain + emulator ✅ done

`android/Dockerfile` pinned to versions enumerated via a throwaway probe image
(`sdkmanager --list`, probe deleted afterwards):

- Base: `ubuntu:24.04`
- Packages: `openjdk-17-jdk-headless cmake ninja-build wget unzip` + X11/GL/audio
  libs **required by the emulator binary even headless** (`libx11-6`,
  `libpng16-16`, `libxkbfile1`, `libnss3`, `libpulse0`, … — found via `ldd`
  on `qemu-system-x86_64`)
- SDK components (each its own RUN layer so an interrupted foreground build
  resumes via layer cache):
  - `platform-tools`, `platforms;android-34`, `build-tools;34.0.0`
  - `ndk;29.0.14206865` (r29 stable — current at build time)
  - `emulator`, `system-images;android-34;google_apis;x86_64`
- `ANDROID_NDK_HOME` derived from the ARG into `/etc/profile.d/android-env.sh`
  (Docker `ENV` cannot do shell parameter expansion)

`docker-run.sh` mounts the repo, passes `--device /dev/kvm` and
`/dev/bus/usb`, sets `KVM=1`.

**Checkpoint ✅**: `docker build` succeeds; all components installed
(verified in-image); `emulator -version` runs (37.1.11.0).

### Phase 2 — Minimal GLES3 APK (vertical slice)

Goal: prove the whole loop end-to-end *before* touching demo code.

1. `android/jni/main-android.c`: android_native_app_glue boilerplate
   (APP_CMD_INIT_WINDOW etc.), EGL init (attribs from ANDROID.md Phase 2),
   `nvgCreateGLES3`, clear-color frame loop, logcat logging.
2. `android/jni/CMakeLists.txt`: `add_library(nanovg-demo SHARED main-android.c
   ...)` + nanovg sources (the fork's header-only backends mean we compile the
   nanovg `.c`/`.h` set from `src/` — reuse the library's `cmake/reflect/_Sources.cmake`
   lists or the simple `add_subdirectory(../.. nanovg)`; pick whichever compiles
   cleanest under the NDK toolchain), link `EGL GLESv3 android log`.
3. `AndroidManifest.xml` (from ANDROID.md, fixed: `android:hasCode="false"`,
   `android.app.lib_name`, exported activity, no storage permission).
4. `build.sh`: NDK toolchain cmake (arm64-v8a), copy `.so` + `aapt2` package →
   `zipalign` → `apksigner` (debug keystore generated once).
5. `run.sh`: create AVD (no-window, gpu swiftshader_indirect), boot emulator,
   `adb install`, `am start`, `adb logcat -s nanovg`.

**Checkpoint ✅**: APK installs and launches; screenshot shows the clear color
with the red smoke-test rectangle (pixel-verified: corner `(77,77,82)` =
clear color, center `(255,0,0)` = rect); logcat shows init logs, zero GL
errors.

Gotchas hit & fixed:
- manifest needs explicit `<uses-sdk minSdkVersion="21"
  targetSdkVersion="34"/>` or Android 14 rejects the APK
  (`INSTALL_FAILED_DEPRECATED_SDK_VERSION`)
- `app_dummy()` is **load-bearing**: it forces the linker to pull in the glue
  object that defines `ANativeActivity_onCreate` (missing symbol otherwise)
- this fork guards even the GLES3 *declarations* behind
  `NANOVG_GLES3_IMPLEMENTATION`, so the backend is instantiated in the main
  TU (like `main-linux-gles.c`), not in a separate file

### Phase 3 — Port the demo

1. Copy `demo/src/main-linux-gles.c` → `demo/src/main-android.c`; replace
   GLFW (init/hints/window/events/time/cursor/framebuffer-size) with
   EGL + `android_app` equivalents (Phase 2 scaffold is the template).
2. Asset loading: add `NVG_ANDROID` path in `demo.c` so fonts load via
   `AAssetManager` + `nvgCreateFontMem` and images via `nvgCreateImageMem`.
   Keep the desktop path byte-identical.
3. Input: map `AMotionEvent` touch (drag = mouse move, tap/click = button) to
   the `mx,my` + mouse-button args `renderDemo` expects. `AMotionEvent_getX/Y`.
4. `build.sh` copies `demo/` assets into `android/assets/`.
5. Keep `demo/CMakeLists.txt` working on Linux (regression check: existing
   `main-linux-gles` build unaffected).

**Checkpoint ✅**: full demo renders on the emulator — text, the 12 images,
icons, emoji fallback — screenshot has real content (71 KB PNG, stddev 0.11 vs
4 KB flat for the smoke test), steady ~40 fps on swiftshader, `logcat` clean;
touch verified: a held finger changes the frame (blowup view) — screenshots
in `android/screenshots/`.

Notes:
- `main-android.c` lives in `android/jni/` (not `demo/src/`) so the host demo
  target is untouched; it ports `main-linux-gles.c`'s loop 1:1
- asset loading: `android/jni/android_assets.{c,h}` wraps `AAssetManager`;
  `demo.c` gains an `NVG_ANDROID` branch in `loadDemoData` using
  `nvgCreateImageMem` / `nvgCreateFontMem` (freeData=1) — the desktop path is
  byte-identical inside `#else`
- `demo.c` also guards its `<GLFW/glfw3.h>` / `<iconv.h>` includes behind
  `NVG_ANDROID` (only real GL use, `glReadPixels`, sits in `#if 0`)
- **Gotcha**: `nanovg_gl.h`'s implementation section lives *outside* the
  include guard → include `demo.h` (which pulls `mulle-nanovg.h`) *before*
  the `NANOVG_GLES3_IMPLEMENTATION` include, exactly like `main-linux-gles.c`
- desktop regression: `NVG_ANDROID` is never defined on desktop, so behavior
  is unchanged by construction (no GLFW/freetype on host to run a full
  desktop build in this session)

### Phase 4 — Polish & device path ✅ (device path pending hardware)

- [x] App label (`NanoVG Demo`) + icon (4 mipmap densities, generated via
      ImageMagick, `res/mipmap-*/ic_launcher.png`)
- [x] Second ABI **armeabi-v7a** in the same APK (both verified via
      `aapt2 dump badging` and install/run on the emulator)
- [x] `build.sh` loop over `${ABIS:-arm64-v8a armeabi-v7a}`, per-ABI CMake
      dirs under `build/cmake-<abi>`
- [ ] Physical device: **not verified — no device attached** (documented path:
      `docker-run.sh` forwards `/dev/bus/usb`, platform-tools are in the
      image, adb runs in the container). Hardware needed to close this item.
- [ ] Release-signed APK: skipped — debug keystore is the demo path (see
      Phase 5)

**Checkpoint ✅ (emulator)**: `aapt2 dump badging` shows label, icons,
minSdk 21 / target 34, `native-code: 'arm64-v8a' 'armeabi-v7a'`; APK
installs and runs on the emulator.

### Phase 4.5 — Fonts: freetype + harfbuzz (step 2) ✅ done 2026-09-06

Goal: run the demo on the font stack the README actually supports — freetype
(default) instead of the stb_truetype bootstrap — and cross-compile harfbuzz
so the optional shaping dependency from the mulle sourcetree is ready.

- [x] `android/build-libs.sh`: downloads pinned sources (freetype **2.13.3**,
      harfbuzz **14.4.0**) into `android/third_party/` and cross-compiles
      static libs per ABI with the NDK toolchain
      (`-DANDROID_ABI=arm64-v8a|armeabi-v7a`, `-DANDROID_PLATFORM=android-21`).
      Incremental: skips a lib/ABI whose `.a` exists (delete the prefix to
      rebuild). Freetype built minimal: all optional deps (zlib/png/bzip2/
      brotli/harfbuzz) disabled via `FT_DISABLE_*`; harfbuzz built with
      glib/icu/graphite2/cairo + subset/raster off.
- [x] Subpixel rendering **enabled** in the vendored freetype source
      (uncommented `FT_CONFIG_OPTION_SUBPIXEL_RENDERING` in `ftoption.h`) —
      `fontstash.h` `#error`s without it unless `NANOVG_NO_CLEARTYPE` is
      defined, and desktop distro freetype builds have it on.
- [x] `jni/CMakeLists.txt`: dropped `FONS_USE_STB_TRUETYPE`; fontstash now
      takes its default `FONS_USE_FREETYPE` (no define, like desktop).
      freetype headers exposed to the `nanovg` lib TU; `libfreetype.a` +
      `libharfbuzz.a` linked into the final `.so` via per-ABI
      `FREETYPE_DIR`/`HARFBUZZ_DIR` passed by `build.sh`.
- [x] `build.sh` calls `build-libs.sh` automatically before the app build.

**Checkpoint ✅**: APK rebuilt, installed, launched on the emulator. Evidence:

- `Demo data loaded (12 images, 4 fonts)` in logcat, no FT/font warnings
- 149 `FT_*` symbols exported from `libnanovg-demo.so` (freetype truly linked)
- Full-frame stats essentially unchanged vs the stb build (mean 0.453 vs
  0.454, stddev 0.116 vs 0.115) — same scene, same text, no tofu/blank glyphs
- Live: CPU ~100% on software GL, frames differ over time (animation runs)
- Screenshot: `android/screenshots/emulator-gles3-freetype.png`

Notes for later:

- harfbuzz has **no consumer code** in this fork (no shaping calls anywhere;
  only the sourcetree/reflect config and docs mention it). The static archive
  is therefore dropped by the linker — the .so is unchanged by it. Building it
  now just removes the dependency step when shaping lands.
- ClearType is now **on**: see Phase 4.6 below. One correction to the earlier
  guess: the GL backend needs **no** `NANOVG_CLEARTYPE` define — the shader's
  `type == 4` branch is compiled unconditionally; ClearType is purely runtime
  (create-flag + `nvgClearType` + background color).
- ~8 MB of new static libs are only in `android/third_party/` (gitignored) —
  nothing grew in the APK beyond what freetype pulls into the .so.

### Phase 4.6 — ClearType text on GLES3 (step 3) ✅ done 2026-09-06

Goal: prove the whole ClearType chain on Android — freetype LCD glyphs → the
RGBA font context → the `dFdx` subpixel shader branch on GLES3 (swiftshader
software GL supports derivatives; the README only excludes GLES2).

- [x] `jni/main-android.c` creates the context with `NVG_CLEARTYPE` (adds a
      second, RGBA/LCD font context; demo fonts load into both).
- [x] `draw_cleartype_strip()` overlays a solid panel at the bottom of each
      frame with two text runs — "Regular" (`nvgClearType 0`) vs
      "ClearType" (`nvgClearType 1`) — mirroring `main-linux-gl-cleartype.c`.
      Usage rules baked in: `nvgTextBackgroundColor` **after** `nvgTextColor`
      (which resets it), background = the actual panel color, integer x
      positions, `nvgClearType` set before `nvgFontFace` (font lookup happens
      in the context selected by the clearType state). `demo.c` untouched —
      the strip lives in the Android main only.

**Checkpoint ✅**: renders on the emulator, no degradation/font warnings in
logcat. Pixel evidence from `adb exec-out screencap` (screenshot:
`android/screenshots/emulator-gles3-cleartype.png`), comparing the two text
lines in identical geometry:

| line | gray mean | R−B channel divergence | meaning |
|---|---|---|---|
| Regular | 0.259 | 0.006 | grayscale AA (R≈G≈B) |
| ClearType | 0.271 | 0.158 | ~25× subpixel fringing = LCD active |

Notes for later:

- This is a *strip demo*, not the whole scene: ClearType needs a known
  monochromatic background (the shader blends glyph coverage against the
  configured background color), so per-text-run `nvgClearType` + background
  matching is the intended usage.
- The `NANOVG_CLEARTYPE` mention in `nanovg_gl.h` is only a comment — the
  `type == 4` shader branch ships unconditionally.
- Toggle off by removing `NVG_CLEARTYPE` from the create flags in
  `main-android.c` and deleting the `draw_cleartype_strip` call.

### Phase 5 — Stretch ⏸ deferred (not started)

Deliberately not attempted in this session: each is a multi-hour effort with
its own unknowns, and phases 0–4.5 already hit the goal (compile + run on
Android + default font stack). Documented starting points:

- **Vulkan backend** (`nvgCreateVk`, minSdk 24): would need VkInstance/Device/
  swapchain setup like the desktop `main-linux-vulkan.c`, plus a separate
  activity or build flag. ANDROID.md has the diff sketch.
- **ndk-gdb / lldb**: attach to the running app inside the container
  (`adb forward` + lldb-server from the NDK).
- **CI**: GitHub Actions building `android/` with the same Dockerfile.
- **Release-signed APK** (Phase 4 carry-over): swap `debug.keystore` for a
  release key in `build.sh`.
- **NotoColorEmoji.ttf (11 MB)**: load on demand or exclude if APK size
  matters; the demo only uses `NotoEmoji-Regular.ttf` today.

## Running the loop (end state)

```bash
cd MulleUI/mulle-nanovg/android
./docker-run.sh    # build image, start container with KVM + usb
./build.sh         # cross-compile + package APK inside container
./run.sh           # boot emulator, install, launch, logcat
```

Rebuild cycle ≈ seconds for the native lib after the first container build.

## Lessons learned (gotchas, for the next Android session)

1. **`app_dummy()` is load-bearing** for static glue linking — without it the
   `.so` lacks `ANativeActivity_onCreate` and the app dies at load.
2. **`nanovg_gl.h` implementation sits outside the include guard** — include it
   exactly once per TU; pull `demo.h`/core headers first.
3. **`<uses-sdk>` is mandatory** on modern Android — aapt2 defaults targetSdk
   to 0 and install fails on API 23+.
4. **The emulator needs X11/GL/audio system libs even headless** — discover
   missing ones with `ldd` on `qemu-system-x86_64` (with
   `LD_LIBRARY_PATH=.../lib64:.../lib64/qt/lib`).
5. **Docker `ENV` has no shell expansion** — derive `ANDROID_NDK_HOME` in a
   RUN step into a profile script.
6. **Foreground-only shells here**: background processes don't survive between
   tool calls — long builds must be resumable (split RUN layers, docker layer
   cache) or daemon-managed (`docker run -d` keeps the emulator alive).
7. **This fork guards GLES3 declarations behind the implementation macro** —
   the backend must be instantiated in the TU that calls `nvgCreateGLES3`.

## Risks & open questions

1. **mulle-sde bypass (Decision 6)**: deviates from AGENTS.md's "mulle-sde is
   THE tool" for the Android target only. If the maintainer wants mulle-sde to
   drive the Android build, budget a spike first — ANDROID.md flags it as a
   known pain point.
2. **Emulator without KVM would be unusably slow** — `/dev/kvm` exists today;
   if the container host changes, fall back to: build in Docker, emulator on
   host, adb connect over TCP.
3. **Software GL** (`swiftshader_indirect`) is enough for nanovg GLES3; GPU
   passthrough (gfxstream) is an optimization, not a requirement.
4. **demo.c asset path divergence**: keep desktop path untouched; the
   `NVG_ANDROID` guard must not change Linux/macOS behavior (regression check
   in Phase 3.5).
5. **stb_truetype vs freetype (Decision 7)**: resolved in Phase 4.5 — the demo
   now renders with freetype and the frame statistics are essentially
   identical to the stb build (mean/stddev within 0.001). ClearType is also
   verified working on Android (Phase 4.6), including the LCD subpixel path.
6. **NotoColorEmoji.ttf is 11 MB** — fine in APK, but if startup is slow,
   lazy-load or defer to Phase 5 (excluded from assets today; only
   NotoEmoji-Regular is loaded by the demo).
7. **Desktop regression unverified in-session** — `NVG_ANDROID` guards keep
   desktop code identical by construction, but nobody ran the GLFW build this
   session (no GLFW/freetype on the host). Cheap to confirm in CI or on a
   dev box.

## Reference

- [ANDROID.md](ANDROID.md) — background research (Dockerfile sketch, EGL
  attribs, manifest, Vulkan diff)
- Android NDK NativeActivity: https://developer.android.com/ndk/guides/concepts#na
- android_native_app_glue: bundled in NDK (`sources/android/native_app_glue`)
- Emulator in Docker with KVM: `--device /dev/kvm` + `KVM=1`