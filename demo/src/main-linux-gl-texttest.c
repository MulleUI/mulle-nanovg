// Gray text baseline test — renders text to FBO, saves PNG (GL backend)

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
   int             fbSize;
   int             font;
   unsigned char   *pixels;

   fbSize = 512;

   if( !glfwInit())
      return( -1);

   glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3);
   glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 0);
   glfwWindowHint( GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
   glfwWindowHint( GLFW_VISIBLE, GLFW_FALSE);

   window = glfwCreateWindow( 100, 100, "texttest", NULL, NULL);
   if( !window)
   {
      glfwTerminate();
      return( -1);
   }

   glfwMakeContextCurrent( window);

   vg = nvgCreateGLES3( NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_EMOJI);
   if( !vg)
      return( -1);

   font = nvgCreateFont( vg, "sans", "Roboto-Regular.ttf");
   printf( "font=%d\n", font);
   if( font < 0)
      return( -1);

   {
      int emojiFont;
      emojiFont = nvgCreateFont( vg, "emoji", "NotoColorEmoji.ttf");
      printf( "emojiFont=%d\n", emojiFont);
      if( emojiFont >= 0)
         nvgSetEmojiFontId( vg, emojiFont);
   }

   fb = nvgluCreateFramebuffer( vg, fbSize, fbSize, 0);
   if( fb == NULL)
      return( -1);

   nvgluBindFramebuffer( fb);
   glViewport( 0, 0, fbSize, fbSize);
   glClearColor( 0.2f, 0.2f, 0.3f, 1.0f);
   glClear( GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

   nvgBeginFrame( vg, fbSize, fbSize, 1.0f);

   nvgFontSize( vg, 48.0f);
   nvgFontFace( vg, "sans");
   nvgFillColor( vg, nvgRGBA( 255, 255, 255, 255));
   nvgTextAlign( vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
   nvgText( vg, 30, 30, "Hello World!", NULL);

   nvgFontSize( vg, 32.0f);
   nvgFillColor( vg, nvgRGBA( 200, 200, 100, 255));
   nvgText( vg, 30, 100, "Gray text baseline OK", NULL);

   // Mixed text + emoji
   nvgFontSize( vg, 36.0f);
   nvgFillColor( vg, nvgRGBA( 255, 255, 255, 255));
   nvgText( vg, 30, 160, "Emoji: \xF0\x9F\x98\x80 \xF0\x9F\x8E\x89", NULL);

   nvgEndFrame( vg);

   pixels = (unsigned char *) malloc( fbSize * fbSize * 4);
   glnvgReadPixels( vg, fb, 0, 0, fbSize, fbSize, pixels);
   nvgluBindFramebuffer( NULL);

   stbi_write_png( "texttest-output.png", fbSize, fbSize, 4,
                   pixels + (fbSize - 1) * fbSize * 4, -fbSize * 4);

   free( pixels);
   nvgluDeleteFramebuffer( fb);
   nvgDeleteGLES3( vg);
   glfwTerminate();
   printf( "Done! texttest-output.png\n");
   return( 0);
}
