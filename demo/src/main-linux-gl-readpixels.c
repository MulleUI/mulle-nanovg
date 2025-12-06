// OpenGL ReadPixels test - render to FBO, read pixels, save to PNG

#include <stdio.h>
#include <stdlib.h>

#define GLFW_INCLUDE_ES3
#include <GLFW/glfw3.h>

#define NANOVG_GLES3_IMPLEMENTATION
#include <nanovg.h>
#include <nanovg_gl.h>
#include <nanovg_gl_utils.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int main(void)
{
   GLFWwindow        *window;
   NVGcontext        *vg;
   NVGframebuffer    fb;
   int               fbSize;
   unsigned char     *pixels;

   fbSize = 512;

   if( !glfwInit())
   {
      printf( "Failed to init GLFW.\n");
      return( -1);
   }

   glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3);
   glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 0);
   glfwWindowHint( GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
   glfwWindowHint( GLFW_VISIBLE, GLFW_FALSE);

   window = glfwCreateWindow( 100, 100, "ReadPixels Test", NULL, NULL);
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

   printf( "Creating %dx%d framebuffer...\n", fbSize, fbSize);
   fb = nvgluCreateFramebuffer( vg, fbSize, fbSize, 0);
   if( fb == NULL)
   {
      printf( "Could not create FBO.\n");
      return( -1);
   }

   nvgluBindFramebuffer( fb);
   glViewport( 0, 0, fbSize, fbSize);
   glClearColor( 0.2f, 0.2f, 0.3f, 1.0f);
   glClear( GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

   printf( "Rendering to framebuffer...\n");
   nvgBeginFrame( vg, fbSize, fbSize, 1.0f);

   nvgBeginPath( vg);
   nvgCircle( vg, fbSize/2, fbSize/2, 100);
   nvgFillColor( vg, nvgRGBA( 255, 192, 0, 255));
   nvgFill( vg);

   nvgBeginPath( vg);
   nvgRect( vg, 50, 50, 150, 150);
   nvgStrokeColor( vg, nvgRGBA( 255, 255, 255, 255));
   nvgStrokeWidth( vg, 5.0f);
   nvgStroke( vg);

   nvgEndFrame( vg);

   printf( "Reading pixels...\n");
   pixels = (unsigned char*)malloc( fbSize * fbSize * 4);
   glnvgReadPixels( vg, fb, 0, 0, fbSize, fbSize, pixels);

   nvgluBindFramebuffer( NULL);

   printf( "Saving to gl-output.png...\n");
   // Use negative stride to flip image (OpenGL reads bottom-to-top, PNG is top-to-bottom)
   stbi_write_png( "gl-output.png", fbSize, fbSize, 4, pixels + (fbSize - 1) * fbSize * 4, -fbSize * 4);

   free( pixels);
   nvgluDeleteFramebuffer( fb);
   nvgDeleteGLES3( vg);
   glfwTerminate();
   
   printf( "Done! Check gl-output.png\n");
   return( 0);
}
