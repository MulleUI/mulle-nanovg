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
// User must define NANOVG_GL2, NANOVG_GL3, NANOVG_GLES2, or NANOVG_GLES3
#if defined(NANOVG_GL2) || defined(NANOVG_GL3) || defined(NANOVG_GLES2) || defined(NANOVG_GLES3)
  #include "nanovg_gl.h"
#endif

// Platform-specific backends
#ifdef NANOVG_MTL
  #include "nanovg_mtl.h"
#endif

#ifdef NANOVG_D3D11
  #include "nanovg_d3d11.h"
#endif

// Vulkan backend
#ifdef NANOVG_VULKAN
  #include "nanovg_vk.h"
#endif

#ifdef __cplusplus
}
#endif

#endif // MULLE_NANOVG_H
