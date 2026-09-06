# mulle-nanovg Library Documentation for AI
<!-- Keywords: vector-graphics, 2d-rendering, font-rendering, opengl, vulkan, metal, nanovg -->

## 1. Introduction & Purpose

**mulle-nanovg** is an enhanced fork of the NanoVG antialiased 2D vector graphics rendering library, combining multiple rendering backends into a single project. It provides an HTML5 Canvas-like API for drawing shapes, text, images, and gradients across OpenGL (2/3/ES2/ES3), Metal, Vulkan, DirectX 11, and rlgl backends.

Key features beyond upstream NanoVG:
- Multiple context types (`NVG_FULL_CONTEXT`, `NVG_PATH_CONTEXT`, `NVG_FONT_CONTEXT`) for offline tesselation and font-only contexts
- Path copying between contexts (`nvgCopyPath`) with pre-compilation support
- ClearType subpixel text rendering (via FreeType LCD)
- Color emoji rendering via fallback font mechanism
- Attributed text with per-run color/font/size
- Additional text metrics (`nvgTextVisualBounds`, `nvgTextVisualMetrics`)
- Command tracing (`nvgTrace`) for debugging and vibecoding
- Low-level font and texture handle access
- Framebuffer support across all backends
- ReadPixels support

Depends on `mulle-freetype` for font rendering (stb_truetype is included but not the default).

## 2. Key Concepts & Design Philosophy

### Canvas-Style API
The API mirrors the HTML5 Canvas: `nvgBeginPath()` clears paths, shape functions define the path, `nvgFill()` or `nvgStroke()` renders it. State (transforms, colors, fonts) is managed via `nvgSave()`/`nvgRestore()` stack.

### Frame-Based Rendering
All drawing must occur between `nvgBeginFrame()` and `nvgEndFrame()`. Geometry is buffered and flushed in `nvgEndFrame()`. Use `nvgChangeFrame()` to flush and change viewport mid-frame (useful for FBO rendering). `nvgFlushFrame()` flushes without discarding font cache. `nvgCancelFrame()` aborts the current frame.

### Context Types
Three `NVGcontextType` variants:
- **NVG_FULL_CONTEXT**: Complete context with GPU rendering — what you normally use.
- **NVG_PATH_CONTEXT**: Stores only path geometry; no fonts or textures. Use for offline tesselation with `nvgPrecompilePath()`, then copy into a FULL context via `nvgCopyPath()`.
- **NVG_FONT_CONTEXT**: Stores only font state; used for text metrics without GPU interaction.

Create with `NVG_PATH_ONLY` or `NVG_FONT_ONLY` flags.

### Even-Odd Fill Rule
Solid shapes wound counter-clockwise (`NVG_CCW`/`NVG_SOLID`), holes wound clockwise (`NVG_CW`/`NVG_HOLE`). Use `nvgPathWinding()` after defining a path to mark holes.

### Font System
Fonts are loaded by name from `.ttf`/`.ttc` files or memory. Glyphs are rasterized into a texture atlas (`fontstash`). Fallback fonts provide glyph substitution (used for emoji). For emoji-range codepoints (U+2000 and above) the fallback lookup prefers a *color* font (checked via `fons__tt_isColorFont`): a monochrome fallback is remembered but only selected if no color fallback has the glyph, so a monochrome emoji font registered before a color one does not win. The default backend uses FreeType; stb_truetype is available with `-DFONS_USE_STB_TRUETYPE`.

### Backend Selection
Each backend is activated with a `#define` before including its header, providing a `nvgCreate*()` constructor. See Section 3.2-3.6 for per-backend details.

## 3. Core API & Data Structures

The primary public header is `nanovg.h`. Backend headers are included as needed.

### 3.1. `nanovg.h` — Core API

#### `struct NVGcontext`
Opaque handle to a drawing context. Created by a backend constructor and destroyed by its corresponding delete function.

---
#### `struct NVGcolor`
```c
struct NVGcolor {
   union {
      float rgba[4];
      struct { float r, g, b, a; };
   };
};
```
ABGR ordering internally for shaders, but accessed via `.r/.g/.b/.a`.

**Color constructors:**
```c
NVGcolor nvgRGB(unsigned char r, unsigned char g, unsigned char b);
NVGcolor nvgRGBf(float r, float g, float b);
NVGcolor nvgRGBA(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
NVGcolor nvgRGBAf(float r, float g, float b, float a);
NVGcolor nvgLerpRGBA(NVGcolor c0, NVGcolor c1, float u);
NVGcolor nvgTransRGBA(NVGcolor c0, unsigned char a);
NVGcolor nvgTransRGBAf(NVGcolor c0, float a);
NVGcolor nvgHSL(float h, float s, float l);
NVGcolor nvgHSLA(float h, float s, float l, unsigned char a);
```

---
#### `struct NVGpaint`
```c
struct NVGpaint {
   float xform[6];
   float extent[2];
   float radius;
   float feather;
   NVGcolor innerColor;
   NVGcolor outerColor;
   int image;
   int clearType;
   int isEmoji;
};
```
Returned by paint constructors, set via `nvgFillPaint()`/`nvgStrokePaint()`.

**Paint constructors:**
```c
NVGpaint nvgLinearGradient(NVGcontext* ctx, float sx, float sy, float ex, float ey, NVGcolor icol, NVGcolor ocol);
NVGpaint nvgBoxGradient(NVGcontext* ctx, float x, float y, float w, float h, float r, float f, NVGcolor icol, NVGcolor ocol);
NVGpaint nvgRadialGradient(NVGcontext* ctx, float cx, float cy, float inr, float outr, NVGcolor icol, NVGcolor ocol);
NVGpaint nvgImagePattern(NVGcontext* ctx, float ox, float oy, float ex, float ey, float angle, int image, float alpha);
```

---
#### `struct NVGglyphPosition`
```c
struct NVGglyphPosition {
   const char* str;
   float x;
   float minx, maxx;
   float miny, maxy;
};
```

#### `struct NVGtextRow`
```c
struct NVGtextRow {
   const char* start;
   const char* end;
   const char* next;
   float width;
   float minx, maxx;
};
```

