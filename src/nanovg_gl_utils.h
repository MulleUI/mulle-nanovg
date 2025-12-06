//
// Copyright (c) 2009-2013 Mikko Mononen memon@inside.org
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
#ifndef NANOVG_GL_UTILS_H
#define NANOVG_GL_UTILS_H

#include "nanovg-framebuffer.h"

// Forward declare struct (defined after header guard)
struct NVGLUframebuffer;
typedef struct NVGLUframebuffer NVGLUframebuffer;

// Helper function to create GL frame buffer to render to.
void nvgluBindReadFramebuffer(NVGframebuffer fb);
void nvgluBindDrawFramebuffer(NVGframebuffer fb);

void nvgluBindFramebuffer(NVGframebuffer fb);
NVGframebuffer nvgluCreateFramebuffer(NVGcontext* ctx, int w, int h, int imageFlags);
void nvgluDeleteFramebuffer(NVGframebuffer fb);
void nvgluClearWithColor(NVGcontext* ctx, NVGcolor color);
void glnvgReadPixels(NVGcontext* ctx, NVGframebuffer fb, int x, int y, int width, int height, void* data);

#endif // NANOVG_GL_UTILS_H

// Struct definition needs to be visible to implementation
#ifndef NVGLUFRAMEBUFFER_STRUCT_DEFINED
#define NVGLUFRAMEBUFFER_STRUCT_DEFINED
struct NVGLUframebuffer {
	NVGcontext* ctx;
	GLuint fbo;
	GLuint rbo;
	GLuint texture;
	int image;
};
typedef struct NVGLUframebuffer NVGLUframebuffer;
#endif

#ifdef NANOVG_GL_IMPLEMENTATION

#include <stdlib.h>
#include <string.h>

#if defined(NANOVG_GL3) || defined(NANOVG_GLES2) || defined(NANOVG_GLES3)
// FBO is core in OpenGL 3>.
#	define NANOVG_FBO_VALID 1
#elif defined(NANOVG_GL2)
// On OS X including glext defines FBO on GL2 too.
#	ifdef __APPLE__
#		include <OpenGL/glext.h>
#		define NANOVG_FBO_VALID 1
#	endif
#endif

static GLint defaultFBO = -1;

//
// The GL_TEXTURE_2D_MULTISAMPLE constant exists in OpenGL and OpenGL ES versions 3.2
// and greater.
// MEMO: this code also exists in nanovg_gl.h
// #if defined( GL_ES_VERSION_3_2) || defined( GL_VERSION_3_2)
// # ifndef __APPLE__
// #  define NVG_TEXTURE_MSAA 4
// #  define NANOVG_HAVE_MSAA
// # endif
// #endif

// https://learnopengl.com/Advanced-OpenGL/Anti-Aliasing

NVGframebuffer nvgluCreateFramebuffer(NVGcontext* ctx, int w, int h, int imageFlags)
{
#ifdef NANOVG_FBO_VALID
	GLint defaultFBO;
	GLint defaultRBO;
	NVGLUframebuffer* fb = NULL;
	int samplerType;

	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &defaultFBO);
	glGetIntegerv(GL_RENDERBUFFER_BINDING, &defaultRBO);

	fb = (NVGLUframebuffer*)malloc(sizeof(NVGLUframebuffer));
	if (fb == NULL) goto error;
	memset(fb, 0, sizeof(NVGLUframebuffer));

	fb->image = nvgCreateImageRGBA(ctx, w, h, imageFlags | NVG_IMAGE_FLIPY | NVG_IMAGE_PREMULTIPLIED, NULL);

#if defined NANOVG_GL2
	fb->texture = nvglImageHandleGL2(ctx, fb->image);
#elif defined NANOVG_GL3
	fb->texture = nvglImageHandleGL3(ctx, fb->image);
#elif defined NANOVG_GLES2
	fb->texture = nvglImageHandleGLES2(ctx, fb->image);
#elif defined NANOVG_GLES3
	fb->texture = nvglImageHandleGLES3(ctx, fb->image);
#endif

#ifdef NANOVG_HAVE_MSAA
	samplerType = (imageFlags & NVG_IMAGE_MSAA)
					  ? GL_TEXTURE_2D_MULTISAMPLE
					  : GL_TEXTURE_2D;
#else
	samplerType = GL_TEXTURE_2D;
#endif

	fb->ctx = ctx;

	// frame buffer object
	glGenFramebuffers(1, &fb->fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fb->fbo);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, samplerType, fb->texture, 0);

	if( ! (imageFlags & NVG_IMAGE_RBO_LESS))
	{
		// render buffer object
		glGenRenderbuffers(1, &fb->rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, fb->rbo);

#ifdef NANOVG_HAVE_MSAA
		if( samplerType == GL_TEXTURE_2D_MULTISAMPLE)
		  glRenderbufferStorageMultisample( GL_RENDERBUFFER, 4, GL_STENCIL_INDEX8, w, h);
		else
#endif
		  glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, w, h);
	  // combine all
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, fb->rbo);
	}

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		if( ! (imageFlags & NVG_IMAGE_RBO_LESS))
		{
#ifdef GL_DEPTH24_STENCIL8
			// If GL_STENCIL_INDEX8 is not supported, try GL_DEPTH24_STENCIL8 as a fallback.
			// Some graphics cards require a depth buffer along with a stencil.
#ifdef NANOVG_HAVE_MSAA
			if( samplerType == GL_TEXTURE_2D_MULTISAMPLE)
			  glRenderbufferStorageMultisample( GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, w, h);
			else
#endif
			  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, samplerType, fb->texture, 0);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, fb->rbo);
			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
