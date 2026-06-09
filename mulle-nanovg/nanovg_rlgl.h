//
// nanovg_rlgl.h — NanoVG render backend using rlgl
//
// Copyright (c) 2024 Nat! for Mulle kybernetiK
// Portions based on nanovg_gl.h (c) 2009-2013 Mikko Mononen
//
// This software is provided 'as-is', without any express or implied warranty.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented.
// 2. Altered source versions must be plainly marked as such.
// 3. This notice may not be removed or altered from any source distribution.
//
#ifndef NANOVG_RLGL_H
#define NANOVG_RLGL_H

#include "nanovg.h"

#ifndef MULLE__NANOVG_RLGL_GLOBAL
# ifdef MULLE__NANOVG_RLGL_BUILD
#  if defined(_WIN32)
#   define MULLE__NANOVG_RLGL_GLOBAL    extern __declspec(dllexport)
#  else
#   define MULLE__NANOVG_RLGL_GLOBAL    extern
#  endif
# else
#  if defined(_WIN32) && !defined(MULLE__NANOVG_RLGL_INCLUDE_STATIC)
#   define MULLE__NANOVG_RLGL_GLOBAL    extern __declspec(dllimport)
#  else
#   define MULLE__NANOVG_RLGL_GLOBAL    extern
#  endif
# endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

MULLE__NANOVG_RLGL_GLOBAL
NVGcontext* nvgCreateRLGL(int flags);

MULLE__NANOVG_RLGL_GLOBAL
void nvgDeleteRLGL(NVGcontext* ctx);

MULLE__NANOVG_RLGL_GLOBAL
int nvglCreateImageFromHandleRLGL(NVGcontext* ctx, unsigned int textureId, int w, int h, int flags);

MULLE__NANOVG_RLGL_GLOBAL
unsigned int nvglImageHandleRLGL(NVGcontext* ctx, int image);

#ifdef __cplusplus
}
#endif

#endif // NANOVG_RLGL_H


// ------------------------------------------------------------------
// Implementation
// ------------------------------------------------------------------
#ifdef NANOVG_RLGL_IMPLEMENTATION

#include "mulle-rlgl.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>

// Using the same create flags from nanovg_gl.h (already in nanovg_gl.h but
// we redefine the enum values we need here for standalone compilation)
#ifndef NVG_ANTIALIAS
enum NVGcreateFlags {
   NVG_ANTIALIAS       = 1<<0,
   NVG_STENCIL_STROKES = 1<<1,
   NVG_DEBUG           = 1<<2,
   NVG_PATH_ONLY       = 1<<3,
   NVG_FONT_ONLY       = 1<<4,
   NVG_CLEARTYPE       = 1<<5,
   NVG_MSAA            = 1<<6,
   NVG_NO_DRAWING      = 1<<7
};
#endif

#ifndef NVG_MIN_STATES
# define NVG_MIN_STATES 1
#endif
#ifndef NVG_MAX_STATES
# define NVG_MAX_STATES 32
#endif

#ifndef NVG_IMAGE_NODELETE
# define NVG_IMAGE_NODELETE  (1<<16)
#endif

// Shader types
enum RLGLNVGshaderType {
   RLNSVG_SHADER_FILLGRAD  = 0,
   RLNSVG_SHADER_FILLIMG   = 1,
   RLNSVG_SHADER_SIMPLE    = 2,
   RLNSVG_SHADER_IMG       = 3,
   RLNSVG_SHADER_CLEARTYPE = 4
};

// Uniform array size (non-UBO path)
#define RLNVG_UNIFORMARRAY_SIZE 11

enum RLGLNVGuniformLoc {
   RLNVG_LOC_VIEWSIZE = 0,
   RLNVG_LOC_TEX,
   RLNVG_LOC_FRAG,
   RLNVG_MAX_LOCS
};

struct RLGLNVGtexture {
   int            id;
   unsigned int   tex;
   int            width, height;
   int            type;
   int            flags;
};
typedef struct RLGLNVGtexture RLGLNVGtexture;

struct RLGLNVGblend {
   int   srcRGB;
   int   dstRGB;
   int   srcAlpha;
   int   dstAlpha;
};
typedef struct RLGLNVGblend RLGLNVGblend;

enum RLGLNVGcallType {
   RLGLNVG_NONE = 0,
   RLGLNVG_FILL,
   RLGLNVG_CONVEXFILL,
   RLGLNVG_STROKE,
   RLGLNVG_TRIANGLES,
};

struct RLGLNVGcall {
   int            type;
   int            image;
   int            pathOffset;
   int            pathCount;
   int            triangleOffset;
   int            triangleCount;
   int            uniformOffset;
   RLGLNVGblend   blendFunc;
};
typedef struct RLGLNVGcall RLGLNVGcall;

struct RLGLNVGpath {
   int   fillOffset;
   int   fillCount;
   int   strokeOffset;
   int   strokeCount;
};
typedef struct RLGLNVGpath RLGLNVGpath;

struct RLGLNVGfragUniforms {
   union {
      struct {
         float          scissorMat[12];
         float          paintMat[12];
         struct NVGcolor innerCol;
         struct NVGcolor outerCol;
         float          scissorExt[2];
         float          scissorScale[2];
         float          extent[2];
         float          radius;
         float          feather;
         float          strokeMult;
         float          strokeThr;
         float          texType;
         float          type;
      };
      float uniformArray[RLNVG_UNIFORMARRAY_SIZE][4];
   };
};
typedef struct RLGLNVGfragUniforms RLGLNVGfragUniforms;

struct RLGLNVGcontext {
   unsigned int      shaderProg;
   int               loc[RLNVG_MAX_LOCS];
   int               locVertex;
   int               locTcoord;

   RLGLNVGtexture   *textures;
   int               ntextures;
   int               ctextures;
   int               utextures;
   int               textureId;

   float             view[2];
   unsigned int      vertBuf;
   unsigned int      vertArr;
   int               fragSize;
   int               flags;
   int               dummyTex;

   // Per frame buffers
   RLGLNVGcall      *calls;
   int               ccalls;
   int               ncalls;
   RLGLNVGpath      *paths;
   int               cpaths;
   int               npaths;
   struct NVGvertex *verts;
   int               cverts;
   int               nverts;
   unsigned char    *uniforms;
   int               cuniforms;
   int               nuniforms;

   // State filter
   int               boundTexture;
   unsigned int      stencilMask;
   int               stencilFunc;
   int               stencilFuncRef;
   unsigned int      stencilFuncMask;
   RLGLNVGblend      blendFunc;
};
typedef struct RLGLNVGcontext RLGLNVGcontext;


// ------------------------------------------------------------------
// Utilities
// ------------------------------------------------------------------

static int rlglnvg__maxi(int a, int b) { return a > b ? a : b; }