#### `struct NVGtextAttribute` (mulle-nanovg addition)
```c
struct NVGtextAttribute {
   const char   *end;
   NVGcolor      textColor;
   NVGcolor      backgroundColor;
   int           fontId;        // -1 = inherit from nvg state
   float         fontSize;      // <=0 = inherit from nvg state
   float         yOffset;       // 0=baseline, negative=superscript
};
```

#### `struct NVGscissor`
```c
struct NVGscissor {
   float xform[6];
   float extent[2];
};
```

#### `struct NVGvertex`
```c
struct NVGvertex {
   float x, y, u, v;
};
```

#### `struct NVGpath`
```c
struct NVGpath {
   int first;
   int count;
   unsigned char closed;
   int nbevel;
   NVGvertex* fill;
   int nfill;
   NVGvertex* stroke;
   int nstroke;
   int winding;
   int convex;
};
```

#### `struct NVGparams`
```c
struct NVGparams {
   void* userPtr;
   int edgeAntiAlias;
   int clearType;          // mulle-nanovg addition
   int emoji;              // mulle-nanovg addition
   int cStates;            // mulle-nanovg addition
   enum NVGcontextType contextType;  // mulle-nanovg addition
   int (*renderCreate)(void* uptr);
   int (*renderCreateTexture)(void* uptr, int type, int w, int h, int imageFlags, const unsigned char* data);
   int (*renderDeleteTexture)(void* uptr, int image);
   int (*renderUpdateTexture)(void* uptr, int image, int x, int y, int w, int h, const unsigned char* data);
   int (*renderGetTextureSize)(void* uptr, int image, int* w, int* h);
   void (*renderViewport)(void* uptr, float width, float height, float devicePixelRatio);
   void (*renderCancel)(void* uptr);
   void (*renderFlush)(void* uptr);
   void (*renderFill)(void* uptr, NVGpaint* paint, NVGcompositeOperationState compositeOperation, NVGscissor* scissor, float fringe, const float* bounds, const NVGpath* paths, int npaths);
   void (*renderStroke)(void* uptr, NVGpaint* paint, NVGcompositeOperationState compositeOperation, NVGscissor* scissor, float fringe, float strokeWidth, const NVGpath* paths, int npaths);
   void (*renderTriangles)(void* uptr, NVGpaint* paint, NVGcompositeOperationState compositeOperation, NVGscissor* scissor, const NVGvertex* verts, int nverts, float fringe);
   void (*renderDelete)(void* uptr);
   int (*renderDefineTexture)(void* uptr, int w, int h, int type, int flags, void *texture);
   int (*renderForgetTexture)(void* uptr, int image);
   int (*renderGetTextureInfo)(void* uptr, int image, int *type, int *flags, void **texture);
   int (*renderValidatePaintTexture)(void* uptr, int image);
};
```
Used to construct a `NVGcontext` with `nvgCreateInternal()`. Backends fill this struct.

---
#### Context Lifecycle
```c
NVGcontext* nvgCreateInternal(NVGparams* params);
void nvgDeleteInternal(NVGcontext* ctx);
NVGparams* nvgInternalParams(NVGcontext* ctx);
void nvgDebugDumpPathCache(NVGcontext* ctx);
```

#### Frame Management
```c
void nvgBeginFrame(NVGcontext* ctx, float windowWidth, float windowHeight, float devicePixelRatio);
void nvgCancelFrame(NVGcontext* ctx);
void nvgChangeFrame(NVGcontext* ctx, float windowWidth, float windowHeight, float devicePixelRatio);
void nvgFlushFrame(NVGcontext* ctx);
void nvgEndFrame(NVGcontext* ctx);
```

#### State Management
```c
void nvgSave(NVGcontext* ctx);
void nvgRestore(NVGcontext* ctx);
void nvgReset(NVGcontext* ctx);
```

#### Render Styles
```c
void nvgShapeAntiAlias(NVGcontext* ctx, int enabled);
void nvgStrokeColor(NVGcontext* ctx, NVGcolor color);
void nvgStrokePaint(NVGcontext* ctx, NVGpaint paint);
void nvgFillColor(NVGcontext* ctx, NVGcolor color);
void nvgFillPaint(NVGcontext* ctx, NVGpaint paint);
void nvgMiterLimit(NVGcontext* ctx, float limit);
void nvgStrokeWidth(NVGcontext* ctx, float size);
void nvgLineCap(NVGcontext* ctx, int cap);   // NVG_BUTT, NVG_ROUND, NVG_SQUARE, NVG_BEVEL, NVG_MITER
void nvgLineJoin(NVGcontext* ctx, int join);  // NVG_MITER, NVG_ROUND, NVG_BEVEL
void nvgGlobalAlpha(NVGcontext* ctx, float alpha);
void nvgBezierTessellation(NVGcontext* ctx, int tess); // NVG_TESS_SUBDIVISION or NVG_TESS_AFD
```

**ClearType (mulle-nanovg addition):**
```c
void nvgClearType(NVGcontext* ctx, int enabled);
void nvgTextColor(NVGcontext* ctx, NVGcolor color);
void nvgTextBackgroundColor(NVGcontext* ctx, NVGcolor color);
```

#### Composite Operations
```c
void nvgGlobalCompositeOperation(NVGcontext* ctx, int op);
void nvgGlobalCompositeBlendFunc(NVGcontext* ctx, int sfactor, int dfactor);
void nvgGlobalCompositeBlendFuncSeparate(NVGcontext* ctx, int srcRGB, int dstRGB, int srcAlpha, int dstAlpha);
```

#### Transforms
```c
void nvgResetTransform(NVGcontext* ctx);
void nvgTransform(NVGcontext* ctx, float a, float b, float c, float d, float e, float f);
void nvgTranslate(NVGcontext* ctx, float x, float y);
void nvgRotate(NVGcontext* ctx, float angle);
void nvgSkewX(NVGcontext* ctx, float angle);
void nvgSkewY(NVGcontext* ctx, float angle);
void nvgScale(NVGcontext* ctx, float x, float y);
void nvgCurrentTransform(NVGcontext* ctx, float* xform);
```

