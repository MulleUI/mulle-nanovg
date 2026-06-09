// Linux demo - rlgl backend (Desktop OpenGL 3.3)

#define GRAPHICS_API_OPENGL_33
#define RLGL_IMPLEMENTATION
#include <mulle-rlgl.h>

// Include GLFW after rlgl (which loads glad.h with GL headers)
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

   glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3);
   glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3);
   glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
   glfwWindowHint( GLFW_STENCIL_BITS, 8);

   window = glfwCreateWindow( 1000, 600, "NanoVG Demo - rlgl GL3.3", NULL, NULL);
   if( !window)
   {
      printf( "Failed to create window.\n");
      glfwTerminate();
      return( -1);
   }

   glfwMakeContextCurrent( window);

   // Load GL function pointers via glad (required for desktop GL)
   rlLoadExtensions( (void *) glfwGetProcAddress);
   rlglInit( 1000, 600);

   vg = nvgCreateRLGL( NVG_ANTIALIAS | NVG_STENCIL_STROKES);
   if( vg == NULL)
   {
      printf( "Could not init nanovg.\n");
      return( -1);
   }
   printf( "NanoVG rlgl GL3.3 context created successfully\n");

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