static RLGLNVGtexture* rlglnvg__allocTexture(RLGLNVGcontext* gl)
{
   RLGLNVGtexture  *tex = NULL;
   int             i;

   if( gl->utextures < gl->ntextures)
   {
      for( i = 0; i < gl->ntextures; i++)
      {
         if( gl->textures[i].id == 0)
         {
            tex = &gl->textures[i];
            break;
         }
      }
   }
   else
   {
      if( gl->ntextures >= gl->ctextures)
      {
         RLGLNVGtexture  *textures;
         int             ctextures;

         ctextures = rlglnvg__maxi( gl->ntextures + 1, 4) + gl->ctextures / 2;
         textures  = (RLGLNVGtexture *) realloc( gl->textures, sizeof( RLGLNVGtexture) * ctextures);
         if( textures == NULL)
            return( NULL);
         gl->textures  = textures;
         gl->ctextures = ctextures;
      }
      i   = gl->ntextures++;
      tex = &gl->textures[i];
   }

   memset( tex, 0, sizeof( *tex));
   tex->id = i + 1;
   gl->utextures++;
   return( tex);
}

static RLGLNVGtexture* rlglnvg__findTexture(RLGLNVGcontext* gl, int id)
{
   int   i;

   if( ! id || id > gl->ntextures)
      return( NULL);
   i = id - 1;
   if( gl->textures[i].id == id)
      return( &gl->textures[i]);
   return( NULL);
}

static int rlglnvg__deleteTexture(RLGLNVGcontext* gl, int id)
{
   RLGLNVGtexture  *tex;

   tex = rlglnvg__findTexture( gl, id);
   if( ! tex)
      return( 0);
   if( (tex->flags & NVG_IMAGE_NODELETE) == 0)
      rlUnloadTexture( tex->tex);
   memset( tex, 0, sizeof( *tex));
   --gl->utextures;
   return( 1);
}

static void rlglnvg__bindTexture(RLGLNVGcontext* gl, unsigned int tex)
{
   if( gl->boundTexture != (int) tex)
   {
      gl->boundTexture = tex;
      rlActiveTextureSlot( 0);
      if( tex)
         rlEnableTexture( tex);
      else
         rlDisableTexture();
   }
}

static void rlglnvg__stencilMask(RLGLNVGcontext* gl, unsigned int mask)
{
   if( gl->stencilMask != mask)
   {
      gl->stencilMask = mask;
      rlStencilMask( mask);
   }
}

static void rlglnvg__stencilFunc(RLGLNVGcontext* gl, int func, int ref, unsigned int mask)
{
   if( gl->stencilFunc != func || gl->stencilFuncRef != ref || gl->stencilFuncMask != mask)
   {
      gl->stencilFunc     = func;
      gl->stencilFuncRef  = ref;
      gl->stencilFuncMask = mask;
      rlStencilFunc( func, ref, mask);
   }
}

static void rlglnvg__blendFuncSeparate(RLGLNVGcontext* gl, const RLGLNVGblend* blend)
{
   if( gl->blendFunc.srcRGB   != blend->srcRGB   ||
       gl->blendFunc.dstRGB   != blend->dstRGB   ||
       gl->blendFunc.srcAlpha != blend->srcAlpha  ||
       gl->blendFunc.dstAlpha != blend->dstAlpha)
   {
      gl->blendFunc = *blend;
      rlSetBlendMode( RL_BLEND_CUSTOM_SEPARATE);
      rlSetBlendFactorsSeparate( blend->srcRGB, blend->dstRGB,
                                 blend->srcAlpha, blend->dstAlpha,
                                 RL_FUNC_ADD, RL_FUNC_ADD);
   }
}

// ------------------------------------------------------------------
// Shader source (same as nanovg_gl.h, non-UBO path)
// ------------------------------------------------------------------

static const char *rlglnvg__shaderHeader(void)
{
   switch( rlGetVersion())
   {
   case RL_OPENGL_33 :
   case RL_OPENGL_43 :
      return "#version 150 core\n"
             "#define NANOVG_GL3 1\n"
             "#define UNIFORMARRAY_SIZE 11\n"
             "\n";

   case RL_OPENGL_ES_30 :
      return "#version 300 es\n"
             "#define NANOVG_GL3 1\n"
             "#define UNIFORMARRAY_SIZE 11\n"
             "\n";

   case RL_OPENGL_ES_20 :
   default :
      return "#version 100\n"
             "#extension GL_OES_standard_derivatives : enable\n"
             "#define NANOVG_GL2 1\n"
             "#define UNIFORMARRAY_SIZE 11\n"
             "\n";
   }
}

static const char *rlglnvg__vertShader =
   "#ifdef NANOVG_GL3\n"
   "   uniform vec2 viewSize;\n"
   "   in vec2 vertex;\n"
   "   in vec2 tcoord;\n"
   "   out vec2 ftcoord;\n"
   "   out vec2 fpos;\n"
   "#else\n"
   "   uniform vec2 viewSize;\n"
   "   attribute vec2 vertex;\n"
   "   attribute vec2 tcoord;\n"
   "   varying vec2 ftcoord;\n"
   "   varying vec2 fpos;\n"
   "#endif\n"
   "void main(void) {\n"
   "   ftcoord = tcoord;\n"
   "   fpos = vertex;\n"
   "   gl_Position = vec4(2.0*vertex.x/viewSize.x - 1.0, 1.0 - 2.0*vertex.y/viewSize.y, 0, 1);\n"
   "}\n";