**Matrix utilities (operate on float[6]):**
```c
void nvgTransformIdentity(float* dst);
void nvgTransformTranslate(float* dst, float tx, float ty);
void nvgTransformScale(float* dst, float sx, float sy);
void nvgTransformRotate(float* dst, float a);
void nvgTransformSkewX(float* dst, float a);
void nvgTransformSkewY(float* dst, float a);
void nvgTransformMultiply(float* dst, const float* src);
void nvgTransformPremultiply(float* dst, const float* src);
int nvgTransformInverse(float* dst, const float* src);
void nvgTransformPoint(float* dstx, float* dsty, const float* xform, float srcx, float srcy);
```

**Angle conversion:**
```c
float nvgDegToRad(float deg);
float nvgRadToDeg(float rad);
```

#### Images
```c
int  nvgCreateImage(NVGcontext* ctx, const char* filename, int imageFlags);
int  nvgCreateImageMem(NVGcontext* ctx, int imageFlags, unsigned char* data, int ndata);
int  nvgCreateImageRGBA(NVGcontext* ctx, int w, int h, int imageFlags, const unsigned char* data);
void nvgUpdateImage(NVGcontext* ctx, int image, const unsigned char* data);
void nvgImageSize(NVGcontext* ctx, int image, int* w, int* h);
void nvgDeleteImage(NVGcontext* ctx, int image);
```

**Low-level texture access (mulle-nanovg addition):**
```c
int  nvgCreateImageTexture(NVGcontext* ctx, int w, int h, int imageFlags, int type, void **texture);
void nvgImageTextureInfo(NVGcontext* ctx, int image, int *imageFlags, int *type, void **texture);
void nvgForgetImageTexture(NVGcontext* ctx, int image);
```

`NVGimageFlags`: `NVG_IMAGE_GENERATE_MIPMAPS`, `NVG_IMAGE_REPEATX`, `NVG_IMAGE_REPEATY`, `NVG_IMAGE_FLIPY`, `NVG_IMAGE_PREMULTIPLIED`, `NVG_IMAGE_NEAREST`, `NVG_IMAGE_MSAA`, `NVG_IMAGE_RBO_LESS`, `NVG_IMAGE_RENDER_TARGET`.

#### Scissoring
```c
void nvgScissor(NVGcontext* ctx, float x, float y, float w, float h);
void nvgIntersectScissor(NVGcontext* ctx, float x, float y, float w, float h);
void nvgResetScissor(NVGcontext* ctx);
void nvgGetScissor(NVGcontext* ctx, NVGscissor *scissor);
void nvgSetScissor(NVGcontext* ctx, NVGscissor *scissor);
```

#### Paths
```c
void nvgBeginPath(NVGcontext* ctx);
void nvgMoveTo(NVGcontext* ctx, float x, float y);
void nvgLineTo(NVGcontext* ctx, float x, float y);
void nvgBezierTo(NVGcontext* ctx, float c1x, float c1y, float c2x, float c2y, float x, float y);
void nvgQuadTo(NVGcontext* ctx, float cx, float cy, float x, float y);
void nvgArcTo(NVGcontext* ctx, float x1, float y1, float x2, float y2, float radius);
void nvgClosePath(NVGcontext* ctx);
void nvgPathWinding(NVGcontext* ctx, int dir);  // NVG_CCW/NVG_CW or NVG_SOLID/NVG_HOLE
void nvgArc(NVGcontext* ctx, float cx, float cy, float r, float a0, float a1, int dir);
void nvgRect(NVGcontext* ctx, float x, float y, float w, float h);
void nvgRoundedRect(NVGcontext* ctx, float x, float y, float w, float h, float r);
void nvgRoundedRectVarying(NVGcontext* ctx, float x, float y, float w, float h, float radTopLeft, float radTopRight, float radBottomRight, float radBottomLeft);
void nvgEllipse(NVGcontext* ctx, float cx, float cy, float rx, float ry);
void nvgCircle(NVGcontext* ctx, float cx, float cy, float r);
void nvgFill(NVGcontext* ctx);
void nvgStroke(NVGcontext* ctx);
void nvgPrecompilePath(NVGcontext *ctx);
void nvgCopyPath(NVGcontext* ctx, NVGcontext *path);
```

#### Fonts
```c
int  nvgCreateFont(NVGcontext* ctx, const char* name, const char* filename);
int  nvgCreateFontAtIndex(NVGcontext* ctx, const char* name, const char* filename, const int fontIndex);
int  nvgCreateFontMem(NVGcontext* ctx, const char* name, unsigned char* data, int ndata, int freeData);
int  nvgCreateFontMemAtIndex(NVGcontext* ctx, const char* name, unsigned char* data, int ndata, int freeData, const int fontIndex);
int  nvgFindFont(NVGcontext* ctx, const char* name);
int  nvgAddFallbackFontId(NVGcontext* ctx, int baseFont, int fallbackFont);
int  nvgAddFallbackFont(NVGcontext* ctx, const char* baseFont, const char* fallbackFont);
void nvgResetFallbackFontsId(NVGcontext* ctx, int baseFont);
void nvgResetFallbackFonts(NVGcontext* ctx, const char* baseFont);
int  nvgSetEmojiFontId(NVGcontext* ctx, int fontId);
int  nvgSetEmojiFont(NVGcontext* ctx, const char* name);
void *nvgGetFontHandle(NVGcontext* ctx, int font);  // returns FT_Face* or stbtt_fontinfo*
```

#### Text Styling
```c
void nvgFontSize(NVGcontext* ctx, float size);
void nvgFontBlur(NVGcontext* ctx, float blur);
void nvgTextLetterSpacing(NVGcontext* ctx, float spacing);
void nvgTextLineHeight(NVGcontext* ctx, float lineHeight);
void nvgTextAlign(NVGcontext* ctx, int align);
void nvgFontFaceId(NVGcontext* ctx, int font);
void nvgFontFace(NVGcontext* ctx, const char* font);
```

`NVGalign`: `NVG_ALIGN_LEFT`, `NVG_ALIGN_CENTER`, `NVG_ALIGN_RIGHT`, `NVG_ALIGN_TOP`, `NVG_ALIGN_MIDDLE`, `NVG_ALIGN_BOTTOM`, `NVG_ALIGN_BASELINE`. Combine horizontal and vertical with bitwise OR.

