//
// main-darwin-mtl-fbo.m - Framebuffer test for macOS/Metal
//
// This demo demonstrates render-to-texture (framebuffer) capability:
//
// WITHOUT FRAMEBUFFER: You can only draw directly to the screen.
//
// WITH FRAMEBUFFER:
//   1. Create an offscreen texture (100x100 pixels)
//   2. Render the pulsing circles pattern into that texture (not to screen)
//   3. Use that texture as a fill pattern for the moving rectangle
//
// WHY THIS MATTERS:
//   - Caching: Render complex content once, reuse it multiple times
//   - Effects: Apply the texture with transformations, repeating patterns, etc.
//   - Post-processing: Render scene to texture, then apply shaders/effects
//   - UI elements: Pre-render widgets, icons, or patterns as textures
//
// In this demo, nvgImagePattern() uses the framebuffer texture and tiles it
// to fill the 250x250 rectangle. The pattern repeats - you see multiple copies
// of the 100x100 circle grid tiled together (NVG_IMAGE_REPEATX | REPEATY flags).
//
// This demonstrates dynamically creating a texture by rendering to it, then
// using that texture as a fill pattern - the core framebuffer use case.
//

#include <stdio.h>
#include <math.h>
#include <stdint.h>

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#import <AppKit/AppKit.h>

#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#define NANOVG_MTL
#include <nanovg.h>
#include <nanovg_mtl.h>

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

   mnvgBindFramebuffer( vg, fb);
   mnvgClearWithColor( vg, nvgRGBA( 0, 0, 0, 0));
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
   mnvgBindFramebuffer( vg, NULL);
}

int main(void)
{
   GLFWwindow     *window;
   NVGcontext     *vg;
   NVGframebuffer fb;
   CAMetalLayer   *metalLayer;
   double         prevt;
   float          pxRatio;
   int            fbHeight;
   int            fbWidth;
   int            winHeight;
   int            winWidth;

   printf( "Starting framebuffer demo...\n");

   if( !glfwInit())
   {
      printf( "Failed to init GLFW.\n");
      return( -1);
   }
   printf( "GLFW initialized\n");

   glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API);
   window = glfwCreateWindow( 1000, 600, "NanoVG FBO Test (Metal)", NULL, NULL);
   if( !window)
   {
      glfwTerminate();
      return( -1);
   }

   NSWindow *nswin = glfwGetCocoaWindow( window);
   NSView *view = [nswin contentView];
   [view setWantsLayer:YES];

   metalLayer = [CAMetalLayer layer];
   metalLayer.device = MTLCreateSystemDefaultDevice();
   metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
   metalLayer.framebufferOnly = YES;
   [view setLayer:metalLayer];

   vg = nvgCreateMTL( (__bridge void*)metalLayer, NVG_ANTIALIAS | NVG_STENCIL_STROKES);
   if( !vg)
   {
      printf( "Could not init nanovg.\n");
      return( -1);
   }
   printf( "NanoVG context created\n");

   glfwGetWindowSize( window, &winWidth, &winHeight);
   glfwGetFramebufferSize( window, &fbWidth, &fbHeight);
   pxRatio = (float) fbWidth / (float) winWidth;
   printf( "Window size: %dx%d, FB size: %dx%d, ratio: %f\n", winWidth, winHeight, fbWidth, fbHeight, pxRatio);

   fb = mnvgCreateFramebuffer( vg, (int) (100 * pxRatio), (int) (100 * pxRatio), NVG_IMAGE_REPEATX | NVG_IMAGE_REPEATY);
   printf( "Framebuffer created: %p\n", fb);
   if( fb == NULL)
   {
      printf( "Could not create FBO.\n");
      return( -1);
   }

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

      metalLayer.drawableSize = CGSizeMake( fbWidth, fbHeight);

      renderPattern( vg, fb, t, pxRatio);

      nvgBeginFrame( vg, winWidth, winHeight, pxRatio);

      // Clear background
      nvgBeginPath( vg);
      nvgRect( vg, 0, 0, winWidth, winHeight);
      nvgFillColor( vg, nvgRGBf( 0.3f, 0.3f, 0.32f));
      nvgFill( vg);

      for( i = 0; i < 20; i++)
      {
         nvgBeginPath( vg);
         nvgRect( vg, 10 + i * 30, 10, 10, winHeight - 20);
         nvgFillColor( vg, nvgHSLA( i / 19.0f, 0.5f, 0.5f, 255));
         nvgFill( vg);
      }

      nvgBeginPath( vg);
      nvgRoundedRect( vg, 140 + sinf( t * 1.3f) * 100, 140 + cosf( t * 1.71244f) * 100, 250, 250, 20);
      nvgFillPaint( vg, nvgImagePattern( vg, 0, 0, 100, 100, 0, (int)(intptr_t)fb, 1.0f));
      nvgFill( vg);

      nvgEndFrame( vg);

      glfwPollEvents();
   }

   nvgDeleteImage( vg, (int)(intptr_t)fb);
   nvgDeleteMTL( vg);
   glfwTerminate();
   return( 0);
}