static const char *rlglnvg__fragShader =
   "#ifdef GL_ES\n"
   "#if defined(GL_FRAGMENT_PRECISION_HIGH) || defined(NANOVG_GL3)\n"
   " precision highp float;\n"
   "#else\n"
   " precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "#ifdef NANOVG_GL3\n"
   "   uniform vec4 frag[UNIFORMARRAY_SIZE];\n"
   "   uniform sampler2D tex;\n"
   "   in vec2 ftcoord;\n"
   "   in vec2 fpos;\n"
   "   out vec4 outColor;\n"
   "#else\n"
   "   uniform vec4 frag[UNIFORMARRAY_SIZE];\n"
   "   uniform sampler2D tex;\n"
   "   varying vec2 ftcoord;\n"
   "   varying vec2 fpos;\n"
   "#endif\n"
   "#define scissorMat mat3(frag[0].xyz, frag[1].xyz, frag[2].xyz)\n"
   "#define paintMat mat3(frag[3].xyz, frag[4].xyz, frag[5].xyz)\n"
   "#define innerCol frag[6]\n"
   "#define outerCol frag[7]\n"
   "#define scissorExt frag[8].xy\n"
   "#define scissorScale frag[8].zw\n"
   "#define extent frag[9].xy\n"
   "#define radius frag[9].z\n"
   "#define feather frag[9].w\n"
   "#define strokeMult frag[10].x\n"
   "#define strokeThr frag[10].y\n"
   "#define texType int(frag[10].z)\n"
   "#define type int(frag[10].w)\n"
   "\n"
   "float sdroundrect(vec2 pt, vec2 ext, float rad) {\n"
   "   vec2 ext2 = ext - vec2(rad,rad);\n"
   "   vec2 d = abs(pt) - ext2;\n"
   "   return min(max(d.x,d.y),0.0) + length(max(d,0.0)) - rad;\n"
   "}\n"
   "\n"
   "float scissorMask(vec2 p) {\n"
   "   vec2 sc = (abs((scissorMat * vec3(p,1.0)).xy) - scissorExt);\n"
   "   sc = vec2(0.5,0.5) - sc * scissorScale;\n"
   "   return clamp(sc.x,0.0,1.0) * clamp(sc.y,0.0,1.0);\n"
   "}\n"
   "#ifdef EDGE_AA\n"
   "float strokeMask() {\n"
   "   return min(1.0, (1.0-abs(ftcoord.x*2.0-1.0))*strokeMult) * min(1.0, ftcoord.y);\n"
   "}\n"
   "#endif\n"
   "\n"
   "void main(void) {\n"
   "   vec4 result;\n"
   "   float scissor = scissorMask(fpos);\n"
   "#ifdef EDGE_AA\n"
   "   float strokeAlpha = strokeMask();\n"
   "   if (strokeAlpha < strokeThr) discard;\n"
   "#else\n"
   "   float strokeAlpha = 1.0;\n"
   "#endif\n"
   "   if (type == 0) {\n"
   "      vec2 pt = (paintMat * vec3(fpos,1.0)).xy;\n"
   "      float d = clamp((sdroundrect(pt, extent, radius) + feather*0.5) / feather, 0.0, 1.0);\n"
   "      vec4 color = mix(innerCol,outerCol,d);\n"
   "      color *= strokeAlpha * scissor;\n"
   "      result = color;\n"
   "   } else if (type == 1) {\n"
   "      vec2 pt = (paintMat * vec3(fpos,1.0)).xy / extent;\n"
   "#ifdef NANOVG_GL3\n"
   "      vec4 color = texture(tex, pt);\n"
   "#else\n"
   "      vec4 color = texture2D(tex, pt);\n"
   "#endif\n"
   "      if (texType == 1) color = vec4(color.xyz*color.w,color.w);"
   "      if (texType == 2) color = vec4(color.x);"
   "      color *= innerCol;\n"
   "      color *= strokeAlpha * scissor;\n"
   "      result = color;\n"
   "   } else if (type == 2) {\n"
   "      result = vec4(1,1,1,1);\n"
   "   } else if (type == 3) {\n"
   "#ifdef NANOVG_GL3\n"
   "      vec4 color = texture(tex, ftcoord);\n"
   "#else\n"
   "      vec4 color = texture2D(tex, ftcoord);\n"
   "#endif\n"
   "      if (texType == 1) color = vec4(color.xyz*color.w,color.w);"
   "      if (texType == 2) color = vec4(color.x);"
   "      color *= scissor;\n"
   "      result = color * innerCol;\n"
   "   } else if (type == 4) {\n"
   "#ifdef NANOVG_GL3\n"
   "      vec4 color = texture(tex, ftcoord);\n"
   "#else\n"
   "      vec4 color = texture2D(tex, ftcoord);\n"
   "#endif\n"
   "      float deriv = dFdx( ftcoord.x );\n"
   "      if( deriv < 0.0 )\n"
   "         color.xz = color.zx;\n"
   "      else if ( deriv == 0.0 )\n"
   "         color = vec4( ( color.x + color.y + color.z ) * ( 1.0 / 3.0 ) );\n"
   "      if( color.w * scissor * innerCol.w < ( 1.0 / 256.0 ) )\n"
   "         discard;\n"
   "      result.xyz = color.xyz * innerCol.xyz + ( vec3( innerCol.w ) - color.xyz ) * outerCol.xyz;\n"
   "      result.w = innerCol.w;\n"
   "      result *= scissor;\n"
   "   }\n"
   "#ifdef NANOVG_GL3\n"
   "   outColor = result;\n"
   "#else\n"
   "   gl_FragColor = result;\n"
   "#endif\n"
   "}\n";


// ------------------------------------------------------------------
// Render callbacks
// ------------------------------------------------------------------

static int rlglnvg__renderCreateTexture(void* uptr, int type, int w, int h, int imageFlags, const unsigned char* data);

static int rlglnvg__renderCreate(void* uptr)
{
   RLGLNVGcontext  *gl = (RLGLNVGcontext *) uptr;
   const char      *header;
   const char      *edgeAA;
   char            fullVert[4096];
   char            fullFrag[8192];

   header = rlglnvg__shaderHeader();
   edgeAA = (gl->flags & NVG_ANTIALIAS) ? "#define EDGE_AA 1\n" : "";

   snprintf( fullVert, sizeof( fullVert), "%s%s%s", header, edgeAA, rlglnvg__vertShader);
   snprintf( fullFrag, sizeof( fullFrag), "%s%s%s", header, edgeAA, rlglnvg__fragShader);

   gl->shaderProg = rlLoadShaderProgram( fullVert, fullFrag);
   if( ! gl->shaderProg)
   {
      fprintf( stderr, "nanovg_rlgl: shader creation failed\n");
      return( 0);
   }

   gl->loc[RLNVG_LOC_VIEWSIZE] = rlGetLocationUniform( gl->shaderProg, "viewSize");
   gl->loc[RLNVG_LOC_TEX]      = rlGetLocationUniform( gl->shaderProg, "tex");
   gl->loc[RLNVG_LOC_FRAG]     = rlGetLocationUniform( gl->shaderProg, "frag");
   gl->locVertex                = rlGetLocationAttrib( gl->shaderProg, "vertex");
   gl->locTcoord                = rlGetLocationAttrib( gl->shaderProg, "tcoord");

   // Create dynamic vertex array
   gl->vertArr = rlLoadVertexArray();
   rlEnableVertexArray( gl->vertArr);
   gl->vertBuf = rlLoadVertexBuffer( NULL, 0, true);
   rlDisableVertexArray();

   gl->fragSize = sizeof( RLGLNVGfragUniforms);

   // Create dummy texture for when no texture is bound
   gl->dummyTex = rlglnvg__renderCreateTexture( gl, NVG_TEXTURE_ALPHA, 1, 1, 0, NULL);

   rlFinish();
   return( 1);
}

static int rlglnvg__renderCreateTexture(void* uptr, int type, int w, int h, int imageFlags, const unsigned char* data)
{
   RLGLNVGcontext  *gl = (RLGLNVGcontext *) uptr;
   RLGLNVGtexture  *tex;
   int             format;
   int             mipmapCount;

   tex = rlglnvg__allocTexture( gl);
   if( tex == NULL)
      return( 0);

   tex->width  = w;
   tex->height = h;
   tex->type   = type;
   tex->flags  = imageFlags;

   if( type == NVG_TEXTURE_RGBA)
      format = RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
   else
      format = RL_PIXELFORMAT_UNCOMPRESSED_GRAYSCALE;

   mipmapCount = (imageFlags & NVG_IMAGE_GENERATE_MIPMAPS) ? -1 : 1;
   tex->tex = rlLoadTexture( data, w, h, format, mipmapCount);
   if( ! tex->tex)
      return( 0);

   // Set texture parameters
   if( imageFlags & NVG_IMAGE_NEAREST)
   {
      rlTextureParameters( tex->tex, RL_TEXTURE_MIN_FILTER, RL_TEXTURE_FILTER_NEAREST);
      rlTextureParameters( tex->tex, RL_TEXTURE_MAG_FILTER, RL_TEXTURE_FILTER_NEAREST);
   }
   else
   {
      if( imageFlags & NVG_IMAGE_GENERATE_MIPMAPS)
         rlTextureParameters( tex->tex, RL_TEXTURE_MIN_FILTER, RL_TEXTURE_FILTER_MIP_LINEAR);
      else
         rlTextureParameters( tex->tex, RL_TEXTURE_MIN_FILTER, RL_TEXTURE_FILTER_LINEAR);
      rlTextureParameters( tex->tex, RL_TEXTURE_MAG_FILTER, RL_TEXTURE_FILTER_LINEAR);
   }

   if( imageFlags & NVG_IMAGE_REPEATX)
      rlTextureParameters( tex->tex, RL_TEXTURE_WRAP_S, RL_TEXTURE_WRAP_REPEAT);
   else
      rlTextureParameters( tex->tex, RL_TEXTURE_WRAP_S, RL_TEXTURE_WRAP_CLAMP);

   if( imageFlags & NVG_IMAGE_REPEATY)
      rlTextureParameters( tex->tex, RL_TEXTURE_WRAP_T, RL_TEXTURE_WRAP_REPEAT);
   else
      rlTextureParameters( tex->tex, RL_TEXTURE_WRAP_T, RL_TEXTURE_WRAP_CLAMP);

   if( imageFlags & NVG_IMAGE_GENERATE_MIPMAPS)
      rlGenTextureMipmaps( tex->tex, w, h, format, &mipmapCount);

   return( tex->id);
}