#### Text Rendering
```c
float nvgText(NVGcontext* ctx, float x, float y, const char* string, const char* end);
void  nvgTextBox(NVGcontext* ctx, float x, float y, float breakRowWidth, const char* string, const char* end);
float nvgAttributedText(NVGcontext* ctx, float x, float y, const char* string, const char* end, struct NVGtextAttribute* attributes, int nAttributes);
```

`nvgText()` returns the horizontal advance (x position where next character would be drawn). `nvgAttributedText()` (mulle-nanovg addition) renders a string with per-segment color, font, size, and y-offset changes.

#### Text Measurement
```c
float nvgTextBounds(NVGcontext* ctx, float x, float y, const char* string, const char* end, float* bounds);
void  nvgTextVisualBounds(NVGcontext* ctx, float x, float y, const char* string, const char* end, float bounds[6]);
void  nvgTextBoxBounds(NVGcontext* ctx, float x, float y, float breakRowWidth, const char* string, const char* end, float* bounds);
int   nvgTextGlyphPositions(NVGcontext* ctx, float x, float y, const char* string, const char* end, NVGglyphPosition* positions, int maxPositions);
void  nvgTextMetrics(NVGcontext* ctx, float* ascender, float* descender, float* lineh);
void  nvgTextMetricsWithGap(NVGcontext* ctx, float* ascender, float* descender, float* lineh, float* lineGap);
void  nvgTextVisualMetrics(NVGcontext* ctx, float* ascender, float* descender, float* lineh);
int   nvgTextBreakLines(NVGcontext* ctx, const char* string, const char* end, float breakRowWidth, NVGtextRow* rows, int maxRows);
```

`nvgTextBounds()` returns `bounds[4]` = `[xmin, ymin, xmax, ymax]` and returns horizontal advance. `nvgTextVisualBounds()` returns `bounds[6]` = `[xmin, ymin, xmax, ymax, ytop, ybot]` where the first four are actual pixel bounds and ytop/ybot match `nvgTextBounds` bounds[1]/bounds[3]. `nvgTextVisualMetrics()` measures from the actual bitmap font in the atlas.

#### Trace (mulle-nanovg addition, disabled when `NVG_TRACECODE_DISABLED` is defined)
```c
FILE *nvgTrace(NVGcontext* ctx, FILE * file);
void  nvgTraceName(NVGcontext* ctx, char *name);
void  nvgTraceComment(NVGcontext* ctx, const char *text);
```

Enable tracing by calling `nvgTrace(ctx, stderr)` (or any `FILE*`). Pass `NULL` to disable. Every NanoVG API call is printed to the file, allowing replay/debug.

### 3.2. `nanovg_gl.h` — OpenGL Backend

Activate with one `#define` before inclusion: `NANOVG_GL2_IMPLEMENTATION`, `NANOVG_GL3_IMPLEMENTATION`, `NANOVG_GLES2_IMPLEMENTATION`, or `NANOVG_GLES3_IMPLEMENTATION`.

**Context creators:**
```c
NVGcontext* nvgCreateGL2(int flags);
void nvgDeleteGL2(NVGcontext* ctx);
int nvglCreateImageFromHandleGL2(NVGcontext* ctx, GLuint textureId, int w, int h, int flags);
GLuint nvglImageHandleGL2(NVGcontext* ctx, int image);

NVGcontext* nvgCreateGL3(int flags);
void nvgDeleteGL3(NVGcontext* ctx);
int nvglCreateImageFromHandleGL3(NVGcontext* ctx, GLuint textureId, int w, int h, int flags);
GLuint nvglImageHandleGL3(NVGcontext* ctx, int image);

NVGcontext* nvgCreateGLES2(int flags);
void nvgDeleteGLES2(NVGcontext* ctx);
int nvglCreateImageFromHandleGLES2(NVGcontext* ctx, GLuint textureId, int w, int h, int flags);
GLuint nvglImageHandleGLES2(NVGcontext* ctx, int image);

NVGcontext* nvgCreateGLES3(int flags);
void nvgDeleteGLES3(NVGcontext* ctx);
int nvglCreateImageFromHandleGLES3(NVGcontext* ctx, GLuint textureId, int w, int h, int flags);
GLuint nvglImageHandleGLES3(NVGcontext* ctx, int image);
```

`NVGcreateFlags` (shared by GL/rlgl):
- `NVG_ANTIALIAS` (1<<0): Geometry-based anti-aliasing.
- `NVG_STENCIL_STROKES` (1<<1): Stencil-based stroke overlap handling (slightly slower but correct for self-intersecting paths).
- `NVG_DEBUG` (1<<2): Additional debug checks.
- `NVG_PATH_ONLY` (1<<3): Create `NVG_PATH_CONTEXT`.
- `NVG_FONT_ONLY` (1<<4): Create `NVG_FONT_CONTEXT`.
- `NVG_CLEARTYPE` (1<<5): Enable ClearType subpixel rendering.
- `NVG_MSAA` (1<<6): Enable MSAA (mutually exclusive with `NVG_ANTIALIAS`).
- `NVG_NO_DRAWING` (1<<7): Dummy backend for tracing only.
- `NVG_EMOJI` (1<<8): RGBA atlas for color emoji.

Additional GL backend flag: `NVG_IMAGE_NODELETE` (1<<16): Do not delete GL texture handle when image is deleted.

### 3.3. `nanovg_rlgl.h` — rlgl Backend

Uses raylib's rlgl as OpenGL abstraction. Works on GL3.3, GLES2, and GLES3 from a single codebase.

Define one before including `mulle-rlgl.h`:
- `GRAPHICS_API_OPENGL_33`
- `GRAPHICS_API_OPENGL_ES3`
- `GRAPHICS_API_OPENGL_ES2`

**Context lifecycle:**
```c
NVGcontext* nvgCreateRLGL(int flags);
void nvgDeleteRLGL(NVGcontext* ctx);
int nvglCreateImageFromHandleRLGL(NVGcontext* ctx, unsigned int textureId, int w, int h, int flags);
unsigned int nvglImageHandleRLGL(NVGcontext* ctx, int image);
```

Same `NVGcreateFlags` as GL backend. Desktop GL requires `rlLoadExtensions()` after context creation; GLES does not.

### 3.4. `nanovg_mtl.h` — Metal Backend (Apple platforms only)

