// ClearType verification (rlgl backend): render with and without CT + emoji

#include <stdio.h>
#include <stdlib.h>

#define GRAPHICS_API_OPENGL_33
#define RLGL_IMPLEMENTATION
#include <mulle-rlgl.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#define NANOVG_RLGL_IMPLEMENTATION
#include <nanovg_rlgl.h>
#include <nanovg_rlgl_utils.h>

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
   glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3);
   glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
   glfwWindowHint( GLFW_VISIBLE, GLFW_FALSE);

   window = glfwCreateWindow( 100, 100, "ct-rlgl", NULL, NULL);
   if( !window) { glfwTerminate(); return( -1); }
   glfwMakeContextCurrent( window);
   rlLoadExtensions( (void *) glfwGetProcAddress);
   rlglInit( 100, 100);

   vg = nvgCreateRLGL( NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_CLEARTYPE | NVG_EMOJI);
   if( !vg) return( -1);

   font = nvgCreateFont( vg, "sans", "Roboto-Regular.ttf");
   if( font < 0) return( -1);

   {
      int emojiFont;
      emojiFont = nvgCreateFont( vg, "emoji", "NotoColorEmoji.ttf");
      if( emojiFont >= 0)
         nvgSetEmojiFontId( vg, emojiFont);
   }

   fb = nvgluCreateFramebufferRLGL( vg, w, h, 0);
   if( fb == NULL) return( -1);

   nvgluBindFramebufferRLGL( fb);
   rlViewport( 0, 0, w, h);
   rlClearColor( 0, 0, 0, 255);
   rlClearScreenBuffers();

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
   nvgluReadPixelsRLGL( vg, fb, 0, 0, w, h, pixels);
   nvgluBindFramebufferRLGL( NULL);

   stbi_write_png( "cleartype-rlgl-test.png", w, h, 4,
                   pixels + (h - 1) * w * 4, -w * 4);

   free( pixels);
   nvgluDeleteFramebufferRLGL( fb);
   nvgDeleteRLGL( vg);
   rlglClose();
   glfwTerminate();
   printf( "Done! cleartype-rlgl-test.png\n");
   return( 0);
}