static int rlglnvg__renderDeleteTexture(void* uptr, int image)
{
   RLGLNVGcontext  *gl = (RLGLNVGcontext *) uptr;

   return( rlglnvg__deleteTexture( gl, image));
}

static int rlglnvg__renderUpdateTexture(void* uptr, int image, int x, int y, int w, int h, const unsigned char* data)
{
   RLGLNVGcontext  *gl = (RLGLNVGcontext *) uptr;
   RLGLNVGtexture  *tex;
   int             format;

   tex = rlglnvg__findTexture( gl, image);
   if( tex == NULL)
      return( 0);

   // rlUpdateTexture needs the full row data on GLES2-style platforms,
   // so we adjust similarly to nanovg_gl.h
   if( tex->type == NVG_TEXTURE_RGBA)
   {
      format = RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
      data  += y * tex->width * 4;
   }
   else
   {
      format = RL_PIXELFORMAT_UNCOMPRESSED_GRAYSCALE;
      data  += y * tex->width;
   }

   rlUpdateTexture( tex->tex, 0, y, tex->width, h, format, data);
   return( 1);
}

static int rlglnvg__renderGetTextureSize(void* uptr, int image, int* w, int* h)
{
   RLGLNVGcontext  *gl = (RLGLNVGcontext *) uptr;
   RLGLNVGtexture  *tex;

   tex = rlglnvg__findTexture( gl, image);
   if( tex == NULL)
      return( 0);
   *w = tex->width;
   *h = tex->height;
   return( 1);
}

// @mulle-nanovg@ extensions
static int rlglnvg__renderDefineTexture(void* uptr, int w, int h, int flags, int type, void *texture)
{
   RLGLNVGcontext  *gl = (RLGLNVGcontext *) uptr;
   RLGLNVGtexture  *tex;

   tex = rlglnvg__allocTexture( gl);
   if( tex == NULL)
      return( 0);
   tex->width  = w;
   tex->height = h;
   tex->flags  = flags;
   tex->type   = type;
   tex->tex    = (unsigned int) (intptr_t) texture;
   return( tex->id);
}

static int rlglnvg__renderValidatePaintTexture(void* uptr, int image)
{
   (void) uptr;
   (void) image;
   return( 1);
}

static int rlglnvg__renderGetTextureInfo(void* uptr, int image, int *p_flags, int *p_type, void **p_tex)
{
   RLGLNVGcontext  *gl = (RLGLNVGcontext *) uptr;
   RLGLNVGtexture  *tex;

   tex = rlglnvg__findTexture( gl, image);
   if( ! tex)
      return( 0);
   if( p_flags)
      *p_flags = tex->flags;
   if( p_type)
      *p_type = tex->type;
   if( p_tex)
      *p_tex = (void *) (intptr_t) tex->tex;
   return( 1);
}

static int rlglnvg__renderForgetTexture(void* uptr, int image)
{
   RLGLNVGcontext  *gl = (RLGLNVGcontext *) uptr;
   RLGLNVGtexture  *tex;

   tex = rlglnvg__findTexture( gl, image);
   if( ! tex)
      return( 0);
   tex->id     = 0;
   tex->flags |= NVG_IMAGE_NODELETE;
   return( 1);
}

static void rlglnvg__renderViewport(void* uptr, float width, float height, float devicePixelRatio)
{
   RLGLNVGcontext  *gl = (RLGLNVGcontext *) uptr;

   (void) devicePixelRatio;
   gl->view[0] = width;
   gl->view[1] = height;
}

// ------------------------------------------------------------------
// Draw helpers
// ------------------------------------------------------------------

static void rlglnvg__setUniforms(RLGLNVGcontext* gl, int uniformOffset, int image)
{
   RLGLNVGfragUniforms  *frag;
   RLGLNVGtexture       *tex = NULL;

   frag = (RLGLNVGfragUniforms *) &gl->uniforms[uniformOffset];
   rlSetUniform( gl->loc[RLNVG_LOC_FRAG], frag->uniformArray, RL_SHADER_UNIFORM_VEC4, RLNVG_UNIFORMARRAY_SIZE);

   if( image != 0)
      tex = rlglnvg__findTexture( gl, image);
   if( tex == NULL)
      tex = rlglnvg__findTexture( gl, gl->dummyTex);

   if( tex != NULL)
      rlglnvg__bindTexture( gl, tex->tex);
   else
      rlglnvg__bindTexture( gl, 0);
}

static void rlglnvg__fill(RLGLNVGcontext* gl, RLGLNVGcall* call)
{
   RLGLNVGpath  *paths;
   int          i, npaths;

   paths  = &gl->paths[call->pathOffset];
   npaths = call->pathCount;

   // Draw shapes into stencil
   rlEnableStencilTest();
   rlglnvg__stencilMask( gl, 0xff);
   rlglnvg__stencilFunc( gl, GL_ALWAYS, 0, 0xff);
   rlColorMask( false, false, false, false);

   rlglnvg__setUniforms( gl, call->uniformOffset, 0);

   rlStencilOpSeparate( GL_FRONT, GL_KEEP, GL_KEEP, GL_INCR_WRAP);
   rlStencilOpSeparate( GL_BACK, GL_KEEP, GL_KEEP, GL_DECR_WRAP);
   rlDisableBackfaceCulling();
   for( i = 0; i < npaths; i++)
      rlDrawVertexArrayFan( paths[i].fillOffset, paths[i].fillCount);
   rlEnableBackfaceCulling();

   // Draw anti-aliased pixels
   rlColorMask( true, true, true, true);
   rlglnvg__setUniforms( gl, call->uniformOffset + gl->fragSize, call->image);

   if( gl->flags & NVG_ANTIALIAS)
   {
      rlglnvg__stencilFunc( gl, GL_EQUAL, 0x00, 0xff);
      rlStencilOp( GL_KEEP, GL_KEEP, GL_KEEP);
      for( i = 0; i < npaths; i++)
         rlDrawVertexArrayStrip( paths[i].strokeOffset, paths[i].strokeCount);
   }

   // Draw fill
   rlglnvg__stencilFunc( gl, GL_NOTEQUAL, 0x0, 0xff);
   rlStencilOp( GL_ZERO, GL_ZERO, GL_ZERO);
   rlDrawVertexArrayStrip( call->triangleOffset, call->triangleCount);

   rlDisableStencilTest();
}

