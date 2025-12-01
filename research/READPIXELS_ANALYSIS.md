# ReadPixels Abstraction Analysis

## Current State

### Metal (mnvgReadPixels)
```c
void mnvgReadPixels(NVGcontext* ctx, int image, int x, int y, int width, int height, void* data);
```
- Reads from an **image/texture**
- Format: RGBA8 (assumed)

### DX11 (d3dnvgReadPixels)
```c
void d3dnvgReadPixels(struct NVGcontext* ctx, int image, int x, int y, int width, int height, void* data);
```
- Reads from an **image/texture**
- Format: RGBA8 (assumed)
- **IDENTICAL signature to Metal!**

### OpenGL (glReadPixels)
```c
void glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void* data);
```
- Reads from **current framebuffer** (not an image)
- Format: Must specify (GL_RGBA, GL_UNSIGNED_BYTE, etc.)
- No context parameter (uses global GL state)

## Complexity Assessment

### ✅ EASY: Metal + DX11 are already unified!
- **Identical signatures**
- Both read from image handles
- Both assume RGBA8 format
- Both need NVGcontext

### ⚠️ MODERATE: OpenGL is different

**Differences**:
1. **No image parameter** - reads from bound framebuffer
2. **Format parameters** - needs GL_RGBA, GL_UNSIGNED_BYTE
3. **No context** - uses global state

**To unify, we need**:
- Wrapper that takes image handle
- Bind framebuffer before reading
- Hard-code format to RGBA8

## Proposed Unified API

```c
// In mulle-nanovg.h
void nvgReadPixels(NVGcontext* ctx, int image, int x, int y, int width, int height, void* data);
```

**Semantics**:
- Reads RGBA8 pixels from the specified image
- If `image == 0`, reads from main framebuffer
- Output format: RGBA, 8-bit per channel, unpremultiplied

## Implementation Strategy

### Option 1: Simple Redirects (RECOMMENDED)

```c
// In nanovg_framebuffer.c

#if defined(__APPLE__)
void nvgReadPixels(NVGcontext* ctx, int image, int x, int y, int width, int height, void* data)
{
    mnvgReadPixels(ctx, image, x, y, width, height, data);
}

#elif defined(_WIN32)
void nvgReadPixels(NVGcontext* ctx, int image, int x, int y, int width, int height, void* data)
{
    d3dnvgReadPixels(ctx, image, x, y, width, height, data);
}

#elif defined(NANOVG_GL3) || defined(NANOVG_GL2) || defined(NANOVG_GLES2) || defined(NANOVG_GLES3)
void nvgReadPixels(NVGcontext* ctx, int image, int x, int y, int width, int height, void* data)
{
    // Need to bind framebuffer first if reading from image
    if (image != 0) {
        NVGLUframebuffer* fb = (NVGLUframebuffer*)(intptr_t)image;
        nvgluBindFramebuffer(fb);
    }
    
    glReadPixels(x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
    
    // Restore previous framebuffer?
    if (image != 0) {
        nvgluBindFramebuffer(NULL);
    }
}
#endif
```

**Complexity**: ~20 lines of code

### Option 2: Full Abstraction with Format Support

Add format parameter for flexibility:
```c
enum NVGpixelFormat {
    NVG_RGBA8 = 0,
    NVG_RGB8 = 1,
    // etc.
};

void nvgReadPixels(NVGcontext* ctx, int image, int x, int y, int width, int height, 
                   enum NVGpixelFormat format, void* data);
```

**Complexity**: ~50 lines + format conversion logic

## Recommendation

**Use Option 1** - Simple redirects with RGBA8 only:

**Pros**:
- ✅ Minimal code (~20 lines)
- ✅ Metal/DX11 already identical
- ✅ OpenGL wrapper is straightforward
- ✅ Covers 99% of use cases (RGBA8)

**Cons**:
- ⚠️ OpenGL version has framebuffer binding overhead
- ⚠️ No format flexibility (RGBA8 only)

**Complexity Rating**: **2/10** (Very Easy)

## Usage Example

```c
// Read pixels from a framebuffer
NVGframebuffer fb = nvgCreateFramebuffer(vg, 512, 512, 0);
nvgBindFramebuffer(vg, fb);
// ... render ...
nvgBindFramebuffer(vg, 0);

// Read back the pixels
unsigned char pixels[512 * 512 * 4];
nvgReadPixels(vg, fb, 0, 0, 512, 512, pixels);

// Save to file, etc.
```

## Conclusion

**Very feasible!** Metal and DX11 are already unified. OpenGL just needs a thin wrapper to match the API. Total effort: ~20 lines of code.
