// Render text + emoji to FBO, save PNG for inspection

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
   int             fbSize;
   int             font, emojiFont;
   unsigned char   *pixels;

   fbSize = 512;

   if( !glfwInit())
      return( -1);

   glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3);
   glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3);
   glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
   glfwWindowHint( GLFW_VISIBLE, GLFW_FALSE);

   window = glfwCreateWindow( 100, 100, "emoji png", NULL, NULL);
   if( !window)
   {
      glfwTerminate();
      return( -1);
   }

   glfwMakeContextCurrent( window);
   rlLoadExtensions( (void *) glfwGetProcAddress);
   rlglInit( 100, 100);

   vg = nvgCreateRLGL( NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_EMOJI);
   if( !vg)
      return( -1);

   font = nvgCreateFont( vg, "sans", "Roboto-Regular.ttf");
   printf( "font=%d\n", font);
   emojiFont = nvgCreateFont( vg, "emoji", "NotoColorEmoji.ttf");
   printf( "emojiFont=%d\n", emojiFont);
   if( emojiFont >= 0)
      nvgSetEmojiFontId( vg, emojiFont);

   fb = nvgluCreateFramebufferRLGL( vg, fbSize, fbSize, 0);
   if( fb == NULL)
      return( -1);

   nvgluBindFramebufferRLGL( fb);
   rlViewport( 0, 0, fbSize, fbSize);
   rlClearColor( 51, 51, 77, 255);
   rlClearScreenBuffers();

   nvgBeginFrame( vg, fbSize, fbSize, 1.0f);

   nvgFontSize( vg, 36.0f);
   nvgFontFace( vg, "sans");
   nvgTextColor( vg, nvgRGBA( 255, 255, 255, 255));
   nvgTextBackgroundColor( vg, nvgRGBA( 51, 51, 77, 255));
   nvgText( vg, 30, 80, "Hello World!", NULL);
   nvgText( vg, 30, 140, "Emoji: \xF0\x9F\x98\x80 \xF0\x9F\x8E\x89", NULL);

   nvgEndFrame( vg);

   pixels = (unsigned char *) malloc( fbSize * fbSize * 4);
   nvgluReadPixelsRLGL( vg, fb, 0, 0, fbSize, fbSize, pixels);
   nvgluBindFramebufferRLGL( NULL);

   stbi_write_png( "emoji-output.png", fbSize, fbSize, 4,
                   pixels + (fbSize - 1) * fbSize * 4, -fbSize * 4);

   free( pixels);
   nvgluDeleteFramebufferRLGL( fb);
   nvgDeleteRLGL( vg);
   rlglClose();
   glfwTerminate();
   printf( "Done! emoji-output.png\n");
   return( 0);
}