static void rlglnvg__convexFill(RLGLNVGcontext* gl, RLGLNVGcall* call)
{
   RLGLNVGpath  *paths;
   int          i, npaths;

   paths  = &gl->paths[call->pathOffset];
   npaths = call->pathCount;

   rlglnvg__setUniforms( gl, call->uniformOffset, call->image);

   for( i = 0; i < npaths; i++)
   {
      rlDrawVertexArrayFan( paths[i].fillOffset, paths[i].fillCount);
      if( paths[i].strokeCount > 0)
         rlDrawVertexArrayStrip( paths[i].strokeOffset, paths[i].strokeCount);
   }
}

static void rlglnvg__stroke(RLGLNVGcontext* gl, RLGLNVGcall* call)
{
   RLGLNVGpath  *paths;
   int          i, npaths;

   paths  = &gl->paths[call->pathOffset];
   npaths = call->pathCount;

   if( gl->flags & NVG_STENCIL_STROKES)
   {
      rlEnableStencilTest();
      rlglnvg__stencilMask( gl, 0xff);

      // Fill the stroke base without overlap
      rlglnvg__stencilFunc( gl, GL_EQUAL, 0x0, 0xff);
      rlStencilOp( GL_KEEP, GL_KEEP, GL_INCR);
      rlglnvg__setUniforms( gl, call->uniformOffset + gl->fragSize, call->image);
      for( i = 0; i < npaths; i++)
         rlDrawVertexArrayStrip( paths[i].strokeOffset, paths[i].strokeCount);

      // Draw anti-aliased pixels
      rlglnvg__setUniforms( gl, call->uniformOffset, call->image);
      rlglnvg__stencilFunc( gl, GL_EQUAL, 0x00, 0xff);
      rlStencilOp( GL_KEEP, GL_KEEP, GL_KEEP);
      for( i = 0; i < npaths; i++)
         rlDrawVertexArrayStrip( paths[i].strokeOffset, paths[i].strokeCount);

      // Clear stencil buffer
      rlColorMask( false, false, false, false);
      rlglnvg__stencilFunc( gl, GL_ALWAYS, 0x0, 0xff);
      rlStencilOp( GL_ZERO, GL_ZERO, GL_ZERO);
      for( i = 0; i < npaths; i++)
         rlDrawVertexArrayStrip( paths[i].strokeOffset, paths[i].strokeCount);
      rlColorMask( true, true, true, true);

      rlDisableStencilTest();
   }
   else
   {
      rlglnvg__setUniforms( gl, call->uniformOffset, call->image);
      for( i = 0; i < npaths; i++)
         rlDrawVertexArrayStrip( paths[i].strokeOffset, paths[i].strokeCount);
   }
}

static void rlglnvg__triangles(RLGLNVGcontext* gl, RLGLNVGcall* call)
{
   rlglnvg__setUniforms( gl, call->uniformOffset, call->image);
   rlDrawVertexArray( call->triangleOffset, call->triangleCount);
}

// ------------------------------------------------------------------
// Flush
// ------------------------------------------------------------------

static void rlglnvg__renderFlush(void* uptr)
{
   RLGLNVGcontext  *gl = (RLGLNVGcontext *) uptr;
   int             i;

   if( gl->ncalls > 0)
   {
      // Flush rlgl internal batch state
      rlDrawRenderBatchActive();

      // Setup GL state
      rlEnableShader( gl->shaderProg);
      rlEnableBackfaceCulling();
      rlSetCullFace( RL_CULL_FACE_BACK);
      rlEnableColorBlend();
      rlDisableDepthTest();
      rlDisableScissorTest();
      rlColorMask( true, true, true, true);
      rlStencilMask( 0xffffffff);
      rlStencilOp( GL_KEEP, GL_KEEP, GL_KEEP);
      rlStencilFunc( GL_ALWAYS, 0, 0xffffffff);
      rlActiveTextureSlot( 0);
      rlDisableTexture();

      // Reset state filter
      gl->boundTexture    = 0;
      gl->stencilMask     = 0xffffffff;
      gl->stencilFunc     = GL_ALWAYS;
      gl->stencilFuncRef  = 0;
      gl->stencilFuncMask = 0xffffffff;
      gl->blendFunc.srcRGB   = -1;
      gl->blendFunc.srcAlpha = -1;
      gl->blendFunc.dstRGB   = -1;
      gl->blendFunc.dstAlpha = -1;

      // Upload vertex data
      rlEnableVertexArray( gl->vertArr);
      rlEnableVertexBuffer( gl->vertBuf);
      rlUploadVertexBuffer( gl->vertBuf, gl->verts, gl->nverts * (int) sizeof( NVGvertex));
      rlEnableVertexAttribute( (unsigned int) gl->locVertex);
      rlEnableVertexAttribute( (unsigned int) gl->locTcoord);
      rlSetVertexAttribute( (unsigned int) gl->locVertex, 2, RL_FLOAT, false, (int) sizeof( NVGvertex), 0);
      rlSetVertexAttribute( (unsigned int) gl->locTcoord, 2, RL_FLOAT, false, (int) sizeof( NVGvertex), 2 * (int) sizeof( float));

      // Set per-frame uniforms
      rlSetUniform( gl->loc[RLNVG_LOC_TEX], (int[]){0}, RL_SHADER_UNIFORM_INT, 1);
      rlSetUniform( gl->loc[RLNVG_LOC_VIEWSIZE], gl->view, RL_SHADER_UNIFORM_VEC2, 1);

      for( i = 0; i < gl->ncalls; i++)
      {
         RLGLNVGcall  *call = &gl->calls[i];

         rlglnvg__blendFuncSeparate( gl, &call->blendFunc);
         switch( call->type)
         {
         case RLGLNVG_FILL       : rlglnvg__fill( gl, call);       break;
         case RLGLNVG_CONVEXFILL : rlglnvg__convexFill( gl, call); break;
         case RLGLNVG_STROKE     : rlglnvg__stroke( gl, call);     break;
         case RLGLNVG_TRIANGLES  : rlglnvg__triangles( gl, call);  break;
         }
      }

      rlDisableVertexAttribute( (unsigned int) gl->locVertex);
      rlDisableVertexAttribute( (unsigned int) gl->locTcoord);
      rlDisableVertexArray();
      rlDisableVertexBuffer();
      rlDisableBackfaceCulling();
      rlDisableShader();
      rlDisableTexture();
   }

   // Reset per-frame buffers
   gl->nverts    = 0;
   gl->npaths    = 0;
   gl->ncalls    = 0;
   gl->nuniforms = 0;
}

static void rlglnvg__renderCancel(void* uptr)
{
   RLGLNVGcontext  *gl = (RLGLNVGcontext *) uptr;

   gl->nverts    = 0;
   gl->npaths    = 0;
   gl->ncalls    = 0;
   gl->nuniforms = 0;
}

// ------------------------------------------------------------------
// Buffer allocation
// ------------------------------------------------------------------

static int rlglnvg__maxVertCount(const NVGpath* paths, int npaths)
{
   int   i, count = 0;

   for( i = 0; i < npaths; i++)
   {
      count += paths[i].nfill;
      count += paths[i].nstroke;
   }
   return( count);
}

