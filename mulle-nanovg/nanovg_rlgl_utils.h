//
// nanovg_rlgl_utils.h — Framebuffer utilities for nanovg_rlgl backend
//
// Mirrors nanovg_gl_utils.h API using rlgl framebuffer functions.
//
#ifndef NANOVG_RLGL_UTILS_H
#define NANOVG_RLGL_UTILS_H

#include "nanovg-framebuffer.h"

struct NVGRLGLframebuffer
{
   NVGcontext     *ctx;
   unsigned int   fbo;
   unsigned int   rbo;
   unsigned int   texture;
   int            image;
};
typedef struct NVGRLGLframebuffer NVGRLGLframebuffer;

NVGframebuffer nvgluCreateFramebufferRLGL(NVGcontext *ctx, int w, int h, int imageFlags);
void           nvgluBindFramebufferRLGL(NVGframebuffer fb);
void           nvgluDeleteFramebufferRLGL(NVGframebuffer fb);
void           nvgluReadPixelsRLGL(NVGcontext *ctx, NVGframebuffer fb, int x, int y, int width, int height, void *data);

#endif // NANOVG_RLGL_UTILS_H


#ifdef NANOVG_RLGL_IMPLEMENTATION

#include <stdlib.h>
#include <string.h>

static unsigned int rlglnvg__defaultFBO = 0;

NVGframebuffer nvgluCreateFramebufferRLGL(NVGcontext *ctx, int w, int h, int imageFlags)
{
   NVGRLGLframebuffer  *fb;
   unsigned int        prevFBO;

   prevFBO = rlGetActiveFramebuffer();

   fb = (NVGRLGLframebuffer *) malloc( sizeof( NVGRLGLframebuffer));
   if( fb == NULL)
      return( NULL);
   memset( fb, 0, sizeof( NVGRLGLframebuffer));

   fb->ctx   = ctx;
   fb->image = nvgCreateImageRGBA( ctx, w, h, imageFlags | NVG_IMAGE_FLIPY | NVG_IMAGE_PREMULTIPLIED, NULL);
   if( fb->image <= 0)
      goto error;

   fb->texture = nvglImageHandleRLGL( ctx, fb->image);

   fb->fbo = rlLoadFramebuffer();
   if( fb->fbo == 0)
      goto error;

   rlEnableFramebuffer( fb->fbo);

   // Attach color texture
   rlFramebufferAttach( fb->fbo, fb->texture, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_TEXTURE2D, 0);

   // Attach stencil renderbuffer
   fb->rbo = rlLoadRenderbuffer( w, h, GL_STENCIL_INDEX8);
   if( fb->rbo != 0)
      rlFramebufferAttach( fb->fbo, fb->rbo, RL_ATTACHMENT_STENCIL, RL_ATTACHMENT_RENDERBUFFER, 0);

   if( ! rlFramebufferComplete( fb->fbo))
   {
      // Fallback: try GL_DEPTH24_STENCIL8
      if( fb->rbo != 0)
         rlUnloadRenderbuffer( fb->rbo);
      fb->rbo = rlLoadRenderbuffer( w, h, GL_DEPTH24_STENCIL8);
      if( fb->rbo != 0)
         rlFramebufferAttach( fb->fbo, fb->rbo, RL_ATTACHMENT_STENCIL, RL_ATTACHMENT_RENDERBUFFER, 0);
      if( ! rlFramebufferComplete( fb->fbo))
         goto error;
   }

   rlEnableFramebuffer( prevFBO);
   return( (NVGframebuffer) fb);

error:
   rlEnableFramebuffer( prevFBO);
   nvgluDeleteFramebufferRLGL( (NVGframebuffer) fb);
   return( NULL);
}


void nvgluBindFramebufferRLGL(NVGframebuffer fb)
{
   NVGRLGLframebuffer  *glfb = (NVGRLGLframebuffer *) fb;

   if( glfb != NULL)
      rlEnableFramebuffer( glfb->fbo);
   else
      rlDisableFramebuffer();
}


void nvgluDeleteFramebufferRLGL(NVGframebuffer fb)
{
   NVGRLGLframebuffer  *glfb = (NVGRLGLframebuffer *) fb;

   if( glfb == NULL)
      return;
   if( glfb->fbo != 0)
      rlUnloadFramebuffer( glfb->fbo);
   if( glfb->rbo != 0)
      rlUnloadRenderbuffer( glfb->rbo);
   if( glfb->image >= 0)
      nvgDeleteImage( glfb->ctx, glfb->image);
   free( glfb);
}


void nvgluReadPixelsRLGL(NVGcontext *ctx, NVGframebuffer fb, int x, int y, int width, int height, void *data)
{
   NVGRLGLframebuffer  *glfb = (NVGRLGLframebuffer *) fb;
   unsigned int        prevFBO;

   (void) ctx;

   prevFBO = rlGetActiveFramebuffer();
   if( glfb)
      rlEnableFramebuffer( glfb->fbo);
   rlReadPixels( x, y, width, height, data);
   rlEnableFramebuffer( prevFBO);
}

#endif // NANOVG_RLGL_IMPLEMENTATION
