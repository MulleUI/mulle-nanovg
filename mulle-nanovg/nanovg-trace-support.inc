#ifndef NANOVG_TRACE_SUPPORT_INC__
#define NANOVG_TRACE_SUPPORT_INC__

#include <stdio.h>

#ifndef NVG_TRACECODE_DISABLED
#include "nanovg-trace-functions.h"
#endif


// Frame Management Functions
#ifndef NVG_TRACECODE_DISABLED
static inline void nvgBeginFrame_enter(NVGcontext* ctx, float windowWidth, float windowHeight, float devicePixelRatio)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return;
   _nvgBeginFrame_enter( fp, ctx->traceName, windowWidth, windowHeight, devicePixelRatio);
}
#else
#define nvgBeginFrame_enter( ctx, windowWidth, windowHeight, devicePixelRatio)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgCancelFrame_enter(NVGcontext* ctx)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgCancelFrame_enter( fp, ctx->traceName);
}
#else
#define nvgCancelFrame_enter( ctx)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgChangeFrame_enter(NVGcontext* ctx, float windowWidth, float windowHeight, float devicePixelRatio)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgChangeFrame_enter( fp, ctx->traceName, windowWidth, windowHeight, devicePixelRatio);
}
#else
#define nvgChangeFrame_enter( ctx, windowWidth, windowHeight, devicePixelRatio)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgFlushFrame_enter(NVGcontext* ctx)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgFlushFrame_enter( fp, ctx->traceName);
}
#else
#define nvgFlushFrame_enter( ctx)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgEndFrame_enter(NVGcontext* ctx)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgEndFrame_enter( fp, ctx->traceName);
}
#else
#define nvgEndFrame_enter( ctx)
#endif


// Composite Operation Functions
#ifndef NVG_TRACECODE_DISABLED
static inline void nvgGlobalCompositeOperation_enter(NVGcontext* ctx, int op)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgGlobalCompositeOperation_enter( fp, ctx->traceName, op);
}
#else
#define nvgGlobalCompositeOperation_enter( ctx, op)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgGlobalCompositeBlendFunc_enter(NVGcontext* ctx, int sfactor, int dfactor)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgGlobalCompositeBlendFunc_enter( fp, ctx->traceName, sfactor, dfactor);
}
#else
#define nvgGlobalCompositeBlendFunc_enter( ctx, sfactor, dfactor)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgGlobalCompositeBlendFuncSeparate_enter(NVGcontext* ctx, int srcRGB, int dstRGB, int srcAlpha, int dstAlpha)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgGlobalCompositeBlendFuncSeparate_enter( fp, ctx->traceName, srcRGB, dstRGB, srcAlpha, dstAlpha);
}
#else
#define nvgGlobalCompositeBlendFuncSeparate_enter( ctx, srcRGB, dstRGB, srcAlpha, dstAlpha)
#endif

// State Management Functions
#ifndef NVG_TRACECODE_DISABLED
static inline void nvgSave_enter(NVGcontext* ctx)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgSave_enter( fp, ctx->traceName);
}
#else
#define nvgSave_enter( ctx)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgRestore_enter(NVGcontext* ctx)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgRestore_enter( fp, ctx->traceName);
}
#else
#define nvgRestore_enter( ctx)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgReset_enter(NVGcontext* ctx)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgReset_enter( fp, ctx->traceName);
}
#else
#define nvgReset_enter( ctx)
#endif

