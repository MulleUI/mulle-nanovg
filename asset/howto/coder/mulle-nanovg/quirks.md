<!-- keywords: pitfalls, invariants, restrictions, caveats -->
# Quirks

## Frame management

- **All rendering must be inside `nvgBeginFrame()` / `nvgEndFrame()`**. Calling
  path or text functions outside this pair produces undefined behavior.
- `nvgEndFrame()` flushes the draw buffer and resets the font texture cache.
  Use `nvgFlushFrame()` if you need to flush without discarding cached glyphs.
- `nvgChangeFrame()` changes the viewport mid-frame without resetting state.
  It must be preceded by `nvgBeginFrame` and followed by `nvgEndFrame`.

## Context types

- `NVG_PATH_CONTEXT` cannot save/restore state (`nvgSave`/`nvgRestore`).
- `NVG_FONT_CONTEXT` cannot draw paths or images — only text metrics and
  glyph positions are available. It does not need a GL context.
- `NVG_PATH_ONLY` and `NVG_FONT_ONLY` are mutually exclusive.

## Text & fonts

- `nvgGetFontHandle()` returns the `FT_Face` or `stbtt_fontinfo*` for a font.
  After calling it, subsequent font state changes (`nvgFontSize`, `nvgTextAlign`,
  etc.) **do not take effect** until you call `nvgGetFontHandle` or any other
  font querying function again (`src/nanovg.h:881-886`).
- Font measure functions return values in **local space** (unaffected by current
  transform), because glyph positions are snapped to pixels internally.
- Only solid color fill is supported for text (`src/nanovg.h:721`).
- For ClearType: call `nvgTextBackgroundColor()` **before** `nvgTextColor()`,
  and set background color **after** `nvgTextColor`. See `src/nanovg.h:329-339`.
- ClearType requires freetype (default). The shader uses `dFdx`, which may not
  work on GLES2. The `CLEARTYPE-INTEGER-X.md` file documents a workaround.

## Image & texture ownership

- `nvgForgetImageTexture()` prevents the texture from being deleted when
  `nvgDeleteImage()` is called. Use this when sharing textures across contexts
  (`src/nanovg.h:528-533`).
- `nvgCreateImageTexture()` allows sharing textures between contexts, but the
  windows these contexts belong to must have been created with object sharing
  enabled (`src/nanovg.h:519-523`).
- `NVG_IMAGE_RENDER_TARGET` flag must be set for images used as Vulkan render
  targets (adds `COLOR_ATTACHMENT` usage). See `src/nanovg.h:178`.

## Path rendering

- NanoVG uses the **even-odd fill rule**. Sub-paths default to CCW winding.
  Mark holes with `nvgPathWinding(vg, NVG_HOLE)` **after** defining the path.
- `nvgCopyPath(ctx, NULL)` is equivalent to `nvgBeginPath(ctx)`.
- `nvgPrecompilePath()` only works on `NVG_PATH_ONLY` contexts and caches
  tessellation data for later reuse via `nvgCopyPath`.

## Framebuffers

- Framebuffer creation requires a valid GL context (or rlgl context). The
  image backing the FBO is created with `NVG_IMAGE_FLIPY` and
  `NVG_IMAGE_PREMULTIPLIED` automatically (see
  `src/nanovg_gl_utils.h:122` and `src/nanovg_rlgl_utils.h:49`).
- Stencil buffer is required. The FBO creation tries `GL_STENCIL_INDEX8` first,
  then falls back to `GL_DEPTH24_STENCIL8` (see `src/nanovg_rlgl_utils.h:65-78`).
- `NVG_IMAGE_RBO_LESS` flag skips renderbuffer allocation (useful when depth is
  not needed).

## State handling

- `nvgSave()`/`nvgRestore()` manage a stack. Default max depth is 32 states
  (`NVG_MAX_STATES` in `src/nanovg_gl.h:48`). Minimum is 1.
- `nvgReset()` resets to defaults without affecting the stack.

## Bezier tessellation

- Two algorithms: `NVG_TESS_SUBDIVISION` (De Casteljau, default) and
  `NVG_TESS_AFD` (Adaptive Forward Differencing, better performance).
  Switch with `nvgBezierTessellation()`.

## Debugging

- Add `NVG_DEBUG` to create flags for extra error checks.
- `nvgTrace()` directs a log of all API calls to a `FILE*`. Use
  `NVG_NO_DRAWING` flag with tracing for a dry-run mode that exercises paths
  and text layout without issuing GPU commands.
- The `nvgDebugDumpPathCache()` function dumps cached path data to stderr.
