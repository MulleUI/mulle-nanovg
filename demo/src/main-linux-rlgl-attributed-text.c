// Attributed text demo: shows per-run color and background color changes
// Demonstrates selection highlighting with correct ClearType background hints

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
   char            *text;
   NVGtextAttribute attrs[3];

   w = 400;
   h = 280;

   if( !glfwInit())
      return( -1);

   glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3);
   glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3);
   glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
   glfwWindowHint( GLFW_VISIBLE, GLFW_FALSE);

   window = glfwCreateWindow( 100, 100, "attributed-text", NULL, NULL);
   if( !window) { glfwTerminate(); return( -1); }
   glfwMakeContextCurrent( window);
   rlLoadExtensions( (void *) glfwGetProcAddress);
   rlglInit( 100, 100);

   vg = nvgCreateRLGL( NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_CLEARTYPE | NVG_EMOJI);
   if( !vg) return( -1);

   font = nvgCreateFont( vg, "sans", "Roboto-Regular.ttf");
   if( font < 0) return( -1);

   int italicFont = nvgCreateFont( vg, "sans-italic", "Roboto-Italic.ttf");
   if( italicFont < 0) return( -1);

   {
      int emojiFont = nvgCreateFont( vg, "emoji", "NotoColorEmoji.ttf");
      if( emojiFont >= 0)
         nvgSetEmojiFontId( vg, emojiFont);
   }

   fb = nvgluCreateFramebufferRLGL( vg, w, h, 0);
   if( fb == NULL) return( -1);

   nvgluBindFramebufferRLGL( fb);
   rlViewport( 0, 0, w, h);
   rlClearColor( 40, 40, 40, 255);
   rlClearScreenBuffers();

   nvgBeginFrame( vg, w, h, 1.0f);

   nvgFontSize( vg, 28.0f);
   nvgFontFace( vg, "sans");
   nvgTextAlign( vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
   nvgClearType( vg, 1);

   // --- Line 1: selection highlighting ---
   // "Hello World!" with "World" selected (blue bg)
   text = "Hello World!";
   {
      NVGcolor white   = nvgRGBA( 255, 255, 255, 255);
      NVGcolor darkBg  = nvgRGBAf( 0.16f, 0.16f, 0.16f, 1.0f);
      NVGcolor selText = nvgRGBA( 255, 255, 255, 255);
      NVGcolor selBg   = nvgRGBAf( 0.2f, 0.4f, 0.8f, 1.0f);

      // Draw selection background rect
      float bounds[4];
      nvgFillColor( vg, white);
      nvgTextBackgroundColor( vg, darkBg);

      // Use attributed text: "Hello " | "World" | "!"
      attrs[0].end             = text + 6;   // "Hello "
      attrs[0].textColor       = white;
      attrs[0].backgroundColor = darkBg;
      attrs[0].fontId          = -1;
      attrs[0].fontSize        = -1;
      attrs[0].yOffset         = 0;

      attrs[1].end             = text + 11;  // "World"
      attrs[1].textColor       = selText;
      attrs[1].backgroundColor = selBg;
      attrs[1].fontId          = -1;
      attrs[1].fontSize        = -1;
      attrs[1].yOffset         = 0;

      attrs[2].end             = text + 12;  // "!"
      attrs[2].textColor       = white;
      attrs[2].backgroundColor = darkBg;
      attrs[2].fontId          = -1;
      attrs[2].fontSize        = -1;
      attrs[2].yOffset         = 0;

      // Draw selection highlight rect behind "World"
      nvgTextBounds( vg, 10, 10, text, text + 6, bounds);
      float selX = bounds[2];
      nvgTextBounds( vg, 10, 10, text, text + 11, bounds);
      float selW = bounds[2] - selX;
      nvgTextBounds( vg, 10, 10, text, text + 12, bounds);
      float selH = bounds[3] - bounds[1];

      nvgBeginPath( vg);
      nvgRect( vg, selX, 10, selW, selH);
      nvgFillColor( vg, selBg);
      nvgFill( vg);

      // Render attributed text
      nvgAttributedText( vg, 10, 10, text, NULL, attrs, 3);
   }

   // --- Line 2: multi-color text ---
   text = "Red Green Blue";
   {
      attrs[0].end             = text + 4;   // "Red "
      attrs[0].textColor       = nvgRGBA( 255, 80, 80, 255);
      attrs[0].backgroundColor = nvgRGBAf( 0.16f, 0.16f, 0.16f, 1.0f);
      attrs[0].fontId          = -1;
      attrs[0].fontSize        = -1;
      attrs[0].yOffset         = 0;

      attrs[1].end             = text + 10;  // "Green "
      attrs[1].textColor       = nvgRGBA( 80, 255, 80, 255);
      attrs[1].backgroundColor = nvgRGBAf( 0.16f, 0.16f, 0.16f, 1.0f);
      attrs[1].fontId          = -1;
      attrs[1].fontSize        = -1;
      attrs[1].yOffset         = 0;

      attrs[2].end             = text + 14;  // "Blue"
      attrs[2].textColor       = nvgRGBA( 80, 80, 255, 255);
      attrs[2].backgroundColor = nvgRGBAf( 0.16f, 0.16f, 0.16f, 1.0f);
      attrs[2].fontId          = -1;
      attrs[2].fontSize        = -1;
      attrs[2].yOffset         = 0;

      nvgAttributedText( vg, 10, 50, text, NULL, attrs, 3);
   }

   // --- Line 3: font size variation (superscript) ---
   text = "E=mc2";
   {
      attrs[0].end             = text + 4;   // "E=mc"
      attrs[0].textColor       = nvgRGBA( 255, 255, 255, 255);
      attrs[0].backgroundColor = nvgRGBAf( 0.16f, 0.16f, 0.16f, 1.0f);
      attrs[0].fontId          = -1;
      attrs[0].fontSize        = -1;
      attrs[0].yOffset         = 0;

      attrs[1].end             = text + 5;   // "2" (superscript)
      attrs[1].textColor       = nvgRGBA( 255, 255, 255, 255);
      attrs[1].backgroundColor = nvgRGBAf( 0.16f, 0.16f, 0.16f, 1.0f);
      attrs[1].fontId          = -1;
      attrs[1].fontSize        = 16.0f;
      attrs[1].yOffset         = -12.0f;

      nvgAttributedText( vg, 10, 90, text, text + 5, attrs, 2);
   }

   // --- Line 4: italic font switch ---
   text = "This is italic text";
   {
      NVGcolor white = nvgRGBA( 255, 255, 255, 255);
      NVGcolor darkBg = nvgRGBAf( 0.16f, 0.16f, 0.16f, 1.0f);

      attrs[0].end             = text + 8;   // "This is "
      attrs[0].textColor       = white;
      attrs[0].backgroundColor = darkBg;
      attrs[0].fontId          = -1;
      attrs[0].fontSize        = -1;
      attrs[0].yOffset         = 0;

      attrs[1].end             = text + 14;  // "italic"
      attrs[1].textColor       = nvgRGBA( 255, 200, 100, 255);
      attrs[1].backgroundColor = darkBg;
      attrs[1].fontId          = italicFont;
      attrs[1].fontSize        = -1;
      attrs[1].yOffset         = 0;

      attrs[2].end             = text + 19;  // " text"
      attrs[2].textColor       = white;
      attrs[2].backgroundColor = darkBg;
      attrs[2].fontId          = -1;
      attrs[2].fontSize        = -1;
      attrs[2].yOffset         = 0;

      nvgAttributedText( vg, 10, 130, text, NULL, attrs, 3);
   }

   // --- Line 5: emoji mixed with text ---
   text = "Hello \xF0\x9F\x98\x80 World";  // "Hello 😀 World"
   {
      NVGcolor white = nvgRGBA( 255, 255, 255, 255);
      NVGcolor darkBg = nvgRGBAf( 0.16f, 0.16f, 0.16f, 1.0f);

      attrs[0].end             = text + 6;   // "Hello "
      attrs[0].textColor       = nvgRGBA( 200, 200, 255, 255);
      attrs[0].backgroundColor = darkBg;
      attrs[0].fontId          = -1;
      attrs[0].fontSize        = -1;
      attrs[0].yOffset         = 0;

      attrs[1].end             = text + 10;  // emoji (4 UTF-8 bytes)
      attrs[1].textColor       = white;
      attrs[1].backgroundColor = darkBg;
      attrs[1].fontId          = -1;
      attrs[1].fontSize        = -1;
      attrs[1].yOffset         = 0;

      attrs[2].end             = text + 16;  // " World"
      attrs[2].textColor       = nvgRGBA( 255, 200, 200, 255);
      attrs[2].backgroundColor = darkBg;
      attrs[2].fontId          = -1;
      attrs[2].fontSize        = -1;
      attrs[2].yOffset         = 0;

      nvgAttributedText( vg, 10, 170, text, NULL, attrs, 3);
   }

   // --- Line 6: comparison with plain nvgText ---
   nvgFillColor( vg, nvgRGBA( 128, 128, 128, 255));
   nvgTextBackgroundColor( vg, nvgRGBAf( 0.16f, 0.16f, 0.16f, 1.0f));
   nvgFontSize( vg, 28.0f);
   nvgText( vg, 10, 210, "Plain nvgText for comparison", NULL);

   nvgEndFrame( vg);

   pixels = (unsigned char *) malloc( w * h * 4);
   nvgluReadPixelsRLGL( vg, fb, 0, 0, w, h, pixels);
   nvgluBindFramebufferRLGL( NULL);

   stbi_write_png( "attributed-text-test.png", w, h, 4,
                   pixels + (h - 1) * w * 4, -w * 4);

   free( pixels);
   nvgluDeleteFramebufferRLGL( fb);
   nvgDeleteRLGL( vg);
   rlglClose();
   glfwTerminate();
   printf( "Done! attributed-text-test.png\n");
   return( 0);
}