// Render Style Functions
#ifndef NVG_TRACECODE_DISABLED
static inline void nvgShapeAntiAlias_enter(NVGcontext* ctx, int enabled)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgShapeAntiAlias_enter( fp, ctx->traceName, enabled);
}
#else
#define nvgShapeAntiAlias_enter( ctx, enabled)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgStrokeColor_enter(NVGcontext* ctx, NVGcolor color)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgStrokeColor_enter( fp, ctx->traceName, color);
}
#else
#define nvgStrokeColor_enter( ctx, color)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgStrokePaint_enter(NVGcontext* ctx, NVGpaint paint)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgStrokePaint_enter( fp, ctx->traceName, paint);
}
#else
#define nvgStrokePaint_enter( ctx, paint)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgFillColor_enter(NVGcontext* ctx, NVGcolor color)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgFillColor_enter( fp, ctx->traceName, color);
}
#else
#define nvgFillColor_enter( ctx, color)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgClearType_enter(NVGcontext* ctx, int enabled)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgClearType_enter( fp, ctx->traceName, enabled);
}
#else
#define nvgClearType_enter( ctx, enabled)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgTextColor_enter(NVGcontext* ctx, NVGcolor color)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgTextColor_enter( fp, ctx->traceName, color);
}
#else
#define nvgTextColor_enter( ctx, color)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgTextBackgroundColor_enter(NVGcontext* ctx, NVGcolor color)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgTextBackgroundColor_enter( fp, ctx->traceName, color);
}
#else
#define nvgTextBackgroundColor_enter( ctx, color)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgFillPaint_enter(NVGcontext* ctx, NVGpaint paint)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgFillPaint_enter( fp, ctx->traceName, paint);
}
#else
#define nvgFillPaint_enter( ctx, paint)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgMiterLimit_enter(NVGcontext* ctx, float limit)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgMiterLimit_enter( fp, ctx->traceName, limit);
}
#else
#define nvgMiterLimit_enter( ctx, limit)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgStrokeWidth_enter(NVGcontext* ctx, float size)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgStrokeWidth_enter( fp, ctx->traceName, size);
}
#else
#define nvgStrokeWidth_enter( ctx, size)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgLineCap_enter(NVGcontext* ctx, int cap)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgLineCap_enter( fp, ctx->traceName, cap);
}
#else
#define nvgLineCap_enter( ctx, cap)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgLineJoin_enter(NVGcontext* ctx, int join)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgLineJoin_enter( fp, ctx->traceName, join);
}
#else
#define nvgLineJoin_enter( ctx, join)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgGlobalAlpha_enter(NVGcontext* ctx, float alpha)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgGlobalAlpha_enter( fp, ctx->traceName, alpha);
}
#else
#define nvgGlobalAlpha_enter( ctx, alpha)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgBezierTessellation_enter(NVGcontext* ctx, int tess)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgBezierTessellation_enter( fp, ctx->traceName, tess);
}
#else
#define nvgBezierTessellation_enter( ctx, tess)
#endif

// Transform Functions
#ifndef NVG_TRACECODE_DISABLED
static inline void nvgResetTransform_enter(NVGcontext* ctx)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgResetTransform_enter( fp, ctx->traceName);
}
#else
#define nvgResetTransform_enter( ctx)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgTransform_enter(NVGcontext* ctx, float a, float b, float c, float d, float e, float f)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgTransform_enter( fp, ctx->traceName, a, b, c, d, e, f);
}
#else
#define nvgTransform_enter( ctx, a, b, c, d, e, f)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgTranslate_enter(NVGcontext* ctx, float x, float y)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgTranslate_enter( fp, ctx->traceName, x, y);
}
#else
#define nvgTranslate_enter( ctx, x, y)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgRotate_enter(NVGcontext* ctx, float angle)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgRotate_enter( fp, ctx->traceName, angle);
}
#else
#define nvgRotate_enter( ctx, angle)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgSkewX_enter(NVGcontext* ctx, float angle)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgSkewX_enter( fp, ctx->traceName, angle);
}
#else
#define nvgSkewX_enter( ctx, angle)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgSkewY_enter(NVGcontext* ctx, float angle)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgSkewY_enter( fp, ctx->traceName, angle);
}
#else
#define nvgSkewY_enter( ctx, angle)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgScale_enter(NVGcontext* ctx, float x, float y)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgScale_enter( fp, ctx->traceName, x, y);
}
#else
#define nvgScale_enter( ctx, x, y)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgCurrentTransform_enter(NVGcontext* ctx, float* xform)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgCurrentTransform_enter( fp, ctx->traceName, xform);
}
#else
#define nvgCurrentTransform_enter( ctx, xform)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgCurrentTransform_leave(NVGcontext* ctx, float* xform)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgCurrentTransform_leave( fp, ctx->traceName, xform);
}
#else
#define nvgCurrentTransform_leave( ctx, xform)
#endif