```c
NVGcontext* nvgCreateMTL(void* metalLayer, int flags);
void nvgDeleteMTL(NVGcontext* ctx);
```

Metal create flags match the GL/rlgl enum for interface-level flags: `NVG_ANTIALIAS` (1<<0), `NVG_STENCIL_STROKES` (1<<1), `NVG_DEBUG` (1<<2), `NVG_CLEARTYPE` (1<<5), `NVG_EMOJI` (1<<8), `NVG_DOUBLE_BUFFER` (1<<12), `NVG_TRIPLE_BUFFER` (1<<13), `NVG_PATH_ONLY` (1<<14), `NVG_FONT_ONLY` (1<<15). `NVG_CLEARTYPE` and `NVG_EMOJI` were added for ClearType/color-emoji parity with the GL backend.

**Additional image flag:**
```c
enum NVGimageFlagsMetal {
  NVG_IMAGE_NODELETE = 1 << 16, // Do not delete Metal texture handle.
};
```

**Framebuffer functions:**
```c
void nvglBindFramebuffer(NVGcontext* ctx, NVGframebuffer fb);
NVGframebuffer mnvgCreateFramebuffer(NVGcontext* ctx, int w, int h, int imageFlags);
void mnvgDeleteFramebuffer(NVGcontext* ctx, NVGframebuffer fb);
void mnvgClearWithColor(NVGcontext* ctx, NVGcolor color);
```

**Metal bridging:**
```c
void* mnvgCommandQueue(NVGcontext* ctx);    // id<MTLCommandQueue>
void* mnvgDevice(NVGcontext* ctx);          // id<MTLDevice>
void* mnvgImageHandle(NVGcontext* ctx, int image);  // id<MTLTexture>
int   mnvgCreateImageFromHandle(NVGcontext* ctx, void* textureId, int imageFlags);
void  mnvgReadPixels(NVGcontext* ctx, int image, int x, int y, int width, int height, void* data);
enum MNVGTarget mnvgTarget();  // MNVG_IOS, MNVG_MACOS, MNVG_SIMULATOR, MNVG_TVOS, MNVG_UNKNOWN
```

### 3.5. `nanovg_d3d11.h` — DirectX 11 Backend

**D3D11-specific `NVGcreateFlags`:**
```c
enum NVGcreateFlags {
   NVG_ANTIALIAS       = 1<<0,
   NVG_STENCIL_STROKES = 1<<1,
   NVG_DEBUG           = 1<<2,
   NVG_PATH_ONLY       = 1<<3,
   NVG_FONT_ONLY       = 1<<4
};
```

**Context lifecycle:**
```c
struct NVGcontext* nvgCreateD3D11(void* ID3D11Device_pDevice, int edgeaa);
void nvgDeleteD3D11(struct NVGcontext* ctx);
```

**Framebuffer support:**
```c
NVGframebuffer d3dnvgCreateFramebuffer(struct NVGcontext* ctx, int w, int h, int flags);
void d3dnvgDeleteFramebuffer(struct NVGcontext* ctx, NVGframebuffer fb);
void d3dnvgBindFramebuffer(struct NVGcontext* ctx, NVGframebuffer fb);
void d3dnvgClearWithColor(struct NVGcontext* ctx, NVGcolor color);
```

**Utilities:**
```c
void d3dnvgCopyImage(struct NVGcontext* ctx, int imgDest, int imgSrc);
void d3dnvgWriteImage(struct NVGcontext* ctx, int imgDest, void* data);
void d3dnvgReadPixels(struct NVGcontext* ctx, NVGframebuffer fb, int x, int y, int width, int height, void* data);
```

**Internal texture access:**
```c
struct D3DNVGtexture* D3Dnvg__findTexture(struct D3DNVGcontext* D3D, int id);
```

**Additional image flags:**
```c
enum NVGimageFlagsD3D11 {
   NVG_IMAGE_NODELETE      = 1<<16,
   NVG_IMAGE_USAGE_DYNAMIC = 1<<18,
   NVG_IMAGE_USAGE_STAGING = 1<<19,
};
```

Precompiled shaders in `d3d11_shaders/`.

### 3.6. `nanovg_vk.h` — Vulkan Backend

Activate with `NANOVG_VULKAN_IMPLEMENTATION`. Header-only like the other backends (include the implementation in exactly one translation unit).

**Vulkan-specific `NVGcreateFlags`:**
```c
enum NVGcreateFlags {
   NVG_ANTIALIAS       = 1 << 0,
   NVG_STENCIL_STROKES = 1 << 1,
   NVG_DEBUG           = 1 << 2,
};
```

**Context lifecycle:**
```c
static NVGcontext* nvgCreateVk(VKNVGCreateInfo createInfo, int flags, VkQueue queue);
static void nvgDeleteVk(NVGcontext *ctx);
```

**Image from handle:**
```c
int vknvgCreateImageFromHandle(NVGcontext* ctx, VkImageView imageView, int w, int h, int imageFlags);
```

**External texture ownership (mulle-nanovg addition):** The Vulkan backend implements the `renderDefineTexture`/`renderForgetTexture`/`renderGetTextureInfo` callbacks for parity with the GL/D3D11/Metal backends, so the core `nvgCreateImageTexture()`/`nvgForgetImageTexture()`/`nvgImageTextureInfo()` work on Vulkan too. The native "texture handle" exchanged through these APIs is a `VkImageView`. Owned textures are marked:
```c
#define NVG_IMAGE_NODELETE  (1<<16)  // Do not delete the underlying Vulkan resources.
```
When `NVG_IMAGE_NODELETE` is set (which `vknvgCreateImageFromHandle` and `renderDefineTexture` always do), `vknvg_deleteTexture` destroys only the nanovg-created `VkSampler` and clears the texture slot; the caller's `VkImageView`/`VkImage`/`VkDeviceMemory` are left intact. `renderGetTextureInfo` returns the stored flags, type, and the view handle.

`VKNVGCreateInfo` contains Vulkan device, renderpass, command buffer, swapchain info, queue family index, allocator callbacks, and optional extensions (`VkNvgExt`). `VkNvgExt` has fields for `dynamicState`, `colorBlendEquation`, and `colorWriteMask` requiring `VK_EXT_EXTENDED_DYNAMIC_STATE` and `VK_EXT_EXTENDED_DYNAMIC_STATE_3` extensions.

