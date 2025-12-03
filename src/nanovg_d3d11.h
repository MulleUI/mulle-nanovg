//
// Copyright (c) 2009-2013 Mikko Mononen memon@inside.org
// Port of _gl.h to _d3d11.h by Chris Maughan
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//
#ifndef NANOVG_D3D11_H
#define NANOVG_D3D11_H

#ifdef __cplusplus
extern "C" {
#endif

// Flag indicating if geoemtry based anti-aliasing is used (may not be needed when using MSAA).
#define NVG_ANTIALIAS 1

// Flag indicating if strokes should be drawn using stencil buffer. The rendering will be a little
// slower, but path overlaps (i.e. self-intersecting or sharp turns) will be drawn just once.
#define NVG_STENCIL_STROKES 2

// Can't forward declare struct ID3D11Device because Windows typedef it...
struct NVGcontext* nvgCreateD3D11(void* ID3D11Device_pDevice, int edgeaa);
void nvgDeleteD3D11(struct NVGcontext* ctx);

// Framebuffer support
NVGframebuffer d3dnvgCreateFramebuffer(struct NVGcontext* ctx, int w, int h, int flags);
void d3dnvgBindFramebuffer(struct NVGcontext* ctx, NVGframebuffer fb);

// Optional utilities
void d3dnvgCopyImage(struct NVGcontext* ctx, int imgDest, int imgSrc);
void d3dnvgWriteImage(struct NVGcontext* ctx, int imgDest, void* data);
void d3dnvgReadPixels(struct NVGcontext* ctx, int image, int x, int y, int width, int height, void* data);

// These are additional flags on top of NVGimageFlags.
enum NVGimageFlagsD3D11 {
	NVG_IMAGE_NODELETE          = 1<<16, // Do not delete texture object.
	NVG_IMAGE_RENDER_TARGET     = 1<<17,
	NVG_IMAGE_USAGE_DYNAMIC     = 1<<18,
	NVG_IMAGE_USAGE_STAGING     = 1<<19,
};

#ifdef __cplusplus
}
#endif

#endif //NANOVG_D3D11_H// Implementation in nanovg_d3d11.c
#endif //NANOVG_D3D11_H