// Image Functions
#ifndef NVG_TRACECODE_DISABLED
static inline void nvgUpdateImage_enter(NVGcontext* ctx, int image, const unsigned char* data)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgUpdateImage_enter( fp, ctx->traceName, image, data);
}
#else
#define nvgUpdateImage_enter( ctx, image, data)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgImageSize_enter(NVGcontext* ctx, int image, int* w, int* h)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgImageSize_enter( fp, ctx->traceName, image, w, h);
}
#else
#define nvgImageSize_enter( ctx, image, w, h)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgImageSize_leave(NVGcontext* ctx, int image, int* w, int* h)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgImageSize_leave( fp, ctx->traceName, image, w, h);
}
#else
#define nvgImageSize_leave( ctx, image, w, h)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgDeleteImage_enter(NVGcontext* ctx, int image)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgDeleteImage_enter( fp, ctx->traceName, image);
}
#else
#define nvgDeleteImage_enter( ctx, image)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgForgetImageTexture_enter(NVGcontext* ctx, int image)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgForgetImageTexture_enter( fp, ctx->traceName, image);
}
#else
#define nvgForgetImageTexture_enter( ctx, image)
#endif

// Scissor Functions
#ifndef NVG_TRACECODE_DISABLED
static inline void nvgScissor_enter(NVGcontext* ctx, float x, float y, float w, float h)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgScissor_enter( fp, ctx->traceName, x, y, w, h);
}
#else
#define nvgScissor_enter( ctx, x, y, w, h)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgIntersectScissor_enter(NVGcontext* ctx, float x, float y, float w, float h)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgIntersectScissor_enter( fp, ctx->traceName, x, y, w, h);
}
#else
#define nvgIntersectScissor_enter( ctx, x, y, w, h)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgResetScissor_enter(NVGcontext* ctx)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgResetScissor_enter( fp, ctx->traceName);
}
#else
#define nvgResetScissor_enter( ctx)
#endif

// Path Functions
#ifndef NVG_TRACECODE_DISABLED
static inline void nvgBeginPath_enter(NVGcontext* ctx)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgBeginPath_enter( fp, ctx->traceName);
}
#else
#define nvgBeginPath_enter( ctx)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgMoveTo_enter(NVGcontext* ctx, float x, float y)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgMoveTo_enter( fp, ctx->traceName, x, y);
}
#else
#define nvgMoveTo_enter( ctx, x, y)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgLineTo_enter(NVGcontext* ctx, float x, float y)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgLineTo_enter( fp, ctx->traceName, x, y);
}
#else
#define nvgLineTo_enter( ctx, x, y)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgBezierTo_enter(NVGcontext* ctx, float c1x, float c1y, float c2x, float c2y, float x, float y)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgBezierTo_enter( fp, ctx->traceName, c1x, c1y, c2x, c2y, x, y);
}
#else
#define nvgBezierTo_enter( ctx, c1x, c1y, c2x, c2y, x, y)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgQuadTo_enter(NVGcontext* ctx, float cx, float cy, float x, float y)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgQuadTo_enter( fp, ctx->traceName, cx, cy, x, y);
}
#else
#define nvgQuadTo_enter( ctx, cx, cy, x, y)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgArcTo_enter(NVGcontext* ctx, float x1, float y1, float x2, float y2, float radius)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgArcTo_enter( fp, ctx->traceName, x1, y1, x2, y2, radius);
}
#else
#define nvgArcTo_enter( ctx, x1, y1, x2, y2, radius)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgClosePath_enter(NVGcontext* ctx)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgClosePath_enter( fp, ctx->traceName);
}
#else
#define nvgClosePath_enter( ctx)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgPathWinding_enter(NVGcontext* ctx, int dir)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgPathWinding_enter( fp, ctx->traceName, dir);
}
#else
#define nvgPathWinding_enter( ctx, dir)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgArc_enter(NVGcontext* ctx, float cx, float cy, float r, float a0, float a1, int dir)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgArc_enter( fp, ctx->traceName, cx, cy, r, a0, a1, dir);
}
#else
#define nvgArc_enter( ctx, cx, cy, r, a0, a1, dir)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgRect_enter(NVGcontext* ctx, float x, float y, float w, float h)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgRect_enter( fp, ctx->traceName, x, y, w, h);
}
#else
#define nvgRect_enter( ctx, x, y, w, h)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgRoundedRect_enter(NVGcontext* ctx, float x, float y, float w, float h, float r)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgRoundedRect_enter( fp, ctx->traceName, x, y, w, h, r);
}
#else
#define nvgRoundedRect_enter( ctx, x, y, w, h, r)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgRoundedRectVarying_enter(NVGcontext* ctx, float x, float y, float w, float h, float radTopLeft, float radTopRight, float radBottomRight, float radBottomLeft)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgRoundedRectVarying_enter( fp, ctx->traceName, x, y, w, h, radTopLeft, radTopRight, radBottomRight, radBottomLeft);
}
#else
#define nvgRoundedRectVarying_enter( ctx, x, y, w, h, radTopLeft, radTopRight, radBottomRight, radBottomLeft)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgEllipse_enter(NVGcontext* ctx, float cx, float cy, float rx, float ry)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgEllipse_enter( fp, ctx->traceName, cx, cy, rx, ry);
}
#else
#define nvgEllipse_enter( ctx, cx, cy, rx, ry)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgCircle_enter(NVGcontext* ctx, float cx, float cy, float r)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgCircle_enter( fp, ctx->traceName, cx, cy, r);
}
#else
#define nvgCircle_enter( ctx, cx, cy, r)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgFill_enter(NVGcontext* ctx)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgFill_enter( fp, ctx->traceName);
}
#else
#define nvgFill_enter( ctx)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgStroke_enter(NVGcontext* ctx)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgStroke_enter( fp, ctx->traceName);
}
#else
#define nvgStroke_enter( ctx)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgPrecompilePath_enter(NVGcontext *ctx)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgPrecompilePath_enter( fp, ctx->traceName);
}
#else
#define nvgPrecompilePath_enter( ctx)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgCopyPath_enter(NVGcontext* ctx, NVGcontext *path)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgCopyPath_enter( fp, ctx->traceName, path);
}
#else
#define nvgCopyPath_enter( ctx, path)
#endif

