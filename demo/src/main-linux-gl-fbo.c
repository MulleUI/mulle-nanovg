//
// demo-linux-gl-fbo.c - Framebuffer test for Linux/GLES3
//

#include <stdio.h>
#include <math.h>
#include <stdint.h>

#define GLFW_INCLUDE_ES3
#include <GLFW/glfw3.h>

#define NANOVG_GLES3_IMPLEMENTATION
#include <nanovg.h>
#include <nanovg_gl.h>
#include <nanovg_gl_utils.h>

static void renderPattern(NVGcontext* vg, NVGframebuffer fb, float t, float pxRatio)
{
   float   r;
   float   s;
   float   sr;
   int     fboHeight;
   int     fboWidth;
   int     ph;
   int     pw;
   int     winHeight;
   int     winWidth;
   int     x;
   int     y;

   if( fb == NULL)
      return;

   s  = 20.0f;
   sr = (cosf( t) + 1) * 0.5f;
   r  = s * 0.6f * (0.2f + 0.8f * sr);

   fboWidth  = (int) (100 * pxRatio);
   fboHeight = (int) (100 * pxRatio);
   winWidth  = (int) (fboWidth / pxRatio);
   winHeight = (int) (fboHeight / pxRatio);

   nvgluBindFramebuffer( fb);
   glViewport( 0, 0, fboWidth, fboHeight);
   glClearColor( 0, 0, 0, 0);
   glClear( GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
   nvgBeginFrame( vg, winWidth, winHeight, pxRatio);

   pw = (int) ceilf( winWidth / s);
   ph = (int) ceilf( winHeight / s);

   nvgBeginPath( vg);
   for( y = 0; y < ph; y++)
   {
      for( x = 0; x < pw; x++)
      {
         float   cx;
         float   cy;

         cx = (x + 0.5f) * s;
         cy = (y + 0.5f) * s;
         nvgCircle( vg, cx, cy, r);
      }
   }
   nvgFillColor( vg, nvgRGBA( 220, 160, 0, 200));
   nvgFill( vg);

   nvgEndFrame( vg);
   nvgluBindFramebuffer( NULL);
}

int main(void)
{
   GLFWwindow     *window;
   NVGcontext     *vg;
   NVGframebuffer fb;
   double         prevt;
   float          pxRatio;
   int            fbHeight;
   int            fbWidth;
   int            winHeight;
   int            winWidth;

   if( !glfwInit())
   {
      printf( "Failed to init GLFW.\n");
      return( -1);
   }

   glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3);
   glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 0);
   glfwWindowHint( GLFW_CLIENT_API, GLFW_OPENGL_ES_API);

   window = glfwCreateWindow( 1000, 600, "NanoVG FBO Test (GLES3)", NULL, NULL);
   if( !window)
   {
      glfwTerminate();
      return( -1);
   }

   glfwMakeContextCurrent( window);

   vg = nvgCreateGLES3( NVG_ANTIALIAS | NVG_STENCIL_STROKES);
   if( !vg)
   {
      printf( "Could not init nanovg.\n");
      return( -1);
   }

   glfwGetWindowSize( window, &winWidth, &winHeight);
   glfwGetFramebufferSize( window, &fbWidth, &fbHeight);
   pxRatio = (float) fbWidth / (float) winWidth;

   fb = nvgluCreateFramebuffer( vg, (int) (100 * pxRatio), (int) (100 * pxRatio), NVG_IMAGE_REPEATX | NVG_IMAGE_REPEATY);
   if( fb == NULL)
   {
      printf( "Could not create FBO.\n");
      return( -1);
   }

   glfwSwapInterval( 0);
   glfwSetTime( 0);
   prevt = glfwGetTime();

   while( !glfwWindowShouldClose( window))
   {
      double   mx;
      double   my;
      double   t;
      int      i;

      t = glfwGetTime();

      glfwGetCursorPos( window, &mx, &my);
      glfwGetWindowSize( window, &winWidth, &winHeight);
      glfwGetFramebufferSize( window, &fbWidth, &fbHeight);
      pxRatio = (float) fbWidth / (float) winWidth;

      renderPattern( vg, fb, t, pxRatio);

      glViewport( 0, 0, fbWidth, fbHeight);
      glClearColor( 0.3f, 0.3f, 0.32f, 1.0f);
      glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

      nvgBeginFrame( vg, winWidth, winHeight, pxRatio);

      for( i = 0; i < 20; i++)
      {
         nvgBeginPath( vg);
         nvgRect( vg, 10 + i * 30, 10, 10, winHeight - 20);
         nvgFillColor( vg, nvgHSLA( i / 19.0f, 0.5f, 0.5f, 255));
         nvgFill( vg);
      }

      nvgBeginPath( vg);
      nvgRoundedRect( vg, 140 + sinf( t * 1.3f) * 100, 140 + cosf( t * 1.71244f) * 100, 250, 250, 20);
      nvgFillPaint( vg, nvgImagePattern( vg, 0, 0, 100, 100, 0, ((NVGLUframebuffer*)fb)->image, 1.0f));
      nvgFill( vg);

      nvgEndFrame( vg);

      glfwSwapBuffers( window);
      glfwPollEvents();
   }

   nvgluDeleteFramebuffer( fb);
   nvgDeleteGLES3( vg);
   glfwTerminate();
   return( 0);
}