static RLGLNVGcall* rlglnvg__allocCall(RLGLNVGcontext* gl)
{
   RLGLNVGcall  *ret;

   if( gl->ncalls + 1 > gl->ccalls)
   {
      RLGLNVGcall  *calls;
      int          ccalls;

      ccalls = rlglnvg__maxi( gl->ncalls + 1, 128) + gl->ccalls / 2;
      calls  = (RLGLNVGcall *) realloc( gl->calls, sizeof( RLGLNVGcall) * ccalls);
      if( calls == NULL)
         return( NULL);
      gl->calls  = calls;
      gl->ccalls = ccalls;
   }
   ret = &gl->calls[gl->ncalls++];
   memset( ret, 0, sizeof( RLGLNVGcall));
   return( ret);
}

static int rlglnvg__allocPaths(RLGLNVGcontext* gl, int n)
{
   int   ret;

   if( gl->npaths + n > gl->cpaths)
   {
      RLGLNVGpath  *paths;
      int          cpaths;

      cpaths = rlglnvg__maxi( gl->npaths + n, 128) + gl->cpaths / 2;
      paths  = (RLGLNVGpath *) realloc( gl->paths, sizeof( RLGLNVGpath) * cpaths);
      if( paths == NULL)
         return( -1);
      gl->paths  = paths;
      gl->cpaths = cpaths;
   }
   ret = gl->npaths;
   gl->npaths += n;
   return( ret);
}

static int rlglnvg__allocVerts(RLGLNVGcontext* gl, int n)
{
   int   ret;

   if( gl->nverts + n > gl->cverts)
   {
      NVGvertex  *verts;
      int        cverts;

      cverts = rlglnvg__maxi( gl->nverts + n, 4096) + gl->cverts / 2;
      verts  = (NVGvertex *) realloc( gl->verts, sizeof( NVGvertex) * cverts);
      if( verts == NULL)
         return( -1);
      gl->verts  = verts;
      gl->cverts = cverts;
   }
   ret = gl->nverts;
   gl->nverts += n;
   return( ret);
}

static int rlglnvg__allocFragUniforms(RLGLNVGcontext* gl, int n)
{
   int   ret, structSize;

   structSize = gl->fragSize;
   if( gl->nuniforms + n > gl->cuniforms)
   {
      unsigned char  *uniforms;
      int            cuniforms;

      cuniforms = rlglnvg__maxi( gl->nuniforms + n, 128) + gl->cuniforms / 2;
      uniforms  = (unsigned char *) realloc( gl->uniforms, structSize * cuniforms);
      if( uniforms == NULL)
         return( -1);
      gl->uniforms  = uniforms;
      gl->cuniforms = cuniforms;
   }
   ret = gl->nuniforms * structSize;
   gl->nuniforms += n;
   return( ret);
}

static RLGLNVGfragUniforms* rlglnvg__fragUniformPtr(RLGLNVGcontext* gl, int i)
{
   return( (RLGLNVGfragUniforms *) &gl->uniforms[i]);
}

static void rlglnvg__vset(NVGvertex* vtx, float x, float y, float u, float v)
{
   vtx->x = x;
   vtx->y = y;
   vtx->u = u;
   vtx->v = v;
}

// ------------------------------------------------------------------
// Paint conversion
// ------------------------------------------------------------------

static void rlglnvg__xformToMat3x4(float* m3, float* t)
{
   m3[0]  = t[0];
   m3[1]  = t[1];
   m3[2]  = 0.0f;
   m3[3]  = 0.0f;
   m3[4]  = t[2];
   m3[5]  = t[3];
   m3[6]  = 0.0f;
   m3[7]  = 0.0f;
   m3[8]  = t[4];
   m3[9]  = t[5];
   m3[10] = 1.0f;
   m3[11] = 0.0f;
}

static NVGcolor rlglnvg__premulColor(NVGcolor c)
{
   c.r *= c.a;
   c.g *= c.a;
   c.b *= c.a;
   return( c);
}

static int rlglnvg__convertPaint(RLGLNVGcontext* gl, RLGLNVGfragUniforms* frag, NVGpaint* paint,
                                  NVGscissor* scissor, float width, float fringe, float strokeThr)
{
   RLGLNVGtexture  *tex = NULL;
   float           invxform[6];

   memset( frag, 0, sizeof( *frag));

   frag->innerCol = rlglnvg__premulColor( paint->innerColor);
   frag->outerCol = rlglnvg__premulColor( paint->outerColor);

   if( scissor->extent[0] < -0.5f || scissor->extent[1] < -0.5f)
   {
      memset( frag->scissorMat, 0, sizeof( frag->scissorMat));
      frag->scissorExt[0]   = 1.0f;
      frag->scissorExt[1]   = 1.0f;
      frag->scissorScale[0] = 1.0f;
      frag->scissorScale[1] = 1.0f;
   }
   else
   {
      nvgTransformInverse( invxform, scissor->xform);
      rlglnvg__xformToMat3x4( frag->scissorMat, invxform);
      frag->scissorExt[0]   = scissor->extent[0];
      frag->scissorExt[1]   = scissor->extent[1];
      frag->scissorScale[0] = sqrtf( scissor->xform[0] * scissor->xform[0] + scissor->xform[2] * scissor->xform[2]) / fringe;
      frag->scissorScale[1] = sqrtf( scissor->xform[1] * scissor->xform[1] + scissor->xform[3] * scissor->xform[3]) / fringe;
   }

   memcpy( frag->extent, paint->extent, sizeof( frag->extent));
   frag->strokeMult = (width * 0.5f + fringe * 0.5f) / fringe;
   frag->strokeThr  = strokeThr;

   if( paint->image != 0)
   {
      tex = rlglnvg__findTexture( gl, paint->image);
      if( tex == NULL)
         return( 0);
      if( (tex->flags & NVG_IMAGE_FLIPY) != 0)
      {
         float m1[6], m2[6];

         nvgTransformTranslate( m1, 0.0f, frag->extent[1] * 0.5f);
         nvgTransformMultiply( m1, paint->xform);
         nvgTransformScale( m2, 1.0f, -1.0f);
         nvgTransformMultiply( m2, m1);
         nvgTransformTranslate( m1, 0.0f, -frag->extent[1] * 0.5f);
         nvgTransformMultiply( m1, m2);
         nvgTransformInverse( invxform, m1);
      }
      else
      {
         nvgTransformInverse( invxform, paint->xform);
      }
      frag->type = RLNSVG_SHADER_FILLIMG;

      if( tex->type == NVG_TEXTURE_RGBA)
         frag->texType = (tex->flags & NVG_IMAGE_PREMULTIPLIED) ? 0.0f : 1.0f;
      else
         frag->texType = 2.0f;
   }
   else
   {
      frag->type    = RLNSVG_SHADER_FILLGRAD;
      frag->radius  = paint->radius;
      frag->feather = paint->feather;
      nvgTransformInverse( invxform, paint->xform);
   }

   rlglnvg__xformToMat3x4( frag->paintMat, invxform);
   return( 1);
}

// ------------------------------------------------------------------
// Blend factor conversion
// ------------------------------------------------------------------

