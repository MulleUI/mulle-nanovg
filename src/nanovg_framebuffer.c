//
// nanovg_framebuffer.c - Unified framebuffer API implementation
//
// Provides a consistent framebuffer API across all backends
//

#include "nanovg.h"
#include <stdint.h>

// Unified framebuffer type
typedef int NVGframebuffer;

// Only compile if a backend is defined
#if defined(NANOVG_GL3) || defined(NANOVG_GL2) || defined(NANOVG_GLES2) || defined(NANOVG_GLES3) || defined(__APPLE__) || defined(_WIN32)

#if defined(NANOVG_GL3) || defined(NANOVG_GL2) || defined(NANOVG_GLES2) || defined(NANOVG_GLES3)
// OpenGL: convert pointer ↔ int handle
#include "nanovg_gl_utils.h"

NVGframebuffer nvgCreateFramebuffer(struct NVGcontext* ctx, int w, int h, int flags)
{
    NVGLUframebuffer* fb = nvgluCreateFramebuffer(ctx, w, h, flags);
    return (NVGframebuffer)(intptr_t)fb;
}

void nvgBindFramebuffer(struct NVGcontext* ctx, NVGframebuffer fb)
{
    nvgluBindFramebuffer((NVGLUframebuffer*)(intptr_t)fb);
}

void nvgDeleteFramebuffer(struct NVGcontext* ctx, NVGframebuffer fb)
{
    nvgluDeleteFramebuffer((NVGLUframebuffer*)(intptr_t)fb);
}

// Unified pixel reading
void nvgReadPixels(struct NVGcontext* ctx, int image, int x, int y, int width, int height, void* data)
{
    // OpenGL reads from currently bound framebuffer
    if (image != 0) {
        nvgluBindFramebuffer((NVGLUframebuffer*)(intptr_t)image);
    }
    
    glReadPixels(x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
    
    if (image != 0) {
        nvgluBindFramebuffer(NULL);
    }
}

#elif defined(__APPLE__)
// Metal: already int-based, just redirect
#include "nanovg_mtl.h"

NVGframebuffer nvgCreateFramebuffer(struct NVGcontext* ctx, int w, int h, int flags)
{
    return mnvgCreateFramebuffer(ctx, w, h, flags);
}

void nvgBindFramebuffer(struct NVGcontext* ctx, NVGframebuffer fb)
{
    mnvgBindFramebuffer(ctx, fb);
}

void nvgDeleteFramebuffer(struct NVGcontext* ctx, NVGframebuffer fb)
{
    nvgDeleteImage(ctx, fb);
}

// Unified pixel reading
void nvgReadPixels(struct NVGcontext* ctx, int image, int x, int y, int width, int height, void* data)
{
    mnvgReadPixels(ctx, image, x, y, width, height, data);
}

#elif defined(_WIN32)
// DX11: already int-based, just redirect
#include "nanovg_d3d11.h"

NVGframebuffer nvgCreateFramebuffer(struct NVGcontext* ctx, int w, int h, int flags)
{
    return d3dnvgCreateFramebuffer(ctx, w, h, flags);
}

void nvgBindFramebuffer(struct NVGcontext* ctx, NVGframebuffer fb)
{
    d3dnvgBindFramebuffer(ctx, fb);
}

void nvgDeleteFramebuffer(struct NVGcontext* ctx, NVGframebuffer fb)
{
    nvgDeleteImage(ctx, fb);
}

// Unified pixel reading
void nvgReadPixels(struct NVGcontext* ctx, int image, int x, int y, int width, int height, void* data)
{
    d3dnvgReadPixels(ctx, image, x, y, width, height, data);
}

#endif

#endif // backend defined
