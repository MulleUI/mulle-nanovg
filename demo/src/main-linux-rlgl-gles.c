// Linux demo - rlgl backend (GLES3)

#define GRAPHICS_API_OPENGL_ES3
#define RLGL_IMPLEMENTATION
#include <mulle-rlgl.h>

// Include GLFW after rlgl (which loads GL headers)
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "demo.h"

#define NANOVG_RLGL_IMPLEMENTATION
#include <nanovg_rlgl.h>


int main( int argc, char** argv)
{
   DemoData    data;
   GLFWwindow  *window;
   NVGcontext  *vg;
   double      dt, mx, my, prevt, t;
   float       pxRatio;
   int         fbHeight, fbWidth, winHeight, winWidth;

   vg    = NULL;
   prevt = 0;

   if( !glfwInit())
   {
      printf( "Failed to init GLFW.\n");
      return( -1);
   }

   glfwWindowHint( GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
   glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3);
   glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 0);
   glfwWindowHint( GLFW_STENCIL_BITS, 8);

   window = glfwCreateWindow( 1000, 600, "NanoVG Demo - rlgl", NULL, NULL);
   if( !window)
   {
      glfwTerminate();
      return( -1);
   }

   glfwMakeContextCurrent( window);

   // Initialize rlgl on the GLFW-created GL context
   rlglInit( 1000, 600);

   vg = nvgCreateRLGL( NVG_ANTIALIAS | NVG_STENCIL_STROKES);
   if( vg == NULL)
   {
      printf( "Could not init nanovg.\n");
      return( -1);
   }
   printf( "NanoVG rlgl context created successfully\n");

   if( loadDemoData( vg, &data) == -1)
   {
      printf( "Could not load demo data.\n");
      return( -1);
   }

   glfwSetTime( 0);
   prevt = glfwGetTime();

   while( !glfwWindowShouldClose( window))
   {
      t  = glfwGetTime();
      dt = t - prevt;
      prevt = t;

      glfwGetCursorPos( window, &mx, &my);
      glfwGetWindowSize( window, &winWidth, &winHeight);
      glfwGetFramebufferSize( window, &fbWidth, &fbHeight);
      pxRatio = (float) fbWidth / (float) winWidth;

      rlViewport( 0, 0, fbWidth, fbHeight);
      rlClearColor( 77, 77, 82, 255);
      rlClearScreenBuffers();

      nvgBeginFrame( vg, winWidth, winHeight, pxRatio);
      renderDemo( vg, mx, my, winWidth, winHeight, t, 0, &data);
      nvgEndFrame( vg);

      glfwSwapBuffers( window);
      glfwPollEvents();
   }

   freeDemoData( vg, &data);
   nvgDeleteRLGL( vg);

   rlglClose();
   glfwTerminate();
   return( 0);
}