static int rlglnvg__convertBlendFuncFactor(int factor)
{
   if( factor == NVG_ZERO)                return( RL_ZERO);
   if( factor == NVG_ONE)                 return( RL_ONE);
   if( factor == NVG_SRC_COLOR)           return( RL_SRC_COLOR);
   if( factor == NVG_ONE_MINUS_SRC_COLOR) return( RL_ONE_MINUS_SRC_COLOR);
   if( factor == NVG_DST_COLOR)           return( RL_DST_COLOR);
   if( factor == NVG_ONE_MINUS_DST_COLOR) return( RL_ONE_MINUS_DST_COLOR);
   if( factor == NVG_SRC_ALPHA)           return( RL_SRC_ALPHA);
   if( factor == NVG_ONE_MINUS_SRC_ALPHA) return( RL_ONE_MINUS_SRC_ALPHA);
   if( factor == NVG_DST_ALPHA)           return( RL_DST_ALPHA);
   if( factor == NVG_ONE_MINUS_DST_ALPHA) return( RL_ONE_MINUS_DST_ALPHA);
   if( factor == NVG_SRC_ALPHA_SATURATE)  return( RL_SRC_ALPHA_SATURATE);
   return( -1);
}

static RLGLNVGblend rlglnvg__blendCompositeOperation(NVGcompositeOperationState op)
{
   RLGLNVGblend   blend;

   blend.srcRGB   = rlglnvg__convertBlendFuncFactor( op.srcRGB);
   blend.dstRGB   = rlglnvg__convertBlendFuncFactor( op.dstRGB);
   blend.srcAlpha = rlglnvg__convertBlendFuncFactor( op.srcAlpha);
   blend.dstAlpha = rlglnvg__convertBlendFuncFactor( op.dstAlpha);
   if( blend.srcRGB == -1 || blend.dstRGB == -1 || blend.srcAlpha == -1 || blend.dstAlpha == -1)
   {
      blend.srcRGB   = RL_ONE;
      blend.dstRGB   = RL_ONE_MINUS_SRC_ALPHA;
      blend.srcAlpha = RL_ONE;
      blend.dstAlpha = RL_ONE_MINUS_SRC_ALPHA;
   }
   return( blend);
}

// ------------------------------------------------------------------
// Render command buffering
// ------------------------------------------------------------------

static void rlglnvg__renderFill(void* uptr, NVGpaint* paint, NVGcompositeOperationState compositeOperation,
                                 NVGscissor* scissor, float fringe, const float* bounds,
                                 const NVGpath* paths, int npaths)
{
   RLGLNVGcontext       *gl = (RLGLNVGcontext *) uptr;
   RLGLNVGcall          *call;
   RLGLNVGfragUniforms  *frag;
   NVGvertex            *quad;
   int                  i, maxverts, offset;

   call = rlglnvg__allocCall( gl);
   if( call == NULL)
      return;

   call->type          = RLGLNVG_FILL;
   call->triangleCount = 4;
   call->pathOffset    = rlglnvg__allocPaths( gl, npaths);
   if( call->pathOffset == -1) goto error;
   call->pathCount = npaths;
   call->image     = paint->image;
   call->blendFunc = rlglnvg__blendCompositeOperation( compositeOperation);

   if( npaths == 1 && paths[0].convex)
   {
      call->type          = RLGLNVG_CONVEXFILL;
      call->triangleCount = 0;
   }

   maxverts = rlglnvg__maxVertCount( paths, npaths) + call->triangleCount;
   offset   = rlglnvg__allocVerts( gl, maxverts);
   if( offset == -1) goto error;

   for( i = 0; i < npaths; i++)
   {
      RLGLNVGpath    *copy = &gl->paths[call->pathOffset + i];
      const NVGpath  *path = &paths[i];

      memset( copy, 0, sizeof( RLGLNVGpath));
      if( path->nfill > 0)
      {
         copy->fillOffset = offset;
         copy->fillCount  = path->nfill;
         memcpy( &gl->verts[offset], path->fill, sizeof( NVGvertex) * path->nfill);
         offset += path->nfill;
      }
      if( path->nstroke > 0)
      {
         copy->strokeOffset = offset;
         copy->strokeCount  = path->nstroke;
         memcpy( &gl->verts[offset], path->stroke, sizeof( NVGvertex) * path->nstroke);
         offset += path->nstroke;
      }
   }

   if( call->type == RLGLNVG_FILL)
   {
      call->triangleOffset = offset;
      quad = &gl->verts[call->triangleOffset];
      rlglnvg__vset( &quad[0], bounds[2], bounds[3], 0.5f, 1.0f);
      rlglnvg__vset( &quad[1], bounds[2], bounds[1], 0.5f, 1.0f);
      rlglnvg__vset( &quad[2], bounds[0], bounds[3], 0.5f, 1.0f);
      rlglnvg__vset( &quad[3], bounds[0], bounds[1], 0.5f, 1.0f);

      call->uniformOffset = rlglnvg__allocFragUniforms( gl, 2);
      if( call->uniformOffset == -1) goto error;
      frag = rlglnvg__fragUniformPtr( gl, call->uniformOffset);
      memset( frag, 0, sizeof( *frag));
      frag->strokeThr = -1.0f;
      frag->type      = RLNSVG_SHADER_SIMPLE;
      rlglnvg__convertPaint( gl, rlglnvg__fragUniformPtr( gl, call->uniformOffset + gl->fragSize), paint, scissor, fringe, fringe, -1.0f);
   }
   else
   {
      call->uniformOffset = rlglnvg__allocFragUniforms( gl, 1);
      if( call->uniformOffset == -1) goto error;
      rlglnvg__convertPaint( gl, rlglnvg__fragUniformPtr( gl, call->uniformOffset), paint, scissor, fringe, fringe, -1.0f);
   }
   return;

error:
   if( gl->ncalls > 0) gl->ncalls--;
}

static void rlglnvg__renderStroke(void* uptr, NVGpaint* paint, NVGcompositeOperationState compositeOperation,
                                   NVGscissor* scissor, float fringe, float strokeWidth,
                                   const NVGpath* paths, int npaths)
{
   RLGLNVGcontext  *gl = (RLGLNVGcontext *) uptr;
   RLGLNVGcall     *call;
   int             i, maxverts, offset;

   call = rlglnvg__allocCall( gl);
   if( call == NULL)
      return;

   call->type       = RLGLNVG_STROKE;
   call->pathOffset = rlglnvg__allocPaths( gl, npaths);
   if( call->pathOffset == -1) goto error;
   call->pathCount = npaths;
   call->image     = paint->image;
   call->blendFunc = rlglnvg__blendCompositeOperation( compositeOperation);

   maxverts = rlglnvg__maxVertCount( paths, npaths);
   offset   = rlglnvg__allocVerts( gl, maxverts);
   if( offset == -1) goto error;

   for( i = 0; i < npaths; i++)
   {
      RLGLNVGpath    *copy = &gl->paths[call->pathOffset + i];
      const NVGpath  *path = &paths[i];

      memset( copy, 0, sizeof( RLGLNVGpath));
      if( path->nstroke)
      {
         copy->strokeOffset = offset;
         copy->strokeCount  = path->nstroke;
         memcpy( &gl->verts[offset], path->stroke, sizeof( NVGvertex) * path->nstroke);
         offset += path->nstroke;
      }
   }

   if( gl->flags & NVG_STENCIL_STROKES)
   {
      call->uniformOffset = rlglnvg__allocFragUniforms( gl, 2);
      if( call->uniformOffset == -1) goto error;
      rlglnvg__convertPaint( gl, rlglnvg__fragUniformPtr( gl, call->uniformOffset), paint, scissor, strokeWidth, fringe, -1.0f);
      rlglnvg__convertPaint( gl, rlglnvg__fragUniformPtr( gl, call->uniformOffset + gl->fragSize), paint, scissor, strokeWidth, fringe, 1.0f - 0.5f / 255.0f);
   }
   else
   {
      call->uniformOffset = rlglnvg__allocFragUniforms( gl, 1);
      if( call->uniformOffset == -1) goto error;
      rlglnvg__convertPaint( gl, rlglnvg__fragUniformPtr( gl, call->uniformOffset), paint, scissor, strokeWidth, fringe, -1.0f);
   }
   return;

error:
   if( gl->ncalls > 0) gl->ncalls--;
}