// Font Functions
#ifndef NVG_TRACECODE_DISABLED
static inline void nvgFontSize_enter(NVGcontext* ctx, float size)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgFontSize_enter( fp, ctx->traceName, size);
}
#else
#define nvgFontSize_enter( ctx, size)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgFontBlur_enter(NVGcontext* ctx, float blur)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgFontBlur_enter( fp, ctx->traceName, blur);
}
#else
#define nvgFontBlur_enter( ctx, blur)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgTextLetterSpacing_enter(NVGcontext* ctx, float spacing)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgTextLetterSpacing_enter( fp, ctx->traceName, spacing);
}
#else
#define nvgTextLetterSpacing_enter( ctx, spacing)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgTextLineHeight_enter(NVGcontext* ctx, float lineHeight)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgTextLineHeight_enter( fp, ctx->traceName, lineHeight);
}
#else
#define nvgTextLineHeight_enter( ctx, lineHeight)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgTextAlign_enter(NVGcontext* ctx, int align)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgTextAlign_enter( fp, ctx->traceName, align);
}
#else
#define nvgTextAlign_enter( ctx, align)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgFontFaceId_enter(NVGcontext* ctx, int font)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgFontFaceId_enter( fp, ctx->traceName, font);
}
#else
#define nvgFontFaceId_enter( ctx, font)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgFontFace_enter(NVGcontext* ctx, const char* font)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgFontFace_enter( fp, ctx->traceName, font);
}
#else
#define nvgFontFace_enter( ctx, font)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgText_enter(NVGcontext* ctx, float x, float y, const char* string, const char* end)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgText_enter( fp, ctx->traceName, x, y, string, end);
}
static inline void nvgText_leave(NVGcontext* ctx, float x, float y, const char* string, const char* end, float return_value)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgText_leave( fp, ctx->traceName, x, y, string, end, return_value);
}
#else
#define nvgText_enter( ctx, x, y, string, end)
#define nvgText_leave( ctx, x, y, string, end, return_value)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgTextBox_enter(NVGcontext* ctx, float x, float y, float breakRowWidth, const char* string, const char* end)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgTextBox_enter( fp, ctx->traceName, x, y, breakRowWidth, string, end);
}
#else
#define nvgTextBox_enter( ctx, x, y, breakRowWidth, string, end)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgResetFallbackFontsId_enter(NVGcontext* ctx, int baseFont)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgResetFallbackFontsId_enter( fp, ctx->traceName, baseFont);
}
#else
#define nvgResetFallbackFontsId_enter( ctx, baseFont)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgResetFallbackFonts_enter(NVGcontext* ctx, const char* baseFont)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgResetFallbackFonts_enter( fp, ctx->traceName, baseFont);
}
#else
#define nvgResetFallbackFonts_enter( ctx, baseFont)
#endif