### 3.7. Framebuffer Utilities

#### `nanovg_gl_utils.h` — GL FBO helpers
```c
NVGframebuffer nvgluCreateFramebuffer(NVGcontext* ctx, int w, int h, int imageFlags);
void nvgluDeleteFramebuffer(NVGframebuffer fb);
void nvgluBindFramebuffer(NVGframebuffer fb);
void nvgluBindReadFramebuffer(NVGframebuffer fb);
void nvgluBindDrawFramebuffer(NVGframebuffer fb);
void nvgluClearWithColor(NVGcontext* ctx, NVGcolor color);
void glnvgReadPixels(NVGcontext* ctx, NVGframebuffer fb, int x, int y, int width, int height, void* data);
```
Internal struct: `NVGLUframebuffer { NVGcontext* ctx; GLuint fbo; GLuint rbo; GLuint texture; int image; }`.

#### `nanovg_rlgl_utils.h` — rlgl FBO helpers
```c
NVGframebuffer nvgluCreateFramebufferRLGL(NVGcontext *ctx, int w, int h, int imageFlags);
void nvgluBindFramebufferRLGL(NVGframebuffer fb);
void nvgluDeleteFramebufferRLGL(NVGframebuffer fb);
void nvgluReadPixelsRLGL(NVGcontext *ctx, NVGframebuffer fb, int x, int y, int width, int height, void *data);
```
Internal struct: `NVGRLGLframebuffer { NVGcontext *ctx; unsigned int fbo; unsigned int rbo; unsigned int texture; int image; }`.

#### `nanovg_vk_utils.h` — Vulkan FBO helpers
```c
void vknvgBindFramebuffer(NVGcontext *ctx, NVGframebuffer fb);
NVGframebuffer vknvgCreateFramebuffer(NVGcontext *ctx, int w, int h, int imageFlags);
void vknvgDeleteFramebuffer(NVGcontext *ctx, NVGframebuffer fb);
void vknvgClearWithColor(NVGcontext *ctx, NVGcolor color);
void vknvgReadPixels(NVGcontext *ctx, NVGframebuffer fb, int x, int y, int width, int height, void *data);
```

### 3.8. `nanovg-framebuffer.h` — Common FBO Type

```c
typedef void* NVGframebuffer;
```
Opaque handle used across all backends for framebuffer objects.

### 3.9. `nanovg-trace-functions.h`

Declares tracing enter/leave functions for every public API call (e.g., `_nvgBeginPath_enter`, `_nvgFill_enter`, `_nvgText_enter`/`_nvgText_leave`). Used internally by the tracing system. AI agents can use the pattern `_nvg{FunctionName}_enter` / `_nvg{FunctionName}_leave` to intercept calls.

### 3.10. `fontstash.h` — Font Rasterization Engine

Internal API that NanoVG uses for glyph rasterization. Uses FreeType by default (`FONS_USE_FREETYPE`), with stb_truetype as an alternative (`FONS_USE_STB_TRUETYPE`).

**Key types:**
```c
typedef struct FONScontext FONScontext;
struct FONSparams { int width, height; int pixelsize; int bytewidth; unsigned char flags; void* userPtr; ... };
struct FONSquad { float x0,y0,s0,t0; float x1,y1,s1,t1; };
struct FONStextIter { float x, y, nextx, nexty, scale, spacing; unsigned int codepoint; ... };
```

**Lifecycle:**
```c
FONScontext* fonsCreateInternal(FONSparams* params);
void fonsDeleteInternal(FONScontext* s);
```

**Font management:**
```c
int fonsAddFont(FONScontext* s, const char* name, const char* path, int fontIndex);
int fonsAddFontMem(FONScontext* s, const char* name, unsigned char* data, int ndata, int freeData, int fontIndex);
int fonsGetFontByName(FONScontext* s, const char* name);
int fonsGetNumFonts(FONScontext* s);
void* fonsGetFontHandle(FONScontext* s, int fontId);  // FT_Face* or stbtt_fontinfo*
```

**Atlas management:**
```c
void fonsGetAtlasSize(FONScontext* s, int* width, int* height);
int fonsExpandAtlas(FONScontext* s, int width, int height);
int fonsResetAtlas(FONScontext* stash, int width, int height, int pixelsize);
const unsigned char* fonsGetTextureData(FONScontext* stash, int* width, int* height);
int fonsValidateTexture(FONScontext* s, int* dirty);
```

**State and rendering:**
```c
void fonsPushState(FONScontext* s);
void fonsPopState(FONScontext* s);
void fonsClearState(FONScontext* s);
void fonsSetSize(FONScontext* s, float size);
void fonsSetColor(FONScontext* s, unsigned int color);
void fonsSetSpacing(FONScontext* s, float spacing);
void fonsSetBlur(FONScontext* s, float blur);
void fonsSetAlign(FONScontext* s, int align);
void fonsSetFont(FONScontext* s, int font);
float fonsDrawText(FONScontext* s, float x, float y, const char* string, const char* end);
float fonsTextBounds(FONScontext* s, float x, float y, const char* string, const char* end, float* bounds);
void fonsLineBounds(FONScontext* s, float y, float* miny, float* maxy);
void fonsVertMetrics(FONScontext* s, float* ascender, float* descender, float* lineh);
void fonsVertMetrics4(FONScontext* s, float* ascender, float* descender, float* lineh, float* lineGap);
int fonsTextIterInit(FONScontext* stash, FONStextIter* iter, float x, float y, const char* str, const char* end, int bitmapOption);
int fonsTextIterNext(FONScontext* stash, FONStextIter* iter, struct FONSquad* quad);
void fonsDrawDebug(FONScontext* s, float x, float y);
```

**Error handling:**
```c
void fonsSetErrorCallback(FONScontext* s, void (*callback)(void* uptr, int error, int val), void* uptr);
```
Error codes: `FONS_ATLAS_FULL`, `FONS_SCRATCH_FULL`, `FONS_STATES_OVERFLOW`, `FONS_STATES_UNDERFLOW`.

### 3.11. `mulle-nanovg.h` — Unified Include

