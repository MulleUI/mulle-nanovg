<!-- keywords: drawing, rendering, text, shapes, framebuffer, emoji, cleartype -->
# Patterns

## Frame loop skeleton (rlgl backend)

```c
#define GRAPHICS_API_OPENGL_33
#define RLGL_IMPLEMENTATION
#include <mulle-rlgl.h>
#define NANOVG_RLGL_IMPLEMENTATION
#include <nanovg_rlgl.h>

// after GL context creation + rlglInit:
NVGcontext *vg = nvgCreateRLGL(NVG_ANTIALIAS | NVG_STENCIL_STROKES);

while (!glfwWindowShouldClose(window)) {
    glfwGetWindowSize(window, &winWidth, &winHeight);
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    float pxRatio = (float)fbWidth / (float)winWidth;

    rlViewport(0, 0, fbWidth, fbHeight);
    rlClearColor(77, 77, 82, 255);
    rlClearScreenBuffers();

    nvgBeginFrame(vg, winWidth, winHeight, pxRatio);
    // ... drawing calls ...
    nvgEndFrame(vg);

    glfwSwapBuffers(window);
    glfwPollEvents();
}

nvgDeleteRLGL(vg);
rlglClose();
```

See `demo/src/main-linux-rlgl-gl.c` for the complete example.

## Drawing a simple shape

```c
nvgBeginPath(vg);
nvgRect(vg, 100, 100, 120, 30);
nvgFillColor(vg, nvgRGBA(255, 192, 0, 255));
nvgFill(vg);
```

## Drawing composite paths with holes

Uses even-odd fill rule. Default winding is CCW. Call `nvgPathWinding` *after*
defining the sub-path to mark it as a hole:

```c
nvgBeginPath(vg);
nvgRect(vg, 100, 100, 120, 30);
nvgCircle(vg, 120, 120, 5);
nvgPathWinding(vg, NVG_HOLE);
nvgFillColor(vg, nvgRGBA(255, 192, 0, 255));
nvgFill(vg);
```

## Text rendering with ClearType

Enable at context creation with `NVG_CLEARTYPE`, then toggle per-text block:

```c
vg = nvgCreateRLGL(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_CLEARTYPE);

// Normal rendering
nvgClearType(vg, 0);
nvgFillColor(vg, nvgRGBA(255, 255, 255, 255));
nvgText(vg, 4, 4, "Hello", NULL);

// ClearType rendering (needs background color hint)
nvgClearType(vg, 1);
nvgTextBackgroundColor(vg, nvgRGBAf(0.0f, 0.0f, 0.0f, 1.0f));  // set first
nvgTextColor(vg, nvgRGBA(255, 255, 255, 255));                  // then text color
nvgText(vg, 4, 52, "Hello", NULL);
```

ClearType only works with freetype (default). Compile with `-DNANOVG_CLEARTYPE`.
The shader uses `dFdx` — not available on GLES2 (see `CLEARTYPE-INTEGER-X.md`).

## Color emoji rendering

```c
// Create context with NVG_EMOJI flag
vg = nvgCreateRLGL(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_EMOJI);

// Load emoji font and register it
int emojiFont = nvgCreateFont(vg, "emoji", "NotoColorEmoji.ttf");
nvgSetEmojiFontId(vg, emojiFont);

// Render text with embedded emoji codepoints
nvgText(vg, 10, 50, "Hello \xF0\x9F\x98\x80 World", NULL);
```

See `demo/src/main-linux-rlgl-emoji.c`.

## Off-screen rendering with framebuffers

```c
#include <nanovg_gl_utils.h>   // for GL backend
// or:
#include <nanovg_rlgl_utils.h> // for rlgl backend

// Create FBO
NVGframebuffer fb = nvgluCreateFramebufferRLGL(vg, 200, 200, 0);

// Render to FBO
nvgluBindFramebufferRLGL(fb);
rlViewport(0, 0, 200, 200);
rlClearColor(0, 0, 0, 255);
rlClearScreenBuffers();

nvgBeginFrame(vg, 200, 200, 1.0f);
// ... draw into FBO ...
nvgEndFrame(vg);

nvgluBindFramebufferRLGL(NULL);  // back to default

// Read pixels
unsigned char *pixels = malloc(200 * 200 * 4);
nvgluReadPixelsRLGL(vg, fb, 0, 0, 200, 200, pixels);
nvgluDeleteFramebufferRLGL(fb);
```

See `demo/src/main-linux-rlgl-cleartype.c` and `demo/src/main-linux-gl-fbo.c`.

## Path pre-compilation with NVG_PATH_CONTEXT

```c
// Create a path-only context (no GL needed)
NVGcontext *pathCtx = nvgCreateRLGL(NVG_PATH_ONLY);

// Define path
nvgBeginPath(pathCtx);
nvgMoveTo(pathCtx, 10, 10);
nvgLineTo(pathCtx, 100, 10);
nvgLineTo(pathCtx, 100, 100);
nvgClosePath(pathCtx);

// Pre-compile tessellation
nvgPrecompilePath(pathCtx);

// Copy into full context and render
nvgCopyPath(vg, pathCtx);
nvgFillColor(vg, nvgRGBA(255, 0, 0, 255));
nvgFill(vg);

nvgDeleteRLGL(pathCtx);
```

## Frame injection for FBO mixing

When mixing FBO rendering with the main pass, use `nvgChangeFrame` and
`nvgFlushFrame` instead of nesting `nvgBeginFrame`/`nvgEndFrame`:

```c
nvgBeginFrame(vg, winWidth, winHeight, pxRatio);  // main frame
// ... draw main scene ...
nvgFlushFrame(vg);                                  // flush without resetting state

// Bind FBO, render more
nvgluBindFramebuffer(fbo);
nvgChangeFrame(vg, fboWidth, fboHeight, pxRatio);   // new viewport, keeps state
// ... draw into FBO ...
nvgEndFrame(vg);                                     // final flush
```

## Attributed text rendering

```c
struct NVGtextAttribute attrs[2];
attrs[0].end            = "Hello ";
attrs[0].textColor      = nvgRGBA(255, 0, 0, 255);
attrs[0].backgroundColor = nvgRGBAf(0, 0, 0, 1);
attrs[0].fontId         = -1;   // inherit
attrs[0].fontSize       = 0;    // inherit
attrs[0].yOffset        = 0;

attrs[1].end            = "World" + 5;
attrs[1].textColor      = nvgRGBA(0, 255, 0, 255);
attrs[1].backgroundColor = nvgRGBAf(0, 0, 0, 1);
attrs[1].fontId         = -1;
attrs[1].fontSize       = 24.0f;
attrs[1].yOffset        = 0;

float advance = nvgAttributedText(vg, 10, 50,
                                  "Hello World",
                                  "Hello World" + 11,
                                  attrs, 2);
```

## Sharing textures between contexts

```c
// Create texture in one context and share
int image = nvgCreateImageTexture(ctx1, w, h, flags, type, &texture);

// Get texture handle
void *texHandle;
int imgFlags, texType;
nvgImageTextureInfo(ctx1, image, &imgFlags, &texType, &texHandle);

// Prevent deletion when image is deleted
nvgForgetImageTexture(ctx1, image);
```

## Tracing API calls

Enable trace output to debug rendering order:

```c
nvgTrace(vg, stdout);          // enable tracing
nvgTraceName(vg, "myvg");      // custom context name in traces
nvgTraceComment(vg, "=== rendering panel ===");  // demarcation
nvgTrace(vg, NULL);            // disable tracing (returns previous FILE*)
```
