<!-- keywords: 2d, vector, graphics, rendering, nanovg, opengl, vulkan, metal, d3d11, rlgl -->
# mulle-nanovg

Vector graphics rendering library for C (fork of NanoVG with multi-backend support).

Use this topic when you need to draw 2D shapes, text, images, or UI elements
using any of the supported backends: OpenGL 2/3/ES2/ES3, Metal, DirectX 11,
Vulkan, or rlgl (raylib GL abstraction).

## When to use

- Drawing antialiased 2D vector graphics (paths, fills, strokes)
- Rendering text with TrueType fonts, ClearType, or color emoji
- Compositing UI elements with gradients, patterns, and scissor clipping
- Off-screen rendering via framebuffer objects
- Pre-compiling path tessellation for reuse (`NVG_PATH_CONTEXT`)

## Understand first

```bash
mulle-sde api apropos NVGcontext
mulle-sde api apropos nvgCreate
mulle-sde api cat nanovg.h
```

## Local references

| Source | Path |
|--------|------|
| Core API header | `src/nanovg.h` |
| Umbrella header | `src/mulle-nanovg.h` |
| GL backend + create flags | `src/nanovg_gl.h` |
| GL framebuffer utils | `src/nanovg_gl_utils.h` |
| rlgl backend | `src/nanovg_rlgl.h` |
| rlgl framebuffer utils | `src/nanovg_rlgl_utils.h` |
| Metal backend | `src/nanovg_mtl.h` |
| Vulkan backend | `src/nanovg_vk.h` |
| D3D11 backend | `src/nanovg_d3d11.h` |
| Framebuffer type | `src/nanovg-framebuffer.h` |
| Tracing | `src/nanovg-trace-functions.h` |
| Demo: full UI demo | `demo/src/demo.c` |
| Demo: rlgl GL3.3 | `demo/src/main-linux-rlgl-gl.c` |
| Demo: rlgl ClearType | `demo/src/main-linux-rlgl-cleartype.c` |
| Demo: rlgl emoji | `demo/src/main-linux-rlgl-emoji.c` |
| Demo: GLES3 FBO | `demo/src/main-linux-gl-fbo.c` |
| Demo: Vulkan | `demo/src/main-linux-vulkan.c` |
| Demo: D3D11 | `demo/src/main-windows-d3d11-minimal.c` |

## Primary workflow

### 1. Choose and activate a backend

Each backend is header-only; activate with a single `#define` before including.

```c
#define NANOVG_RLGL_IMPLEMENTATION
#include <nanovg_rlgl.h>
```

See the backend table in `README.md` for per-platform defines and creator
functions. The rlgl backend is the most portable (GL3.3 + GLES2/3).

### 2. Create context and frame loop

```c
NVGcontext *vg = nvgCreateRLGL(NVG_ANTIALIAS | NVG_STENCIL_STROKES);
// ... inside render loop:
nvgBeginFrame(vg, winWidth, winHeight, devicePixelRatio);
// ... drawing calls ...
nvgEndFrame(vg);
nvgDeleteRLGL(vg);
```

### 3. Pick a context type

| Flag | Enum value | Use case |
|------|-----------|----------|
| `NVG_FULL_CONTEXT` | 0 | Full rendering (default) |
| `NVG_PATH_ONLY` | `1<<3` | Pre-compile path tessellation, no GL needed |
| `NVG_FONT_ONLY` | `1<<4` | Text metrics only, no GL needed |
| `NVG_CLEARTYPE` | `1<<5` | Enable sub-pixel ClearType text rendering |
| `NVG_EMOJI` | `1<<8` | Enable color emoji atlas (RGBA) |
| `NVG_MSAA` | `1<<6` | Multisample anti-aliasing |
| `NVG_NO_DRAWING` | `1<<7` | Tracing/dry-run mode |

## Major API families

| Family | Key entry points | Source |
|--------|-----------------|--------|
| Frame management | `nvgBeginFrame`, `nvgEndFrame`, `nvgCancelFrame`, `nvgChangeFrame`, `nvgFlushFrame` | `src/nanovg.h` |
| Shape paths | `nvgBeginPath`, `nvgMoveTo`, `nvgLineTo`, `nvgBezierTo`, `nvgRect`, `nvgRoundedRect`, `nvgEllipse`, `nvgCircle`, `nvgArc`, `nvgFill`, `nvgStroke`, `nvgCopyPath` | `src/nanovg.h` |
| Text & fonts | `nvgCreateFont`, `nvgFontSize`, `nvgFontFace`, `nvgText`, `nvgTextBox`, `nvgTextBounds`, `nvgTextVisualBounds`, `nvgTextMetrics`, `nvgTextBreakLines`, `nvgClearType`, `nvgSetEmojiFontId`, `nvgAttributedText` | `src/nanovg.h` |
| Paints & styles | `nvgFillColor`, `nvgStrokeColor`, `nvgLinearGradient`, `nvgBoxGradient`, `nvgRadialGradient`, `nvgImagePattern`, `nvgStrokeWidth` | `src/nanovg.h` |
| Images | `nvgCreateImage`, `nvgCreateImageRGBA`, `nvgCreateImageTexture`, `nvgImageTextureInfo`, `nvgForgetImageTexture` | `src/nanovg.h` |
| Transforms | `nvgTranslate`, `nvgRotate`, `nvgScale`, `nvgTransform`, `nvgResetTransform` | `src/nanovg.h` |
| Scissoring | `nvgScissor`, `nvgIntersectScissor`, `nvgResetScissor`, `nvgGetScissor`, `nvgSetScissor` | `src/nanovg.h` |
| State | `nvgSave`, `nvgRestore`, `nvgReset`, `nvgGlobalAlpha` | `src/nanovg.h` |
| Framebuffers | `nvgluCreateFramebuffer`, `nvgluBindFramebuffer`, `glngvReadPixels` | `src/nanovg_gl_utils.h` |
| rlgl Framebuffers | `nvgluCreateFramebufferRLGL`, `nvgluBindFramebufferRLGL`, `nvgluReadPixelsRLGL` | `src/nanovg_rlgl_utils.h` |
| Tracing | `nvgTrace`, `nvgTraceName`, `nvgTraceComment` | `src/nanovg.h` |
| Backend-specific | `nvgCreateRLGL`/`nvgDeleteRLGL`, `nvgCreateGL3`/`nvgCreateMTL`/`nvgCreateDX11`/`nvgCreateVk` | per-backend header |

## Verify

- All rendering calls must be between `nvgBeginFrame()` and `nvgEndFrame()`.
- The stencil buffer is required. Ensure the window/framebuffer has stencil bits.
- Context creation returns `NULL` on failure — always check the return value.
- Use `NVG_DEBUG` flag for extra OpenGL error checks during development.