Master include header that conditionally pulls in the appropriate backend based on compile defines (`NANOVG_GL2`, `NANOVG_GL3`, `NANOVG_GLES2`, `NANOVG_GLES3`, `NANOVG_MTL`, `NANOVG_D3D11`, `NANOVG_VULKAN`).

### 3.12. Key Enums

```c
enum NVGcontextType   { NVG_FULL_CONTEXT = 0, NVG_PATH_CONTEXT = 1, NVG_FONT_CONTEXT = 2 };
enum NVGwinding       { NVG_CCW = 1, NVG_CW = 2 };
enum NVGsolidity      { NVG_SOLID = 1, NVG_HOLE = 2 };
enum NVGlineCap       { NVG_BUTT, NVG_ROUND, NVG_SQUARE, NVG_BEVEL, NVG_MITER };
enum NVGblendFactor   { NVG_ZERO, NVG_ONE, NVG_SRC_COLOR, NVG_ONE_MINUS_SRC_COLOR, ... };
enum NVGcompositeOperation { NVG_SOURCE_OVER, NVG_SOURCE_IN, NVG_SOURCE_OUT, ... };
enum NVGtess          { NVG_TESS_SUBDIVISION, NVG_TESS_AFD };
enum NVGtexture       { NVG_TEXTURE_ALPHA = 0x01, NVG_TEXTURE_RGBA = 0x02 };
```

## 4. Performance Characteristics

- **Path rendering:** Geometry is tesselated on CPU each frame. Use `NVG_PATH_CONTEXT` with `nvgPrecompilePath()` and `nvgCopyPath()` for static path optimization.
- **Tessellation:** `NVG_TESS_AFD` (adaptive forward differencing) is faster than default `NVG_TESS_SUBDIVISION` (De Casteljau).
- **Text atlas:** Glyphs are cached in a texture atlas. First use of a glyph at a given size causes atlas update. Atlas expansion triggers re-upload; handle `FONS_ATLAS_FULL` via error callback.
- **Frame buffering:** Commands are batched and flushed at `nvgEndFrame()`. `nvgFlushFrame()` flushes without clearing font cache.
- **MSAA:** Set `NVG_MSAA` flag instead of `NVG_ANTIALIAS` if using hardware MSAA.
- **Thread safety:** Not thread-safe. All operations on a given `NVGcontext` must be serialized.
- **Stencil:** Setting `NVG_STENCIL_STROKES` slows strokes slightly but ensures correct overlap rendering. The render target must have a stencil buffer.

## 5. AI Usage Recommendations & Patterns

### Best Practices
- Always create context with the backend constructor, draw between `nvgBeginFrame()`/`nvgEndFrame()`, and delete with the corresponding `nvgDelete*()`.
- Use `nvgSave()`/`nvgRestore()` to isolate state changes (colors, transforms, fonts, scissor).
- Set font state (size, face, alignment) before calling `nvgText()` or `nvgTextBounds()`.
- Provide `end` parameter as `NULL` for null-terminated strings, or point past last char for substrings.
- For multi-line text, use `nvgTextBox()` or pre-calculate line breaks with `nvgTextBreakLines()`.
- `nvgTextBreakLines()` can count lines by passing `NULL` for `rows`.

### Common Pitfalls
- Do not call drawing functions outside `nvgBeginFrame()`/`nvgEndFrame()`.
- Stencil buffer is required — ensure render target has one.
- `nvgText()` returns horizontal advance, not visual width. Use `nvgTextBounds()` for bounding box.
- `nvgGetFontHandle()` resets font state. Re-query font metrics after calling it.
- `nvgClearType()` requires FreeType compiled with `FT_CONFIG_OPTION_SUBPIXEL_RENDERING`.
- Color emoji requires `NVG_EMOJI` flag for RGBA atlas; the emoji font should be set via `nvgSetEmojiFont()`.
- `NVG_PATH_CONTEXT` does not support font or image operations.
- Do not delete an image while it's used in a paint in the current frame.
- Images created from caller-owned handles (`nvgCreateImageTexture()`, `vknvgCreateImageFromHandle()`, `nvglCreateImageFromHandle*()`) are marked `NVG_IMAGE_NODELETE`. Deleting the nanovg image releases only nanovg's internal slot/sampler — never the caller's native texture/view. The caller retains ownership and must clean it up itself.

### Idiomatic Patterns
- **FBO rendering:** Call `nvgChangeFrame()` to switch render targets mid-frame; then `nvgEndFrame()` at the end.
- **Static path optimization:** Create a `NVG_PATH_CONTEXT`, define path, call `nvgPrecompilePath()`, then `nvgCopyPath()` into the main context.
- **Attributed text:** Fill `NVGtextAttribute` array where each attribute's `.end` points past its run's last byte. Attributes tile contiguously.
- **ClearType:** Always call `nvgTextColor()` before `nvgTextBackgroundColor()`. Provide the known background color for best results.

## 6. Integration Examples

### Example 1: Basic Setup with GL3 Backend

```c
#define NANOVG_GL3_IMPLEMENTATION
#include "nanovg_gl.h"

// After OpenGL context + window creation:
NVGcontext* vg = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);
if( ! vg)
   return( -1);

// Load a font
nvgCreateFont( vg, "sans", "fonts/Roboto-Regular.ttf");
nvgCreateFont( vg, "sans-bold", "fonts/Roboto-Bold.ttf");

// Main loop
while( ! glfwWindowShouldClose( window))
{
   glfwGetWindowSize( window, &winWidth, &winHeight);
   glfwGetFramebufferSize( window, &fbWidth, &fbHeight);
   pxRatio = (float) fbWidth / (float) winWidth;

   glClear( GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

   nvgBeginFrame( vg, winWidth, winHeight, pxRatio);

   // Draw a rounded rectangle
   nvgBeginPath( vg);
   nvgRoundedRect( vg, 10, 10, 200, 60, 4);
   nvgFillColor( vg, nvgRGBA( 28, 30, 34, 255));
   nvgFill( vg);

   // Draw text
   nvgFontSize( vg, 20.0f);
   nvgFontFace( vg, "sans-bold");
   nvgTextAlign( vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
   nvgFillColor( vg, nvgRGBA( 255, 255, 255, 255));
   nvgText( vg, 20, 40, "Hello, World!", NULL);

   nvgEndFrame( vg);
   glfwSwapBuffers( window);
   glfwPollEvents();
}

nvgDeleteGL3( vg);
```