// Image Creation Functions
#ifndef NVG_TRACECODE_DISABLED
static inline void nvgCreateImage_enter(NVGcontext* ctx, const char* filename, int imageFlags)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgCreateImage_enter( fp, ctx->traceName, filename, imageFlags);
}
#else
#define nvgCreateImage_enter( ctx, filename, imageFlags)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgCreateImage_leave(NVGcontext* ctx, const char* filename, int imageFlags, int return_value)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgCreateImage_leave( fp, ctx->traceName, filename, imageFlags, return_value);
}
#else
#define nvgCreateImage_leave( ctx, filename, imageFlags, return_value)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgCreateImageMem_enter(NVGcontext* ctx, int imageFlags, unsigned char* data, int ndata)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgCreateImageMem_enter( fp, ctx->traceName, imageFlags, data, ndata);
}
#else
#define nvgCreateImageMem_enter( ctx, imageFlags, data, ndata)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgCreateImageMem_leave(NVGcontext* ctx, int imageFlags, unsigned char* data, int ndata, int return_value)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgCreateImageMem_leave( fp, ctx->traceName, imageFlags, data, ndata, return_value);
}
#else
#define nvgCreateImageMem_leave( ctx, imageFlags, data, ndata, return_value)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgCreateImageRGBA_enter(NVGcontext* ctx, int w, int h, int imageFlags, const unsigned char* data)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgCreateImageRGBA_enter( fp, ctx->traceName, w, h, imageFlags, data);
}
#else
#define nvgCreateImageRGBA_enter( ctx, w, h, imageFlags, data)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgCreateImageRGBA_leave(NVGcontext* ctx, int w, int h, int imageFlags, const unsigned char* data, int return_value)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgCreateImageRGBA_leave( fp, ctx->traceName, w, h, imageFlags, data, return_value);
}
#else
#define nvgCreateImageRGBA_leave( ctx, w, h, imageFlags, data, return_value)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgCreateImageTexture_enter(NVGcontext* ctx, int w, int h, int imageFlags, int type, void **texture)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgCreateImageTexture_enter( fp, ctx->traceName, w, h, imageFlags, type, texture);
}
#else
#define nvgCreateImageTexture_enter( ctx, w, h, imageFlags, type, texture)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgCreateImageTexture_leave(NVGcontext* ctx, int w, int h, int imageFlags, int type, void **texture, int return_value)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgCreateImageTexture_leave( fp, ctx->traceName, w, h, imageFlags, type, texture, return_value);
}
#else
#define nvgCreateImageTexture_leave( ctx, w, h, imageFlags, type, texture, return_value)
#endif

// Image Creation Functions (continued)
#ifndef NVG_TRACECODE_DISABLED
static inline void nvgImageTextureInfo_enter(NVGcontext* ctx, int image, int *imageFlags, int *type, void **texture)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgImageTextureInfo_enter( fp, ctx->traceName, image, imageFlags, type, texture);
}
#else
#define nvgImageTextureInfo_enter( ctx, image, imageFlags, type, texture)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgImageTextureInfo_leave(NVGcontext* ctx, int image, int *imageFlags, int *type, void **texture)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgImageTextureInfo_leave( fp, ctx->traceName, image, imageFlags, type, texture);
}
#else
#define nvgImageTextureInfo_leave( ctx, image, imageFlags, type, texture)
#endif