static void rlglnvg__renderTriangles(void* uptr, NVGpaint* paint, NVGcompositeOperationState compositeOperation,
                                      NVGscissor* scissor, const NVGvertex* verts, int nverts, float fringe)
{
   RLGLNVGcontext       *gl = (RLGLNVGcontext *) uptr;
   RLGLNVGcall          *call;
   RLGLNVGfragUniforms  *frag;

   call = rlglnvg__allocCall( gl);
   if( call == NULL)
      return;

   call->type     = RLGLNVG_TRIANGLES;
   call->image    = paint->image;
   call->blendFunc = rlglnvg__blendCompositeOperation( compositeOperation);

   call->triangleOffset = rlglnvg__allocVerts( gl, nverts);
   if( call->triangleOffset == -1) goto error;
   call->triangleCount = nverts;

   memcpy( &gl->verts[call->triangleOffset], verts, sizeof( NVGvertex) * nverts);

   call->uniformOffset = rlglnvg__allocFragUniforms( gl, 1);
   if( call->uniformOffset == -1) goto error;
   frag = rlglnvg__fragUniformPtr( gl, call->uniformOffset);
   rlglnvg__convertPaint( gl, frag, paint, scissor, 1.0f, fringe, -1.0f);
   frag->type = (paint->clearType) ? RLNSVG_SHADER_CLEARTYPE : RLNSVG_SHADER_IMG;
   return;

error:
   if( gl->ncalls > 0) gl->ncalls--;
}

// ------------------------------------------------------------------
// Delete
// ------------------------------------------------------------------

static void rlglnvg__renderDelete(void* uptr)
{
   RLGLNVGcontext  *gl = (RLGLNVGcontext *) uptr;
   int             i;

   if( gl == NULL)
      return;

   rlUnloadShaderProgram( gl->shaderProg);
   rlUnloadVertexArray( gl->vertArr);
   rlUnloadVertexBuffer( gl->vertBuf);

   for( i = 0; i < gl->ntextures; i++)
   {
      if( gl->textures[i].tex != 0 && (gl->textures[i].flags & NVG_IMAGE_NODELETE) == 0)
         rlUnloadTexture( gl->textures[i].tex);
   }
   free( gl->textures);
   free( gl->paths);
   free( gl->verts);
   free( gl->uniforms);
   free( gl->calls);
   free( gl);
}

// ------------------------------------------------------------------
// Public API
// ------------------------------------------------------------------

MULLE__NANOVG_RLGL_GLOBAL
NVGcontext* nvgCreateRLGL(int flags)
{
   NVGparams        params;
   NVGcontext      *ctx = NULL;
   RLGLNVGcontext  *gl;

   memset( &params, 0, sizeof( params));

   switch( flags & (NVG_PATH_ONLY | NVG_FONT_ONLY))
   {
   case 0 :
      params.contextType = NVG_FULL_CONTEXT;
      params.cStates     = NVG_MAX_STATES;
      break;
   case NVG_PATH_ONLY :
      params.contextType = NVG_PATH_CONTEXT;
      params.cStates     = NVG_MIN_STATES;
      break;
   case NVG_FONT_ONLY :
      params.contextType = NVG_FONT_CONTEXT;
      params.cStates     = NVG_MAX_STATES;
      break;
   default :
      return( NULL);
   }

   if( params.contextType == NVG_FULL_CONTEXT)
   {
      gl = (RLGLNVGcontext *) malloc( sizeof( RLGLNVGcontext));
      if( gl == NULL)
         return( NULL);
      memset( gl, 0, sizeof( RLGLNVGcontext));

      params.renderCreate             = rlglnvg__renderCreate;
      params.renderCreateTexture      = rlglnvg__renderCreateTexture;
      params.renderDeleteTexture      = rlglnvg__renderDeleteTexture;
      params.renderDefineTexture      = rlglnvg__renderDefineTexture;
      params.renderForgetTexture      = rlglnvg__renderForgetTexture;
      params.renderGetTextureInfo     = rlglnvg__renderGetTextureInfo;
      params.renderValidatePaintTexture = rlglnvg__renderValidatePaintTexture;
      params.renderGetTextureSize     = rlglnvg__renderGetTextureSize;
      params.renderUpdateTexture      = rlglnvg__renderUpdateTexture;
      params.renderViewport           = rlglnvg__renderViewport;
      params.renderCancel             = rlglnvg__renderCancel;
      params.renderFlush              = rlglnvg__renderFlush;
      params.renderFill               = rlglnvg__renderFill;
      params.renderStroke             = rlglnvg__renderStroke;
      params.renderTriangles          = rlglnvg__renderTriangles;
      params.renderDelete             = rlglnvg__renderDelete;
      params.userPtr                  = gl;
      gl->flags = flags;
   }

   params.edgeAntiAlias = flags & NVG_ANTIALIAS ? 1 : 0;
   params.clearType     = flags & NVG_CLEARTYPE ? 1 : 0;

   ctx = nvgCreateInternal( &params);
   if( ctx == NULL)
      return( NULL);

   return( ctx);
}

MULLE__NANOVG_RLGL_GLOBAL
void nvgDeleteRLGL(NVGcontext* ctx)
{
   nvgDeleteInternal( ctx);
}

MULLE__NANOVG_RLGL_GLOBAL
int nvglCreateImageFromHandleRLGL(NVGcontext* ctx, unsigned int textureId, int w, int h, int imageFlags)
{
   RLGLNVGcontext  *gl = (RLGLNVGcontext *) nvgInternalParams( ctx)->userPtr;
   RLGLNVGtexture  *tex;

   tex = rlglnvg__allocTexture( gl);
   if( tex == NULL)
      return( 0);
   tex->type   = NVG_TEXTURE_RGBA;
   tex->tex    = textureId;
   tex->flags  = imageFlags;
   tex->width  = w;
   tex->height = h;
   return( tex->id);
}

MULLE__NANOVG_RLGL_GLOBAL
unsigned int nvglImageHandleRLGL(NVGcontext* ctx, int image)
{
   RLGLNVGcontext  *gl = (RLGLNVGcontext *) nvgInternalParams( ctx)->userPtr;
   RLGLNVGtexture  *tex;

   tex = rlglnvg__findTexture( gl, image);
   if( tex == NULL)
      return( 0);
   return( tex->tex);
}

#endif // NANOVG_RLGL_IMPLEMENTATION
