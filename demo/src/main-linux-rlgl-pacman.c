// Linux demo - nanovg + rlgl coexistence
// nanovg renders the demo scene, rlgl draws a pac-man chasing the cursor

#include <stdio.h>
#include <math.h>

#define GRAPHICS_API_OPENGL_33
#define RLGL_IMPLEMENTATION
#include <mulle-rlgl.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "demo.h"

#define NANOVG_RLGL_IMPLEMENTATION
#include <nanovg_rlgl.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


// Draw a pac-man (circle with mouth wedge) using rlgl immediate mode
static void drawPacMan(float cx, float cy, float radius, float angle, float mouthAngle, unsigned int texId)
{
   int   i;
   int   segments;
   float startAngle;
   float endAngle;
   float step;
   float a;
   float u, v;

   segments   = 30;
   startAngle = angle + mouthAngle;
   endAngle   = angle + (2.0f * (float) M_PI) - mouthAngle;
   step       = (endAngle - startAngle) / (float) segments;

   // Set up orthographic projection matching window coords
   rlDrawRenderBatchActive();
   rlSetTexture( texId);

   rlBegin( RL_TRIANGLES);
   for( i = 0; i < segments; i++)
   {
      a = startAngle + step * (float) i;

      // Triangle fan from center — texture mapped radially
      rlColor4ub( 255, 220, 0, 255);
      rlTexCoord2f( 0.5f, 0.5f);
      rlVertex2f( cx, cy);

      u = 0.5f + cosf( a) * 0.5f;
      v = 0.5f + sinf( a) * 0.5f;
      rlColor4ub( 255, 200, 0, 255);
      rlTexCoord2f( u, v);
      rlVertex2f( cx + cosf( a) * radius, cy + sinf( a) * radius);

      u = 0.5f + cosf( a + step) * 0.5f;
      v = 0.5f + sinf( a + step) * 0.5f;
      rlTexCoord2f( u, v);
      rlVertex2f( cx + cosf( a + step) * radius, cy + sinf( a + step) * radius);
   }
   rlEnd();

   rlSetTexture( 0);

   // Eye
   rlBegin( RL_TRIANGLES);
   {
      float ex, ey, er;

      ex = cx + cosf( angle + 0.8f) * radius * 0.45f;
      ey = cy + sinf( angle + 0.8f) * radius * 0.45f;
      er = radius * 0.12f;

      for( i = 0; i < 12; i++)
      {
         a = (float) i / 12.0f * 2.0f * (float) M_PI;
         rlColor4ub( 0, 0, 0, 255);
         rlVertex2f( ex, ey);
         rlVertex2f( ex + cosf( a) * er, ey + sinf( a) * er);
         rlVertex2f( ex + cosf( a + (2.0f * (float) M_PI / 12.0f)) * er,
                     ey + sinf( a + (2.0f * (float) M_PI / 12.0f)) * er);
      }
   }
   rlEnd();

   rlDrawRenderBatchActive();
}


// Generate a subtle procedural texture for pac-man (radial gradient with speckle)
static unsigned int createPacManTexture(void)
{
   unsigned char  pixels[64 * 64 * 4];
   unsigned char  v;
   int            x, y, i;
   float          dx, dy, d;

   i = 0;
   for( y = 0; y < 64; y++)
   {
      for( x = 0; x < 64; x++)
      {
         dx = (x - 32.0f) / 32.0f;
         dy = (y - 32.0f) / 32.0f;
         d  = sqrtf( dx * dx + dy * dy);

         // Radial darkening toward edges + subtle speckle
         v = (unsigned char)(255.0f - d * 60.0f - (float)((x * 7 + y * 13) % 11));
         pixels[i++] = v;        // R
         pixels[i++] = (unsigned char)(v * 0.85f);  // G (slightly orange)
         pixels[i++] = 30;       // B
         pixels[i++] = 255;      // A
      }
   }
   return( rlLoadTexture( pixels, 64, 64, RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, 1));
}


int main( int argc, char** argv)
{
   DemoData      data;
   GLFWwindow    *window;
   NVGcontext    *vg;
   unsigned int  pacTex;
   double        mx, my, prevt, t, dt;
   float         pxRatio;
   float         pacX, pacY, pacAngle, mouthAnim;
   int           fbHeight, fbWidth, winHeight, winWidth;

   vg       = NULL;
   prevt    = 0;
   pacX     = 500.0f;
   pacY     = 500.0f;
   pacAngle = 0.0f;

   if( !glfwInit())
   {
      printf( "Failed to init GLFW.\n");
      return( -1);
   }

   glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3);
   glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3);
   glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
   glfwWindowHint( GLFW_STENCIL_BITS, 8);

   window = glfwCreateWindow( 1000, 600, "NanoVG + rlgl Pac-Man", NULL, NULL);
   if( !window)
   {
      printf( "Failed to create window.\n");
      glfwTerminate();
      return( -1);
   }

   glfwMakeContextCurrent( window);
   rlLoadExtensions( (void *) glfwGetProcAddress);
   rlglInit( 1000, 600);

   // Create rlgl texture for pac-man (proves textures coexist with nanovg)
   pacTex = createPacManTexture();

   vg = nvgCreateRLGL( NVG_ANTIALIAS | NVG_STENCIL_STROKES);
   if( vg == NULL)
   {
      printf( "Could not init nanovg.\n");
      return( -1);
   }

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

      // --- nanovg renders the demo scene ---
      nvgBeginFrame( vg, winWidth, winHeight, pxRatio);
      renderDemo( vg, mx, my, winWidth, winHeight, t, 0, &data);
      nvgEndFrame( vg);

      // --- rlgl draws pac-man chasing the cursor ---
      {
         float dx, dy, dist, speed, targetAngle;

         dx   = (float) mx - pacX;
         dy   = (float) my - pacY;
         dist = sqrtf( dx * dx + dy * dy);

         // Move toward cursor
         speed = 200.0f * (float) dt;
         if( dist > 5.0f)
         {
            targetAngle = atan2f( dy, dx);
            pacAngle    = targetAngle;
            if( speed > dist)
               speed = dist;
            pacX += (dx / dist) * speed;
            pacY += (dy / dist) * speed;
         }

         // Animate mouth (faster when closer to target)
         mouthAnim = (sinf( (float) t * 12.0f) + 1.0f) * 0.25f + 0.05f;

         // Set up rlgl projection to match window pixel coords
         rlMatrixMode( RL_PROJECTION);
         rlLoadIdentity();
         rlOrtho( 0, (double) winWidth, (double) winHeight, 0, -1.0, 1.0);
         rlMatrixMode( RL_MODELVIEW);
         rlLoadIdentity();

         rlDisableDepthTest();
         rlEnableColorBlend();

         drawPacMan( pacX, pacY, 30.0f, pacAngle, mouthAnim, pacTex);
      }

      glfwSwapBuffers( window);
      glfwPollEvents();
   }

   freeDemoData( vg, &data);
   rlUnloadTexture( pacTex);
   nvgDeleteRLGL( vg);
   rlglClose();
   glfwTerminate();
   return( 0);
}
