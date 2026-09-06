// Android demo - port of demo/src/main-linux-gles.c to NativeActivity + EGL.
// Differences from the GLFW version:
//   - window/context/input via android_native_app_glue + EGL (no GLFW)
//   - fonts/images loaded from the APK assets via AAssetManager + *Mem APIs
//   - touch input maps to the mouse position/button the demo expects
#include <android_native_app_glue.h>
#include <android/log.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <string.h>
#include <time.h>

// demo.h first (like main-linux-gles.c): mulle-nanovg.h pulls in the core
// API. The GLES3 backend is then instantiated exactly once below; its
// implementation section lives outside nanovg_gl.h's include guard, so it
// must not be re-included via demo.h afterwards.
#include "demo.h"
#include "android_assets.h"

#define NANOVG_GLES3_IMPLEMENTATION
#include <nanovg_gl.h>

#define LOG_TAG "nanovg"
#define LOGI(...) __android_log_print( ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print( ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

typedef struct Engine
{
   struct android_app  *app;
   EGLDisplay           display;
   EGLSurface           surface;
   EGLContext           context;
   NVGcontext          *vg;
   DemoData             demo;
   int32_t              width;
   int32_t              height;
   int                  animating;
   int                  initialized;
   float                mx, my;      // touch position in window pixels
   int                  blowup;      // 1 while touching
   double               prevt, t;
}
Engine;


static int engine_init_display( Engine *engine)
{
   const EGLint attribs[] =
   {
      EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
      EGL_SURFACE_TYPE,    EGL_WINDOW_BIT,
      EGL_BLUE_SIZE,       8,
      EGL_GREEN_SIZE,      8,
      EGL_RED_SIZE,        8,
      EGL_ALPHA_SIZE,      8,
      EGL_DEPTH_SIZE,      24,
      EGL_STENCIL_SIZE,    8,
      EGL_NONE
   };
   const EGLint contextAttribs[] =
   {
      EGL_CONTEXT_CLIENT_VERSION, 3,
      EGL_NONE
   };
   EGLConfig  config;
   EGLint     numConfigs;
   EGLint     format;

   engine->display = eglGetDisplay( EGL_DEFAULT_DISPLAY);
   if( engine->display == EGL_NO_DISPLAY)
   {
      LOGE( "eglGetDisplay failed");
      return( -1);
   }
   if( !eglInitialize( engine->display, NULL, NULL))
   {
      LOGE( "eglInitialize failed");
      engine->display = EGL_NO_DISPLAY;
      return( -1);
   }
   if( !eglChooseConfig( engine->display, attribs, &config, 1, &numConfigs))
   {
      LOGE( "eglChooseConfig failed");
      return( -1);
   }
   if( !eglGetConfigAttrib( engine->display, config, EGL_NATIVE_VISUAL_ID, &format))
   {
      LOGE( "eglGetConfigAttrib failed");
      return( -1);
   }

   ANativeWindow_setBuffersGeometry( engine->app->window, 0, 0, format);

   engine->surface = eglCreateWindowSurface( engine->display, config, engine->app->window, NULL);
   if( engine->surface == EGL_NO_SURFACE)
   {
      LOGE( "eglCreateWindowSurface failed");
      return( -1);
   }
   engine->context = eglCreateContext( engine->display, config, EGL_NO_CONTEXT, contextAttribs);
   if( engine->context == EGL_NO_CONTEXT)
   {
      LOGE( "eglCreateContext failed");
      return( -1);
   }
   if( !eglMakeCurrent( engine->display, engine->surface, engine->surface, engine->context))
   {
      LOGE( "eglMakeCurrent failed");
      return( -1);
   }

   engine->width  = ANativeWindow_getWidth( engine->app->window);
   engine->height = ANativeWindow_getHeight( engine->app->window);
   LOGI( "EGL init OK, window %dx%d", engine->width, engine->height);

   // NVG_CLEARTYPE: needs freetype (now cross-compiled for Android) and the
   // dFdx-capable GLES3 backend. It makes nanovg keep a second, RGBA/LCD
   // font context that ClearType glyphs are rasterized into.
   engine->vg = nvgCreateGLES3( NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_CLEARTYPE);
   if( engine->vg == NULL)
   {
      LOGE( "Could not init nanovg");
      return( -1);
   }
   LOGI( "NanoVG GLES3 context created successfully (ClearType enabled)");

   if( loadDemoData( engine->vg, &engine->demo) == -1)
   {
      LOGE( "Could not load demo data (assets missing?)");
      return( -1);
   }
   LOGI( "Demo data loaded (12 images, 4 fonts)");

   engine->initialized = 1;
   return( 0);
}