### Example 2: rlgl Backend (Desktop GL3.3)

```c
#define GRAPHICS_API_OPENGL_33
#define RLGL_IMPLEMENTATION
#include <mulle-rlgl.h>

#define NANOVG_RLGL_IMPLEMENTATION
#include <nanovg_rlgl.h>

// After glfwMakeContextCurrent(window):
rlLoadExtensions( (void *) glfwGetProcAddress);
rlglInit( 1000, 600);

NVGcontext* vg = nvgCreateRLGL( NVG_ANTIALIAS | NVG_STENCIL_STROKES);

// In main loop:
rlViewport( 0, 0, fbWidth, fbHeight);
rlClearColor( 77, 77, 82, 255);
rlClearScreenBuffers();

nvgBeginFrame( vg, winWidth, winHeight, pxRatio);
// ... drawing commands ...
nvgEndFrame( vg);

glfwSwapBuffers( window);

// Cleanup:
nvgDeleteRLGL( vg);
rlglClose();
```

### Example 3: FBO Rendering with GL Utilities

```c
#define NANOVG_GL3_IMPLEMENTATION
#include "nanovg_gl.h"
#define NANOVG_GL_IMPLEMENTATION
#include "nanovg_gl_utils.h"

NVGcontext*   vg = nvgCreateGL3( NVG_ANTIALIAS | NVG_STENCIL_STROKES);
NVGframebuffer fb = nvgluCreateFramebuffer( vg, 256, 256, NVG_IMAGE_FLIPY);

// Render to FBO first
nvgluBindFramebuffer( fb);
nvgBeginFrame( vg, 256, 256, 1.0f);
nvgluClearWithColor( vg, nvgRGBAf( 0, 0, 0, 0));
nvgBeginPath( vg);
nvgRect( vg, 10, 10, 100, 80);
nvgFillColor( vg, nvgRGBA( 255, 0, 0, 255));
nvgFill( vg);
nvgEndFrame( vg);

// Switch to default framebuffer
nvgluBindFramebuffer( NULL);
nvgBeginFrame( vg, 800, 600, 1.0f);
NVGpaint img = nvgImagePattern( vg, 50, 50, 256, 256, 0, /* FBO's image handle */, 1.0f);
nvgBeginPath( vg);
nvgRect( vg, 0, 0, 256, 256);
nvgFillPaint( vg, img);
nvgFill( vg);
nvgEndFrame( vg);

nvgluDeleteFramebuffer( fb);
nvgDeleteGL3( vg);
```

### Example 4: Static Path Precompilation

```c
NVGcontext*  pathCtx = nvgCreateGL3( NVG_PATH_ONLY);
NVGcontext*  fullCtx = nvgCreateGL3( NVG_ANTIALIAS | NVG_STENCIL_STROKES);

// Precompile a complex path once
nvgBeginPath( pathCtx);
nvgCircle( pathCtx, 50, 50, 40);
nvgRect( pathCtx, 10, 30, 80, 20);
nvgPathWinding( pathCtx, NVG_HOLE);
nvgPrecompilePath( pathCtx);

// Each frame, copy the path and fill it
nvgBeginFrame( fullCtx, w, h, 1.0f);
nvgCopyPath( fullCtx, pathCtx);
nvgFillColor( fullCtx, nvgRGBA( 0, 192, 255, 255));
nvgFill( fullCtx);
nvgEndFrame( fullCtx);
```

### Example 5: Attributed Text

```c
const char* text = "BoldRed then SmallBlue";
struct NVGtextAttribute attrs[2];

attrs[0].end             = text + 7;  // "BoldRed"
attrs[0].textColor       = nvgRGBA( 255, 0, 0, 255);
attrs[0].backgroundColor = nvgRGBA( 255, 255, 255, 255);
attrs[0].fontId          = 1;         // bold font
attrs[0].fontSize        = 24.0f;
attrs[0].yOffset         = 0;

attrs[1].end             = NULL;      // remaining text
attrs[1].textColor       = nvgRGBA( 0, 0, 255, 255);
attrs[1].backgroundColor = nvgRGBA( 255, 255, 255, 255);
attrs[1].fontId          = 0;         // regular font
attrs[1].fontSize        = 16.0f;
attrs[1].yOffset         = 0;

nvgAttributedText( vg, 10, 100, text, NULL, attrs, 2);
```

### Example 6: Emoji Setup

```c
int  sansFont  = nvgCreateFont( vg, "sans", "Roboto-Regular.ttf");
int  emojiFont = nvgCreateFont( vg, "emoji", "NotoColorEmoji.ttf");

nvgAddFallbackFontId( vg, sansFont, emojiFont);
nvgSetEmojiFontId( vg, emojiFont);

// Emoji glyphs in text will now render in color
nvgFontSize( vg, 32.0f);
nvgFontFace( vg, "sans");
nvgText( vg, 10, 50, "Hello \xF0\x9F\x98\x80 World", NULL);  // 😀
```

### Example 7: Tracing

```c
nvgTrace( vg, stderr);              // start tracing to stderr
nvgTraceName( vg, "myCanvas");
nvgTraceComment( vg, "Start of important section");

nvgBeginPath( vg);                  // traced: nvgBeginPath(myCanvas)
nvgRect( vg, 0, 0, 100, 100);      // traced: nvgRect(myCanvas, 0, 0, 100, 100)
nvgFill( vg);                        // traced: nvgFill(myCanvas)

nvgTrace( vg, NULL);                // stop tracing
```

## 7. Dependencies

Direct `mulle-sde` library dependencies:
- `mulle-freetype` — FreeType font rendering wrapper
- `stb_image_resize2.h` (embedded, from stb)
- `stb_image.h` (embedded, from stb)
- `stb_truetype.h` (embedded, from stb; optional alternative to FreeType)

System/optional dependencies:
- `freetype` (system) — font rasterization
- `harfbuzz` (optional) — text shaping
- Platform links: `m`, `GLESv2`, `vulkan` (Linux); `d3d11`, `dxgi` (Windows); Metal/QuartzCore/CoreGraphics/Foundation/IOKit/AppKit (Darwin)
