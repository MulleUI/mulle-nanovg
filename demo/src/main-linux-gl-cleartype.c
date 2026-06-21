// ClearType verification: render 'A' with and without, dump small PNG

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
   GLFWwindow      *window;
   NVGcontext      *vg;
   NVGframebuffer  fb;
   int             w, h, font;
   unsigned char   *pixels;

   w = 200;
   h = 96;

   if( !glfwInit())
      return( -1);

   glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3);
   glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 0);
   glfwWindowHint( GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
   glfwWindowHint( GLFW_VISIBLE, GLFW_FALSE);

   window = glfwCreateWindow( 100, 100, "ct", NULL, NULL);
   if( !window) { glfwTerminate(); return( -1); }
   glfwMakeContextCurrent( window);

   vg = nvgCreateGLES3( NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_CLEARTYPE | NVG_EMOJI);
   if( !vg) return( -1);

   font = nvgCreateFont( vg, "sans", "Roboto-Regular.ttf");
   if( font < 0) return( -1);

   {
      int emojiFont;
      emojiFont = nvgCreateFont( vg, "emoji", "NotoColorEmoji.ttf");
      if( emojiFont >= 0)
         nvgSetEmojiFontId( vg, emojiFont);
   }

   fb = nvgluCreateFramebuffer( vg, w, h, 0);
   if( fb == NULL) return( -1);

   nvgluBindFramebuffer( fb);
   glViewport( 0, 0, w, h);
   glClearColor( 0.0f, 0.0f, 0.0f, 1.0f);
   glClear( GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

   nvgBeginFrame( vg, w, h, 1.0f);

   nvgFontSize( vg, 32.0f);
   nvgFontFace( vg, "sans");
   nvgFillColor( vg, nvgRGBA( 255, 255, 255, 255));
   nvgTextAlign( vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);

   // Top: no ClearType
   nvgClearType( vg, 0);
   nvgText( vg, 4, 4, "A\xF0\x9F\x98\x80 Hi", NULL);

   // Bottom: ClearType
   nvgClearType( vg, 1);
   nvgTextBackgroundColor( vg, nvgRGBAf( 0.0f, 0.0f, 0.0f, 1.0f));
   nvgText( vg, 4, 52, "A\xF0\x9F\x98\x80 Hi", NULL);

   nvgEndFrame( vg);

   pixels = (unsigned char *) malloc( w * h * 4);
   glnvgReadPixels( vg, fb, 0, 0, w, h, pixels);
   nvgluBindFramebuffer( NULL);

   stbi_write_png( "cleartype-test.png", w, h, 4,
                   pixels + (h - 1) * w * 4, -w * 4);

   free( pixels);
   nvgluDeleteFramebuffer( fb);
   nvgDeleteGLES3( vg);
   glfwTerminate();
   printf( "Done! cleartype-test.png\n");
   return( 0);
}