static void engine_term_display( Engine *engine)
{
   if( engine->vg != NULL)
   {
      freeDemoData( engine->vg, &engine->demo);
      nvgDeleteGLES3( engine->vg);
      engine->vg = NULL;
   }
   if( engine->display != EGL_NO_DISPLAY)
   {
      eglMakeCurrent( engine->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
      if( engine->context != EGL_NO_CONTEXT)
         eglDestroyContext( engine->display, engine->context);
      if( engine->surface != EGL_NO_SURFACE)
         eglDestroySurface( engine->display, engine->surface);
      eglTerminate( engine->display);
   }
   engine->display = EGL_NO_DISPLAY;
   engine->context = EGL_NO_CONTEXT;
   engine->surface = EGL_NO_SURFACE;
   engine->initialized = 0;
}


// ClearType comparison strip: "Regular" (alpha AA) over "ClearType" (freetype
// LCD subpixel AA) on a solid panel. For correct LCD blending the text
// background color must match the panel behind the text, and must be set
// *after* nvgTextColor (which resets it). Set nvgClearType before binding the
// font face: nvgFontFace resolves the font in the context selected by the
// current clearType state.
static void draw_cleartype_strip( NVGcontext *vg, float width, float height)
{
   const NVGcolor panel  = nvgRGBAf( 0.11f, 0.11f, 0.13f, 1.0f);
   const float    x      = 8.0f;
   const float    y      = height - 62.0f;
   const float    w      = 304.0f;
   const float    h      = 54.0f;

   nvgSave( vg);

   nvgBeginPath( vg);
   nvgRoundedRect( vg, x, y, w, h, 6.0f);
   nvgFillColor( vg, panel);
   nvgFill( vg);

   nvgFontSize( vg, 17.0f);
   nvgTextAlign( vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);

   nvgClearType( vg, 0);
   nvgFontFace( vg, "sans");
   nvgTextColor( vg, nvgRGBAf( 1.0f, 1.0f, 1.0f, 1.0f));
   nvgText( vg, x + 10.0f, y + 6.0f, "Regular   AaBbCc 123", NULL);

   nvgClearType( vg, 1);
   nvgFontFace( vg, "sans");
   nvgTextColor( vg, nvgRGBAf( 1.0f, 1.0f, 1.0f, 1.0f));
   nvgTextBackgroundColor( vg, panel);
   nvgText( vg, x + 10.0f, y + 28.0f, "ClearType AaBbCc 123", NULL);

   nvgClearType( vg, 0);
   nvgRestore( vg);
}


static void engine_draw_frame( Engine *engine)
{
   int    width, height;

   if( engine->display == EGL_NO_DISPLAY)
      return;

   width  = ANativeWindow_getWidth( engine->app->window);
   height = ANativeWindow_getHeight( engine->app->window);
   if( width != engine->width || height != engine->height)
   {
      engine->width  = width;
      engine->height = height;
   }

   glViewport( 0, 0, engine->width, engine->height);
   glClearColor( 0.3f, 0.3f, 0.32f, 1.0f);
   glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

   nvgBeginFrame( engine->vg, engine->width, engine->height, 1.0f);
   renderDemo( engine->vg, engine->mx, engine->my,
               engine->width, engine->height, engine->t,
               engine->blowup, &engine->demo);
   draw_cleartype_strip( engine->vg, (float) engine->width, (float) engine->height);
   nvgEndFrame( engine->vg);

   eglSwapBuffers( engine->display, engine->surface);
}


static int32_t engine_handle_input( struct android_app *app, AInputEvent *event)
{
   Engine  *engine = (Engine *) app->userData;
   int      action;

   if( AInputEvent_getType( event) != AINPUT_EVENT_TYPE_MOTION)
      return( 0);

   engine->mx = AMotionEvent_getX( event, 0);
   engine->my = AMotionEvent_getY( event, 0);

   action = AMotionEvent_getAction( event) & AMOTION_EVENT_ACTION_MASK;
   switch( action)
   {
   case AMOTION_EVENT_ACTION_DOWN:
      engine->blowup = 1;
      break;
   case AMOTION_EVENT_ACTION_UP:
   case AMOTION_EVENT_ACTION_CANCEL:
      engine->blowup = 0;
      break;
   }
   return( 1);
}


static void engine_handle_cmd( struct android_app *app, int32_t cmd)
{
   Engine *engine = (Engine *) app->userData;

   switch( cmd)
   {
   case APP_CMD_INIT_WINDOW:
      LOGI( "APP_CMD_INIT_WINDOW");
      if( app->window != NULL)
      {
         if( !engine->initialized)
            engine_init_display( engine);
         engine->animating = 1;
      }
      break;

   case APP_CMD_TERM_WINDOW:
      LOGI( "APP_CMD_TERM_WINDOW");
      engine->animating = 0;
      engine_term_display( engine);
      break;

   case APP_CMD_GAINED_FOCUS:
      LOGI( "APP_CMD_GAINED_FOCUS");
      engine->animating = 1;
      break;

   case APP_CMD_LOST_FOCUS:
      LOGI( "APP_CMD_LOST_FOCUS");
      engine->animating = 0;
      break;

   case APP_CMD_DESTROY:
      LOGI( "APP_CMD_DESTROY");
      engine->animating = 0;
      break;
   }
}


static double now_seconds( void)
{
   struct timespec  ts;

   clock_gettime( CLOCK_MONOTONIC, &ts);
   return( (double) ts.tv_sec + (double) ts.tv_nsec / 1000000000.0);
}


void android_main( struct android_app *app)
{
   Engine  engine;
   double  dt;

   // Make sure glue isn't stripped: this call forces the linker to pull in
   // android_native_app_glue.c, which defines ANativeActivity_onCreate.
   // (Deprecated but load-bearing for static glue linking.)
   app_dummy();

   memset( &engine, 0, sizeof( engine));
   engine.app      = app;
   engine.display  = EGL_NO_DISPLAY;
   engine.surface  = EGL_NO_SURFACE;
   engine.context  = EGL_NO_CONTEXT;
   engine.mx       = 160.0f;
   engine.my       = 320.0f;

   app->userData     = &engine;
   app->onAppCmd     = engine_handle_cmd;
   app->onInputEvent = engine_handle_input;

   androidAssetsInit( app->activity->assetManager);

   LOGI( "android_main entered");

   engine.t = now_seconds();
   while( 1)
   {
      int                          events;
      struct android_poll_source  *source;

      while( ALooper_pollOnce( engine.animating ? 0 : -1, NULL, &events,
                               (void **) &source) >= 0)
      {
         if( source != NULL)
            source->process( app, source);
         if( app->destroyRequested)
         {
            LOGI( "destroy requested, exiting");
            if( engine.initialized)
               engine_term_display( &engine);
            return;
         }
      }

      if( engine.animating)
      {
         dt          = now_seconds() - engine.t;
         engine.t   += dt;
         (void) dt;
         engine_draw_frame( &engine);
      }
   }
}