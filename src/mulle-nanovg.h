//
// mulle-nanovg.h - Unified NanoVG header with multiple backends
//

#ifndef MULLE_NANOVG_H
#define MULLE_NANOVG_H

#ifdef __cplusplus
extern "C" {
#endif

// Core NanoVG API
#include "nanovg.h"

// OpenGL backend - only include if OpenGL is available
// Don't include by default to avoid conflicts
// User must define NANOVG_GL2, NANOVG_GL3, NANOVG_GLES2, or NANOVG_GLES3
#if defined(NANOVG_GL2) || defined(NANOVG_GL3) || defined(NANOVG_GLES2) || defined(NANOVG_GLES3)
  #include "nanovg_gl.h"
  #include "nanovg_gl_utils.h"
#endif

// Platform-specific backends
#ifdef __APPLE__
  #include "nanovg_mtl.h"
#endif

#ifdef _WIN32
  #include "nanovg_d3d11.h"
#endif

// Vulkan backend
#ifdef NANOVG_VULKAN
  #include "nanovg_vk.h"
  #include "nanovg_vk_utils.h"
#endif

// Unified framebuffer API
typedef void* NVGframebuffer;

NVGframebuffer nvgCreateFramebuffer(struct NVGcontext* ctx, int w, int h, int flags);
void nvgBindFramebuffer(struct NVGcontext* ctx, NVGframebuffer fb);
void nvgDeleteFramebuffer(struct NVGcontext* ctx, NVGframebuffer fb);

// Unified pixel reading (reads RGBA8 from image/framebuffer)
void nvgReadPixels(struct NVGcontext* ctx, int image, int x, int y, int width, int height, void* data);

#ifdef __cplusplus
}
#endif

#endif // MULLE_NANOVG_H