// Paint Functions
#ifndef NVG_TRACECODE_DISABLED
static inline void nvgLinearGradient_enter(NVGcontext* ctx, float sx, float sy, float ex, float ey, NVGcolor icol, NVGcolor ocol)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgLinearGradient_enter( fp, ctx->traceName, sx, sy, ex, ey, icol, ocol);
}
#else
#define nvgLinearGradient_enter( ctx, sx, sy, ex, ey, icol, ocol)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgLinearGradient_leave(NVGcontext* ctx, float sx, float sy, float ex, float ey, NVGcolor icol, NVGcolor ocol, NVGpaint return_value)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgLinearGradient_leave( fp, ctx->traceName, sx, sy, ex, ey, icol, ocol, return_value);
}
#else
#define nvgLinearGradient_leave( ctx, sx, sy, ex, ey, icol, ocol, return_value)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgBoxGradient_enter(NVGcontext* ctx, float x, float y, float w, float h, float r, float f, NVGcolor icol, NVGcolor ocol)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgBoxGradient_enter( fp, ctx->traceName, x, y, w, h, r, f, icol, ocol);
}
#else
#define nvgBoxGradient_enter( ctx, x, y, w, h, r, f, icol, ocol)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgBoxGradient_leave(NVGcontext* ctx, float x, float y, float w, float h, float r, float f, NVGcolor icol, NVGcolor ocol, NVGpaint return_value)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgBoxGradient_leave( fp, ctx->traceName, x, y, w, h, r, f, icol, ocol, return_value);
}
#else
#define nvgBoxGradient_leave( ctx, x, y, w, h, r, f, icol, ocol, return_value)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgRadialGradient_enter(NVGcontext* ctx, float cx, float cy, float inr, float outr, NVGcolor icol, NVGcolor ocol)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgRadialGradient_enter( fp, ctx->traceName, cx, cy, inr, outr, icol, ocol);
}
#else
#define nvgRadialGradient_enter( ctx, cx, cy, inr, outr, icol, ocol)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgRadialGradient_leave(NVGcontext* ctx, float cx, float cy, float inr, float outr, NVGcolor icol, NVGcolor ocol, NVGpaint return_value)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgRadialGradient_leave( fp, ctx->traceName, cx, cy, inr, outr, icol, ocol, return_value);
}
#else
#define nvgRadialGradient_leave( ctx, cx, cy, inr, outr, icol, ocol, return_value)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgImagePattern_enter(NVGcontext* ctx, float ox, float oy, float ex, float ey, float angle, int image, float alpha)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgImagePattern_enter( fp, ctx->traceName, ox, oy, ex, ey, angle, image, alpha);
}
#else
#define nvgImagePattern_enter( ctx, ox, oy, ex, ey, angle, image, alpha)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgImagePattern_leave(NVGcontext* ctx, float ox, float oy, float ex, float ey, float angle, int image, float alpha, NVGpaint return_value)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgImagePattern_leave( fp, ctx->traceName, ox, oy, ex, ey, angle, image, alpha, return_value);
}
#else
#define nvgImagePattern_leave( ctx, ox, oy, ex, ey, angle, image, alpha, return_value)
#endif

// Internal/Utility Functions
#ifndef NVG_TRACECODE_DISABLED
static inline void nvgSetScissor_enter(NVGcontext* ctx, NVGscissor *scissor)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgSetScissor_enter( fp, ctx->traceName, scissor);
}
#else
#define nvgSetScissor_enter( ctx, scissor)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgGetScissor_enter(NVGcontext* ctx, NVGscissor *scissor)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgGetScissor_enter( fp, ctx->traceName, scissor);
}
#else
#define nvgGetScissor_enter( ctx, scissor)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgGetScissor_leave(NVGcontext* ctx, NVGscissor *scissor)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgGetScissor_leave( fp, ctx->traceName, scissor);
}
#else
#define nvgGetScissor_leave( ctx, scissor)
#endif

