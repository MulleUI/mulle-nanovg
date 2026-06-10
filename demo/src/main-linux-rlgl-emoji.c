// Linux emoji demo - tests color emoji rendering with NotoColorEmoji.ttf

#include <stdio.h>
#include <math.h>

#define GRAPHICS_API_OPENGL_33
#define RLGL_IMPLEMENTATION
#include <mulle-rlgl.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#define NANOVG_RLGL_IMPLEMENTATION
#include <nanovg_rlgl.h>


int main( void)
{
   GLFWwindow    *window;
   NVGcontext    *vg;
   int           fbHeight, fbWidth, winHeight, winWidth;
   int           font, emojiFont;
   float         pxRatio;

   if( !glfwInit())
   {
      printf( "Failed to init GLFW.\n");
      return( -1);
   }

   glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3);
   glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3);
   glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
   glfwWindowHint( GLFW_STENCIL_BITS, 8);

   window = glfwCreateWindow( 800, 600, "NanoVG Emoji Demo", NULL, NULL);
   if( !window)
   {
      printf( "Failed to create window.\n");
      glfwTerminate();
      return( -1);
   }

   glfwMakeContextCurrent( window);
   rlLoadExtensions( (void *) glfwGetProcAddress);
   rlglInit( 800, 600);

   vg = nvgCreateRLGL( NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_EMOJI | NVG_CLEARTYPE);
   if( vg == NULL)
   {
      printf( "Could not init nanovg.\n");
      return( -1);
   }

   font = nvgCreateFont( vg, "sans", "Roboto-Regular.ttf");
   if( font == -1)
   {
      printf( "Could not load sans font.\n");
      return( -1);
   }

   emojiFont = nvgCreateFont( vg, "emoji", "NotoColorEmoji.ttf");
   if( emojiFont == -1)
   {
      printf( "Could not load emoji font.\n");
      return( -1);
   }

   nvgSetEmojiFontId( vg, emojiFont);

   glfwSetTime( 0);

   while( !glfwWindowShouldClose( window))
   {
      glfwGetWindowSize( window, &winWidth, &winHeight);
      glfwGetFramebufferSize( window, &fbWidth, &fbHeight);
      pxRatio = (float) fbWidth / (float) winWidth;

      glViewport( 0, 0, fbWidth, fbHeight);
      glClearColor( 0.2f, 0.2f, 0.2f, 1.0f);
      glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

      nvgBeginFrame( vg, winWidth, winHeight, pxRatio);

      // Regular text
      nvgFontSize( vg, 36.0f);
      nvgFontFace( vg, "sans");
      nvgFillColor( vg, nvgRGBA( 255, 255, 255, 255));
      nvgText( vg, 50, 80, "Hello World!", NULL);

      // Text with emoji intermixed
      nvgFontSize( vg, 36.0f);
      nvgText( vg, 50, 140, "Emoji: \xF0\x9F\x98\x80 \xF0\x9F\x8E\x89 \xF0\x9F\x9A\x80", NULL);  // 😀 🎉 🚀

      // Larger emoji
      nvgFontSize( vg, 64.0f);
      nvgText( vg, 50, 230, "\xF0\x9F\x98\x80\xF0\x9F\x98\x8D\xF0\x9F\xA4\x94\xF0\x9F\x91\x8D", NULL);  // 😀😍🤔👍

      // Mixed text and emoji at different sizes
      nvgFontSize( vg, 24.0f);
      nvgText( vg, 50, 300, "Mixed: A\xF0\x9F\x98\x80" "B\xF0\x9F\x8E\x89" "C", NULL);  // Mixed: A😀B🎉C

      // Size scaling test
      nvgFontSize( vg, 16.0f);
      nvgText( vg, 50, 350, "16px: \xF0\x9F\x98\x80", NULL);

      nvgFontSize( vg, 32.0f);
      nvgText( vg, 200, 350, "32px: \xF0\x9F\x98\x80", NULL);

      nvgFontSize( vg, 48.0f);
      nvgText( vg, 400, 350, "48px: \xF0\x9F\x98\x80", NULL);

      // ClearType section
      nvgClearType( vg, 1);
      nvgTextBackgroundColor( vg, nvgRGBAf( 0.2f, 0.2f, 0.2f, 1.0f));
      nvgFontSize( vg, 36.0f);
      nvgTextColor( vg, nvgRGBA( 200, 220, 255, 255));
      nvgText( vg, 50, 420, "ClearType: Hello \xF0\x9F\x98\x80 World!", NULL);
      nvgClearType( vg, 0);

      nvgEndFrame( vg);

      glfwSwapBuffers( window);
      glfwPollEvents();
   }

   nvgDeleteRLGL( vg);
   glfwTerminate();
   return( 0);
}
