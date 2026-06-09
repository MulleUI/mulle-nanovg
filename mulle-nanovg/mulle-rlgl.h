//
// mulle-rlgl.h — rlgl extensions for nanovg
//
// Includes rlgl.h and adds operations it doesn't expose:
// stencil, draw modes (fan/strip), and renderbuffer management.
//
#ifndef MULLE_RLGL_H
#define MULLE_RLGL_H

#include "rlgl.h"

// --- Stencil operations (needed for NVG_STENCIL_STROKES) ---

static inline void rlEnableStencilTest(void)  { glEnable(GL_STENCIL_TEST); }
static inline void rlDisableStencilTest(void) { glDisable(GL_STENCIL_TEST); }

static inline void rlStencilFunc(int func, int ref, unsigned int mask)
{
   glStencilFunc( func, ref, mask);
}

static inline void rlStencilOp(int sfail, int dpfail, int dppass)
{
   glStencilOp( sfail, dpfail, dppass);
}

static inline void rlStencilMask(unsigned int mask)
{
   glStencilMask( mask);
}

static inline void rlStencilOpSeparate(int face, int sfail, int dpfail, int dppass)
{
   glStencilOpSeparate( face, sfail, dpfail, dppass);
}

static inline void rlClearStencil(int s)
{
   glClearStencil( s);
}

// --- Draw modes (rlDrawVertexArray is GL_TRIANGLES only) ---

static inline void rlDrawVertexArrayFan(int offset, int count)
{
   glDrawArrays( GL_TRIANGLE_FAN, offset, count);
}

static inline void rlDrawVertexArrayStrip(int offset, int count)
{
   glDrawArrays( GL_TRIANGLE_STRIP, offset, count);
}

// --- Renderbuffer (for FBO stencil attachment) ---

static inline unsigned int rlLoadRenderbuffer(int width, int height, unsigned int format)
{
   unsigned int id = 0;

   glGenRenderbuffers( 1, &id);
   glBindRenderbuffer( GL_RENDERBUFFER, id);
   glRenderbufferStorage( GL_RENDERBUFFER, format, width, height);
   return( id);
}

static inline void rlUnloadRenderbuffer(unsigned int id)
{
   glDeleteRenderbuffers( 1, &id);
}

// --- glFinish (for sync after init) ---

static inline void rlFinish(void)
{
   glFinish();
}

// --- Upload vertex buffer (realloc + upload, unlike rlUpdateVertexBuffer which is SubData only) ---

static inline void rlUploadVertexBuffer(unsigned int id, const void *data, int dataSize)
{
   glBindBuffer( GL_ARRAY_BUFFER, id);
   glBufferData( GL_ARRAY_BUFFER, dataSize, data, GL_STREAM_DRAW);
}

// --- ReadPixels (for FBO readback) ---

static inline void rlReadPixels(int x, int y, int width, int height, void *data)
{
   glReadPixels( x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
}

#endif // MULLE_RLGL_H