#endif // GL_DEPTH24_STENCIL8
				goto error;
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, defaultFBO);
	if( ! (imageFlags & NVG_IMAGE_RBO_LESS))
		glBindRenderbuffer(GL_RENDERBUFFER, defaultRBO);
	return( (NVGframebuffer) fb);
error:
	glBindFramebuffer(GL_FRAMEBUFFER, defaultFBO);
	if( ! (imageFlags & NVG_IMAGE_RBO_LESS))
		glBindRenderbuffer(GL_RENDERBUFFER, defaultRBO);
	nvgluDeleteFramebuffer( (NVGframebuffer) fb);
	return( NULL);
#else
	NVG_NOTUSED(ctx);
	NVG_NOTUSED(w);
	NVG_NOTUSED(h);
	NVG_NOTUSED(imageFlags);
	return( NULL);
#endif
}

void nvgluBindFramebuffer(NVGframebuffer fb)
{
#ifdef NANOVG_FBO_VALID
	NVGLUframebuffer* glfb = (NVGLUframebuffer*)fb;
	if (defaultFBO == -1) glGetIntegerv(GL_FRAMEBUFFER_BINDING, &defaultFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, glfb != NULL ? glfb->fbo : defaultFBO);
#else
	NVG_NOTUSED(fb);
#endif
}

void nvgluBindReadFramebuffer(NVGframebuffer fb)
{
#if (defined( NANOVG_GL3) || defined( NANOVG_GLES3)) && defined( NANOVG_FBO_VALID)
	NVGLUframebuffer* glfb = (NVGLUframebuffer*)fb;
	if (defaultFBO == -1) glGetIntegerv(GL_FRAMEBUFFER_BINDING, &defaultFBO);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, glfb != NULL ? glfb->fbo : defaultFBO);
#else
	NVG_NOTUSED(fb);
#endif
}

void nvgluBindDrawFramebuffer(NVGframebuffer fb)
{
#if (defined( NANOVG_GL3) || defined( NANOVG_GLES3)) && defined( NANOVG_FBO_VALID)
	NVGLUframebuffer* glfb = (NVGLUframebuffer*)fb;
	if (defaultFBO == -1) glGetIntegerv(GL_FRAMEBUFFER_BINDING, &defaultFBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, glfb != NULL ? glfb->fbo : defaultFBO);
#else
	NVG_NOTUSED(fb);
#endif
}


void nvgluDeleteFramebuffer(NVGframebuffer fb)
{
#ifdef NANOVG_FBO_VALID
	NVGLUframebuffer* glfb = (NVGLUframebuffer*)fb;
	if (glfb == NULL) return;
	if (glfb->fbo != 0)
		glDeleteFramebuffers(1, &glfb->fbo);
	if (glfb->rbo != 0)
		glDeleteRenderbuffers(1, &glfb->rbo);
	if (glfb->image >= 0)
		nvgDeleteImage(glfb->ctx, glfb->image);
	glfb->ctx = NULL;
	glfb->fbo = 0;
	glfb->rbo = 0;
	glfb->texture = 0;
	glfb->image = -1;
	free(glfb);
#else
	NVG_NOTUSED(fb);
#endif
}

void nvgluClearWithColor(NVGcontext* ctx, NVGcolor color)
{
#ifdef NANOVG_FBO_VALID
	// Clear both color and stencil buffers
	// This is consistent across all backends (GL, Metal, D3D11)
	// Depth is not cleared as it's not used in 2D rendering
	float alpha = color.a;
	glClearColor(color.r * alpha, color.g * alpha, color.b * alpha, color.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
#else
	NVG_NOTUSED(ctx);
	NVG_NOTUSED(color);
#endif
}

void glnvgReadPixels(NVGcontext* ctx, NVGframebuffer fb, int x, int y, int width, int height, void* data)
{
#ifdef NANOVG_FBO_VALID
	NVGLUframebuffer* glfb = (NVGLUframebuffer*)fb;
	GLint prevFBO;
	
	NVG_NOTUSED(ctx);
	
	if (glfb) {
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, glfb->fbo);
		glReadPixels(x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glBindFramebuffer(GL_FRAMEBUFFER, prevFBO);
	} else {
		// Read from currently bound framebuffer (window)
		glReadPixels(x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
	}
#else
	NVG_NOTUSED(ctx);
	NVG_NOTUSED(fb);
	NVG_NOTUSED(x);
	NVG_NOTUSED(y);
	NVG_NOTUSED(width);
	NVG_NOTUSED(height);
	NVG_NOTUSED(data);
#endif
}

#ifdef NANOVG_GL_UTILS_EXTENSION_SOURCE
# include NANOVG_GL_UTILS_EXTENSION_SOURCE
#endif

#endif // NANOVG_GL_IMPLEMENTATION