// Font Functions (continued)
#ifndef NVG_TRACECODE_DISABLED
static inline void nvgCreateFont_enter(NVGcontext* ctx, const char* name, const char* filename)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgCreateFont_enter( fp, ctx->traceName, name, filename);
}
#else
#define nvgCreateFont_enter( ctx, name, filename)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgCreateFont_leave(NVGcontext* ctx, const char* name, const char* filename, int return_value)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgCreateFont_leave( fp, ctx->traceName, name, filename, return_value);
}
#else
#define nvgCreateFont_leave( ctx, name, filename, return_value)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgCreateFontAtIndex_enter(NVGcontext* ctx, const char* name, const char* filename, const int fontIndex)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgCreateFontAtIndex_enter( fp, ctx->traceName, name, filename, fontIndex);
}
#else
#define nvgCreateFontAtIndex_enter( ctx, name, filename, fontIndex)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgCreateFontAtIndex_leave(NVGcontext* ctx, const char* name, const char* filename, const int fontIndex, int return_value)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgCreateFontAtIndex_leave( fp, ctx->traceName, name, filename, fontIndex, return_value);
}
#else
#define nvgCreateFontAtIndex_leave( ctx, name, filename, fontIndex, return_value)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgCreateFontMem_enter(NVGcontext* ctx, const char* name, unsigned char* data, int ndata, int freeData)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgCreateFontMem_enter( fp, ctx->traceName, name, data, ndata, freeData);
}
#else
#define nvgCreateFontMem_enter( ctx, name, data, ndata, freeData)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgCreateFontMem_leave(NVGcontext* ctx, const char* name, unsigned char* data, int ndata, int freeData, int return_value)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgCreateFontMem_leave( fp, ctx->traceName, name, data, ndata, freeData, return_value);
}
#else
#define nvgCreateFontMem_leave( ctx, name, data, ndata, freeData, return_value)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgCreateFontMemAtIndex_enter(NVGcontext* ctx, const char* name, unsigned char* data, int ndata, int freeData, const int fontIndex)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgCreateFontMemAtIndex_enter( fp, ctx->traceName, name, data, ndata, freeData, fontIndex);
}
#else
#define nvgCreateFontMemAtIndex_enter( ctx, name, data, ndata, freeData, fontIndex)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgCreateFontMemAtIndex_leave(NVGcontext* ctx, const char* name, unsigned char* data, int ndata, int freeData, const int fontIndex, int return_value)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgCreateFontMemAtIndex_leave( fp, ctx->traceName, name, data, ndata, freeData, fontIndex, return_value);
}
#else
#define nvgCreateFontMemAtIndex_leave( ctx, name, data, ndata, freeData, fontIndex, return_value)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgFindFont_enter(NVGcontext* ctx, const char* name)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgFindFont_enter( fp, ctx->traceName, name);
}
#else
#define nvgFindFont_enter( ctx, name)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgFindFont_leave(NVGcontext* ctx, const char* name, int return_value)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgFindFont_leave( fp, ctx->traceName, name, return_value);
}
#else
#define nvgFindFont_leave( ctx, name, return_value)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgAddFallbackFontId_enter(NVGcontext* ctx, int baseFont, int fallbackFont)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgAddFallbackFontId_enter( fp, ctx->traceName, baseFont, fallbackFont);
}
#else
#define nvgAddFallbackFontId_enter( ctx, baseFont, fallbackFont)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgAddFallbackFontId_leave(NVGcontext* ctx, int baseFont, int fallbackFont, int return_value)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgAddFallbackFontId_leave( fp, ctx->traceName, baseFont, fallbackFont, return_value);
}
#else
#define nvgAddFallbackFontId_leave( ctx, baseFont, fallbackFont, return_value)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgAddFallbackFont_enter(NVGcontext* ctx, const char* baseFont, const char* fallbackFont)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgAddFallbackFont_enter( fp, ctx->traceName, baseFont, fallbackFont);
}
#else
#define nvgAddFallbackFont_enter( ctx, baseFont, fallbackFont)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgAddFallbackFont_leave(NVGcontext* ctx, const char* baseFont, const char* fallbackFont, int return_value)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgAddFallbackFont_leave( fp, ctx->traceName, baseFont, fallbackFont, return_value);
}
#else
#define nvgAddFallbackFont_leave( ctx, baseFont, fallbackFont, return_value)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgGetFontHandle_enter(NVGcontext* ctx, int fontId)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgGetFontHandle_enter( fp, ctx->traceName, fontId);
}
#else
#define nvgGetFontHandle_enter( ctx, fontId)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgGetFontHandle_leave(NVGcontext* ctx, int fontId, void* return_value)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgGetFontHandle_leave( fp, ctx->traceName, fontId, return_value);
}
#else
#define nvgGetFontHandle_leave( ctx, fontId, return_value)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgTextBounds_enter(NVGcontext* ctx, float x, float y, const char* string, const char* end, float* bounds)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgTextBounds_enter( fp, ctx->traceName, x, y, string, end, bounds);
}
#else
#define nvgTextBounds_enter( ctx, x, y, string, end, bounds)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgTextBounds_leave(NVGcontext* ctx, float x, float y, const char* string, const char* end, float* bounds, float return_value)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgTextBounds_leave( fp, ctx->traceName, x, y, string, end, bounds, return_value);
}
#else
#define nvgTextBounds_leave( ctx, x, y, string, end, bounds, return_value)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgTextVisualBounds_enter(NVGcontext* ctx, float x, float y, const char* string, const char* end, float bounds[6])
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgTextVisualBounds_enter( fp, ctx->traceName, x, y, string, end, bounds);
}
#else
#define nvgTextVisualBounds_enter( ctx, x, y, string, end, bounds)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgTextVisualBounds_leave(NVGcontext* ctx, float x, float y, const char* string, const char* end, float bounds[6])
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgTextVisualBounds_leave( fp, ctx->traceName, x, y, string, end, bounds);
}
#else
#define nvgTextVisualBounds_leave( ctx, x, y, string, end, bounds)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgTextBoxBounds_enter(NVGcontext* ctx, float x, float y, float breakRowWidth, const char* string, const char* end, float* bounds)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgTextBoxBounds_enter( fp, ctx->traceName, x, y, breakRowWidth, string, end, bounds);
}
#else
#define nvgTextBoxBounds_enter( ctx, x, y, breakRowWidth, string, end, bounds)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgTextBoxBounds_leave(NVGcontext* ctx, float x, float y, float breakRowWidth, const char* string, const char* end, float* bounds)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgTextBoxBounds_leave( fp, ctx->traceName, x, y, breakRowWidth, string, end, bounds);
}
#else
#define nvgTextBoxBounds_leave( ctx, x, y, breakRowWidth, string, end, bounds)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgTextGlyphPositions_enter(NVGcontext* ctx, float x, float y, const char* string, const char* end, NVGglyphPosition* positions, int maxPositions)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgTextGlyphPositions_enter( fp, ctx->traceName, x, y, string, end, positions, maxPositions);
}
#else
#define nvgTextGlyphPositions_enter( ctx, x, y, string, end, positions, maxPositions)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgTextGlyphPositions_leave(NVGcontext* ctx, float x, float y, const char* string, const char* end, NVGglyphPosition* positions, int maxPositions, int return_value)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgTextGlyphPositions_leave( fp, ctx->traceName, x, y, string, end, positions, maxPositions, return_value);
}
#else
#define nvgTextGlyphPositions_leave( ctx, x, y, string, end, positions, maxPositions, return_value)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgTextMetrics_enter(NVGcontext* ctx, float* ascender, float* descender, float* lineh)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgTextMetrics_enter( fp, ctx->traceName, ascender, descender, lineh);
}
#else
#define nvgTextMetrics_enter( ctx, ascender, descender, lineh)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgTextMetrics_leave(NVGcontext* ctx, float* ascender, float* descender, float* lineh)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgTextMetrics_leave( fp, ctx->traceName, ascender, descender, lineh);
}
#else
#define nvgTextMetrics_leave( ctx, ascender, descender, lineh)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgTextVisualMetrics_enter(NVGcontext* ctx, float* ascender, float* descender, float* lineh)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgTextVisualMetrics_enter( fp, ctx->traceName, ascender, descender, lineh);
}
#else
#define nvgTextVisualMetrics_enter( ctx, ascender, descender, lineh)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgTextVisualMetrics_leave(NVGcontext* ctx, float* ascender, float* descender, float* lineh)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgTextVisualMetrics_leave( fp, ctx->traceName, ascender, descender, lineh);
}
#else
#define nvgTextVisualMetrics_leave( ctx, ascender, descender, lineh)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgTextBreakLines_enter(NVGcontext* ctx, const char* string, const char* end, float breakRowWidth, NVGtextRow* rows, int maxRows)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgTextBreakLines_enter( fp, ctx->traceName, string, end, breakRowWidth, rows, maxRows);
}
#else
#define nvgTextBreakLines_enter( ctx, string, end, breakRowWidth, rows, maxRows)
#endif

#ifndef NVG_TRACECODE_DISABLED
static inline void nvgTextBreakLines_leave(NVGcontext* ctx, const char* string, const char* end, float breakRowWidth, NVGtextRow* rows, int maxRows, int return_value)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return; 
   _nvgTextBreakLines_leave( fp, ctx->traceName, string, end, breakRowWidth, rows, maxRows, return_value);
}
#else
#define nvgTextBreakLines_leave( ctx, string, end, breakRowWidth, rows, maxRows, return_value)
#endif

// Tracing Utilities
#ifndef NVG_TRACECODE_DISABLED
static inline void nvgTraceComment_enter(NVGcontext* ctx, const char *text)
{
   FILE   *fp;

   if( ! ctx)
      return;
   fp = ctx->trace;
   if( ! fp)
      return;
   _nvgTraceComment_enter( fp, ctx->traceName, text);
}
#else
#define nvgTraceComment_enter( ctx, text)
#endif

#endif // NANOVG_TRACE_SUPPORT_H