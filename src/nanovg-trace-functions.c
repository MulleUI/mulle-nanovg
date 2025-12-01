#include "nanovg-trace-functions.h"

#include <string.h>

// Enum printing functions
//static void   nvgContextTypePrint(FILE *fp, int value)
//{
//   switch( value)
//   {   case NVG_FULL_CONTEXT:
//      fprintf(fp, "NVG_FULL_CONTEXT");
//      break;
//   case NVG_PATH_CONTEXT:
//      fprintf(fp, "NVG_PATH_CONTEXT");
//      break;
//   case NVG_FONT_CONTEXT:
//      fprintf(fp, "NVG_FONT_CONTEXT");
//      break;
//   default:
//      fprintf(fp, "%d", value);
//      break;
//   }
//}


static void   nvgWindingPrint(FILE *fp, int value)
{
   switch( value)
   {   
   case NVG_CCW:
      fprintf(fp, "NVG_CCW");
      break;
   case NVG_CW:
      fprintf(fp, "NVG_CW");
      break;
   default:
      fprintf(fp, "%d", value);
      break;
   }
}


static void   nvgSolidityPrint(FILE *fp, int value)
{
   switch( value)
   {   
   case NVG_SOLID:
      fprintf(fp, "NVG_SOLID");
      break;
   case NVG_HOLE:
      fprintf(fp, "NVG_HOLE");
      break;
   default:
      fprintf(fp, "%d", value);
      break;
   }
}


static void   nvgLineCapPrint(FILE *fp, int value)
{
   switch( value)
   {   
   case NVG_BUTT:
      fprintf(fp, "NVG_BUTT");
      break;
   case NVG_ROUND:
      fprintf(fp, "NVG_ROUND");
      break;
   case NVG_SQUARE:
      fprintf(fp, "NVG_SQUARE");
      break;
   case NVG_BEVEL:
      fprintf(fp, "NVG_BEVEL");
      break;
   case NVG_MITER:
      fprintf(fp, "NVG_MITER");
      break;
   default:
      fprintf(fp, "%d", value);
      break;
   }
}


static void   nvgAlignPrint(FILE *fp, int value)
{   
   // Since it's bitfield, handle combinations
   int first = 1;
   if( value & NVG_ALIGN_LEFT)
   {      if( !first) fprintf(fp, "|");
      fprintf(fp, "NVG_ALIGN_LEFT");
      first = 0;
   }
   if( value & NVG_ALIGN_CENTER)
   {      if( !first) fprintf(fp, "|");
      fprintf(fp, "NVG_ALIGN_CENTER");
      first = 0;
   }
   if( value & NVG_ALIGN_RIGHT)
   {      if( !first) fprintf(fp, "|");
      fprintf(fp, "NVG_ALIGN_RIGHT");
      first = 0;
   }
   if( value & NVG_ALIGN_TOP)
   {      if( !first) fprintf(fp, "|");
      fprintf(fp, "NVG_ALIGN_TOP");
      first = 0;
   }
   if( value & NVG_ALIGN_MIDDLE)
   {      if( !first) fprintf(fp, "|");
      fprintf(fp, "NVG_ALIGN_MIDDLE");
      first = 0;
   }
   if( value & NVG_ALIGN_BOTTOM)
   {      if( !first) fprintf(fp, "|");
      fprintf(fp, "NVG_ALIGN_BOTTOM");
      first = 0;
   }
   if( value & NVG_ALIGN_BASELINE)
   {      if( !first) fprintf(fp, "|");
      fprintf(fp, "NVG_ALIGN_BASELINE");
      first = 0;
   }
   if( first)
   {      fprintf(fp, "%d", value);
   }
}

static void   nvgBlendFactorPrint(FILE *fp, int value)
{   switch( value)
   {   case NVG_ZERO:
      fprintf(fp, "NVG_ZERO");
      break;
   case NVG_ONE:
      fprintf(fp, "NVG_ONE");
      break;
   case NVG_SRC_COLOR:
      fprintf(fp, "NVG_SRC_COLOR");
      break;
   case NVG_ONE_MINUS_SRC_COLOR:
      fprintf(fp, "NVG_ONE_MINUS_SRC_COLOR");
      break;
   case NVG_DST_COLOR:
      fprintf(fp, "NVG_DST_COLOR");
      break;
   case NVG_ONE_MINUS_DST_COLOR:
      fprintf(fp, "NVG_ONE_MINUS_DST_COLOR");
      break;
   case NVG_SRC_ALPHA:
      fprintf(fp, "NVG_SRC_ALPHA");
      break;
   case NVG_ONE_MINUS_SRC_ALPHA:
      fprintf(fp, "NVG_ONE_MINUS_SRC_ALPHA");
      break;
   case NVG_DST_ALPHA:
      fprintf(fp, "NVG_DST_ALPHA");
      break;
   case NVG_ONE_MINUS_DST_ALPHA:
      fprintf(fp, "NVG_ONE_MINUS_DST_ALPHA");
      break;
   case NVG_SRC_ALPHA_SATURATE:
      fprintf(fp, "NVG_SRC_ALPHA_SATURATE");
      break;
   default:
      fprintf(fp, "%d", value);
      break;
   }
}

static void   nvgCompositeOperationPrint(FILE *fp, int value)
{   switch( value)
   {   case NVG_SOURCE_OVER:
      fprintf(fp, "NVG_SOURCE_OVER");
      break;
   case NVG_SOURCE_IN:
      fprintf(fp, "NVG_SOURCE_IN");
      break;
   case NVG_SOURCE_OUT:
      fprintf(fp, "NVG_SOURCE_OUT");
      break;
   case NVG_ATOP:
      fprintf(fp, "NVG_ATOP");
      break;
   case NVG_DESTINATION_OVER:
      fprintf(fp, "NVG_DESTINATION_OVER");
      break;
   case NVG_DESTINATION_IN:
      fprintf(fp, "NVG_DESTINATION_IN");
      break;
   case NVG_DESTINATION_OUT:
      fprintf(fp, "NVG_DESTINATION_OUT");
      break;
   case NVG_DESTINATION_ATOP:
      fprintf(fp, "NVG_DESTINATION_ATOP");
      break;
   case NVG_LIGHTER:
      fprintf(fp, "NVG_LIGHTER");
      break;
   case NVG_COPY:
      fprintf(fp, "NVG_COPY");
      break;
   case NVG_XOR:
      fprintf(fp, "NVG_XOR");
      break;
   default:
      fprintf(fp, "%d", value);
      break;
   }
}

static void nvgImageFlagsPrint(FILE *fp, int value)
{   // Bitfield
   int first = 1;
   if( value & NVG_IMAGE_GENERATE_MIPMAPS)
   {      if( !first) fprintf(fp, "|");
      fprintf(fp, "NVG_IMAGE_GENERATE_MIPMAPS");
      first = 0;
   }
   if( value & NVG_IMAGE_REPEATX)
   {      if( !first) fprintf(fp, "|");
      fprintf(fp, "NVG_IMAGE_REPEATX");
      first = 0;
   }
   if( value & NVG_IMAGE_REPEATY)
   {      if( !first) fprintf(fp, "|");
      fprintf(fp, "NVG_IMAGE_REPEATY");
      first = 0;
   }
   if( value & NVG_IMAGE_FLIPY)
   {      if( !first) fprintf(fp, "|");
      fprintf(fp, "NVG_IMAGE_FLIPY");
      first = 0;
   }
   if( value & NVG_IMAGE_PREMULTIPLIED)
   {      if( !first) fprintf(fp, "|");
      fprintf(fp, "NVG_IMAGE_PREMULTIPLIED");
      first = 0;
   }
   if( value & NVG_IMAGE_NEAREST)
   {      if( !first) fprintf(fp, "|");
      fprintf(fp, "NVG_IMAGE_NEAREST");
      first = 0;
   }
   if( value & NVG_IMAGE_MSAA)
   {      if( !first) fprintf(fp, "|");
      fprintf(fp, "NVG_IMAGE_MSAA");
      first = 0;
   }
   if( value & NVG_IMAGE_RBO_LESS)
   {      if( !first) fprintf(fp, "|");
      fprintf(fp, "NVG_IMAGE_RBO_LESS");
      first = 0;
   }
   if( first)
   {      fprintf(fp, "%d", value);
   }
}

static void   nvgTessPrint(FILE *fp, int value)
{   switch( value)
   {   case NVG_TESS_SUBDIVISION:
      fprintf(fp, "NVG_TESS_SUBDIVISION");
      break;
   case NVG_TESS_AFD:
      fprintf(fp, "NVG_TESS_AFD");
      break;
   default:
      fprintf(fp, "%d", value);
      break;
   }
}

static void   nvgTexturePrint(FILE *fp, int value)
{   switch( value)
   {   case NVG_TEXTURE_ALPHA:
      fprintf(fp, "NVG_TEXTURE_ALPHA");
      break;
   case NVG_TEXTURE_RGBA:
      fprintf(fp, "NVG_TEXTURE_RGBA");
      break;
   default:
      fprintf(fp, "%d", value);
      break;
   }
}


static void    NVGpaintPrint(FILE *fp, struct NVGpaint *paint)
{
   char   *sep;

   fprintf( fp, "(struct NVGpaint) {");
   sep = " ";

   if( paint->xform[ 0] != 0.0 || paint->xform[ 1] != 0.0 || paint->xform[ 2] != 0.0 ||
       paint->xform[ 3] != 0.0 || paint->xform[ 4] != 0.0 || paint->xform[ 5] != 0.0)
   {
      fprintf( fp, "%s.xform={ %g,%g,%g,%g,%g,%g}",
                    sep,
                    paint->xform[0], paint->xform[1], paint->xform[2],
                    paint->xform[3], paint->xform[4], paint->xform[5]);
      sep = ", ";
   }

   if( paint->extent[ 0] != 0.0 || paint->extent[ 1] != 0.0)
   { 
      fprintf( fp, "%s.extent={%g,%g}", 
                       sep, paint->extent[0], paint->extent[1]);
      sep = ", ";
   }

   if( paint->radius != 0)
   {
      fprintf( fp, "%s.radius=%g", sep, paint->radius);
      sep = ", ";
   }

   if( paint->feather != 0)
   {
      fprintf( fp, "%s.feather=%g", sep, paint->feather);
      sep = ", ";
   }

   if( paint->innerColor.r != 0.0 ||
       paint->innerColor.g != 0.0 ||
       paint->innerColor.b != 0.0 ||
       paint->innerColor.a)
   {
      fprintf( fp, "%s.innerColor=CGColorDataWithRGBA( %g,%g,%g,%g)",
                     sep, 
                     paint->innerColor.r,
                     paint->innerColor.g,
                     paint->innerColor.b,
                     paint->innerColor.a);
      sep = ", ";
   } 

   if( paint->outerColor.r != 0.0 ||
       paint->outerColor.g != 0.0 ||
       paint->outerColor.b != 0.0 ||
       paint->outerColor.a)
   {
      fprintf( fp, "%s.outerColor=CGColorDataWithRGBA( %g,%g,%g,%g)",
                     sep, 
                     paint->outerColor.r,
                     paint->outerColor.g,
                     paint->outerColor.b,
                     paint->outerColor.a);
      sep = ", ";
   } 

   if( paint->image != 0)
   {
      fprintf( fp, "%s.image=%d", sep, paint->image);
      sep = ", ";
   }

   if( sep[ 0] != ',')
      fprintf( fp, " 0");

   fprintf( fp, "}");
}


// Tracing function implementations (empty for now)
void _nvgBeginFrame_enter( FILE *fp, char *ctxName, float windowWidth, float windowHeight, float devicePixelRatio)
{
   fprintf(fp, "nvgBeginFrame( %s, %f, %f, %f)\n", ctxName ? ctxName : "vg", windowWidth, windowHeight, devicePixelRatio);
}

void _nvgCancelFrame_enter(FILE *fp, char *ctxName)
{
   fprintf(fp, "nvgCancelFrame( %s)\n", ctxName ? ctxName : "vg");
}

void _nvgChangeFrame_enter( FILE *fp, char *ctxName, float windowWidth, float windowHeight, float devicePixelRatio)
{
   fprintf(fp, "nvgChangeFrame( %s, %f, %f, %f)\n", ctxName ? ctxName : "vg", windowWidth, windowHeight, devicePixelRatio);
}

void _nvgFlushFrame_enter(FILE *fp, char *ctxName)
{
   fprintf(fp, "nvgFlushFrame( %s)\n", ctxName ? ctxName : "vg");
}

void _nvgEndFrame_enter(FILE *fp, char *ctxName)
{
   fprintf(fp, "nvgEndFrame( %s)\n", ctxName ? ctxName : "vg");
}

void _nvgGlobalCompositeOperation_enter( FILE *fp, char *ctxName, int op)
{
   fprintf(fp, "nvgGlobalCompositeOperation( %s, ", ctxName ? ctxName : "vg");
   nvgCompositeOperationPrint(fp, op);
   fprintf(fp, ")\n");
}

void _nvgGlobalCompositeBlendFunc_enter( FILE *fp, char *ctxName, int sfactor, int dfactor)
{
   fprintf(fp, "nvgGlobalCompositeBlendFunc( %s, ", ctxName ? ctxName : "vg");
   nvgBlendFactorPrint(fp, sfactor);
   fprintf(fp, ", ");
   nvgBlendFactorPrint(fp, dfactor);
   fprintf(fp, ")\n");
}

void _nvgGlobalCompositeBlendFuncSeparate_enter( FILE *fp, char *ctxName, int srcRGB, int dstRGB, int srcAlpha, int dstAlpha)
{
   fprintf(fp, "nvgGlobalCompositeBlendFuncSeparate( %s, ", ctxName ? ctxName : "vg");
   nvgBlendFactorPrint(fp, srcRGB);
   fprintf(fp, ", ");
   nvgBlendFactorPrint(fp, dstRGB);
   fprintf(fp, ", ");
   nvgBlendFactorPrint(fp, srcAlpha);
   fprintf(fp, ", ");
   nvgBlendFactorPrint(fp, dstAlpha);
   fprintf(fp, ")\n");
}

void _nvgSave_enter(FILE *fp, char *ctxName)
{
   fprintf(fp, "nvgSave( %s)\n", ctxName ? ctxName : "vg");
}

void _nvgRestore_enter(FILE *fp, char *ctxName)
{
   fprintf(fp, "nvgRestore( %s)\n", ctxName ? ctxName : "vg");
}

void _nvgReset_enter(FILE *fp, char *ctxName)
{
   fprintf(fp, "nvgReset( %s)\n", ctxName ? ctxName : "vg");
}

void _nvgShapeAntiAlias_enter( FILE *fp, char *ctxName, int enabled)
{
   fprintf(fp, "nvgShapeAntiAlias( %s, %s)\n", ctxName ? ctxName : "vg", enabled ? "YES" : "NO");
}

void _nvgStrokeColor_enter( FILE *fp, char *ctxName, NVGcolor color)
{
   fprintf(fp, "nvgStrokeColor( %s, {r:%f, g:%f, b:%f, a:%f})\n", ctxName ? ctxName : "vg", color.r, color.g, color.b, color.a);
}

void _nvgStrokePaint_enter( FILE *fp, char *ctxName, NVGpaint paint)
{
   fprintf(fp, "nvgStrokePaint( %s, ", ctxName ? ctxName : "vg");
   NVGpaintPrint( fp, &paint);
   fprintf( fp, ")\n");
}

void _nvgFillColor_enter( FILE *fp, char *ctxName, NVGcolor color)
{
   fprintf(fp, "nvgFillColor( %s, {r:%f, g:%f, b:%f, a:%f})\n", ctxName ? ctxName : "vg", color.r, color.g, color.b, color.a);
}

void _nvgClearType_enter( FILE *fp, char *ctxName, int enabled)
{
   fprintf(fp, "nvgClearType( %s, %s)\n", ctxName ? ctxName : "vg", enabled ? "YES" : "NO");
}

void _nvgTextColor_enter( FILE *fp, char *ctxName, NVGcolor color)
{
   fprintf(fp, "nvgTextColor( %s, {r:%f, g:%f, b:%f, a:%f})\n", ctxName ? ctxName : "vg", color.r, color.g, color.b, color.a);
}

void _nvgTextBackgroundColor_enter( FILE *fp, char *ctxName, NVGcolor color)
{
   fprintf(fp, "nvgTextBackgroundColor( %s, CGColorMake( %f, %f, %f, %f))\n", ctxName ? ctxName : "vg", color.r, color.g, color.b, color.a);
}

void _nvgFillPaint_enter( FILE *fp, char *ctxName, NVGpaint paint)
{
   fprintf(fp, "nvgFillPaint( %s, ", ctxName ? ctxName : "vg");
   NVGpaintPrint( fp, &paint);
   fprintf(fp, ")\n");
}

void _nvgMiterLimit_enter( FILE *fp, char *ctxName, float limit)
{
   fprintf(fp, "nvgMiterLimit( %s, %f)\n", ctxName ? ctxName : "vg", limit);
}

void _nvgStrokeWidth_enter( FILE *fp, char *ctxName, float size)
{
   fprintf(fp, "nvgStrokeWidth( %s, %f)\n", ctxName ? ctxName : "vg", size);
}

void _nvgLineCap_enter( FILE *fp, char *ctxName, int cap)
{
   fprintf(fp, "nvgLineCap( %s, ", ctxName ? ctxName : "vg");
   nvgLineCapPrint(fp, cap);
   fprintf(fp, ")\n");
}

void _nvgLineJoin_enter( FILE *fp, char *ctxName, int join)
{
   fprintf(fp, "nvgLineJoin( %s, ", ctxName ? ctxName : "vg");
   // Note: LineJoin uses the same enum values as LineCap, so we can reuse the function
   nvgLineCapPrint(fp, join);
   fprintf(fp, ")\n");
}

void _nvgGlobalAlpha_enter( FILE *fp, char *ctxName, float alpha)
{
   fprintf(fp, "nvgGlobalAlpha( %s, %f)\n", ctxName ? ctxName : "vg", alpha);
}

void _nvgBezierTessellation_enter( FILE *fp, char *ctxName, int tess)
{
   fprintf(fp, "nvgBezierTessellation( %s, ", ctxName ? ctxName : "vg");
   nvgTessPrint(fp, tess);
   fprintf(fp, ")\n");
}

void _nvgResetTransform_enter(FILE *fp, char *ctxName)
{
   fprintf(fp, "nvgResetTransform( %s)\n", ctxName ? ctxName : "vg");
}

void _nvgTransform_enter( FILE *fp, char *ctxName, float a, float b, float c, float d, float e, float f)
{
   fprintf(fp, "nvgTransform( %s, %f, %f, %f, %f, %f, %f)\n", ctxName ? ctxName : "vg", a, b, c, d, e, f);
}

void _nvgTranslate_enter( FILE *fp, char *ctxName, float x, float y)
{
   fprintf(fp, "nvgTranslate( %s, %f, %f)\n", ctxName ? ctxName : "vg", x, y);
}

void _nvgRotate_enter( FILE *fp, char *ctxName, float angle)
{
   fprintf(fp, "nvgRotate( %s, %f)\n", ctxName ? ctxName : "vg", angle);
}

void _nvgSkewX_enter( FILE *fp, char *ctxName, float angle)
{
   fprintf(fp, "nvgSkewX( %s, %f)\n", ctxName ? ctxName : "vg", angle);
}

void _nvgSkewY_enter( FILE *fp, char *ctxName, float angle)
{
   fprintf(fp, "nvgSkewY( %s, %f)\n", ctxName ? ctxName : "vg", angle);
}

void _nvgScale_enter( FILE *fp, char *ctxName, float x, float y)
{
   fprintf(fp, "nvgScale( %s, %f, %f)\n", ctxName ? ctxName : "vg", x, y);
}

void _nvgCurrentTransform_enter( FILE *fp, char *ctxName, float* xform)
{
   fprintf(fp, "nvgCurrentTransform( %s, %p)\n", ctxName ? ctxName : "vg", xform);
}

void _nvgCurrentTransform_leave(FILE *fp, char *ctxName, float* xform)
{
   fprintf(fp, "nvgCurrentTransform <- (float[ 6]) { %f, %f, %f, %f, %f, %f }\n",
    xform[ 0], xform[ 1], xform[ 2], xform[ 3], xform[ 4], xform[ 5]);
}

void _nvgUpdateImage_enter( FILE *fp, char *ctxName, int image, const unsigned char* data)
{
   fprintf(fp, "nvgUpdateImage( %s, %d, %p)\n", ctxName ? ctxName : "vg", image, data);
}


void _nvgDeleteImage_enter( FILE *fp, char *ctxName, int image)
{
   fprintf(fp, "nvgDeleteImage( %s, %d)\n", ctxName ? ctxName : "vg", image);
}

void _nvgForgetImageTexture_enter( FILE *fp, char *ctxName, int image)
{
   fprintf(fp, "nvgForgetImageTexture( %s, %d)\n", ctxName ? ctxName : "vg", image);
}

void _nvgScissor_enter( FILE *fp, char *ctxName, float x, float y, float w, float h)
{
   fprintf(fp, "nvgScissor( %s, %f, %f, %f, %f)\n", ctxName ? ctxName : "vg", x, y, w, h);
}

void _nvgIntersectScissor_enter( FILE *fp, char *ctxName, float x, float y, float w, float h)
{
   fprintf(fp, "nvgIntersectScissor( %s, %f, %f, %f, %f)\n", ctxName ? ctxName : "vg", x, y, w, h);
}

void _nvgResetScissor_enter(FILE *fp, char *ctxName)
{
   fprintf(fp, "nvgResetScissor( %s)\n", ctxName ? ctxName : "vg");
}

void _nvgBeginPath_enter(FILE *fp, char *ctxName)
{
   fprintf(fp, "nvgBeginPath( %s)\n", ctxName ? ctxName : "vg");
}

void _nvgMoveTo_enter( FILE *fp, char *ctxName, float x, float y)
{
   fprintf(fp, "nvgMoveTo( %s, %f, %f)\n", ctxName ? ctxName : "vg", x, y);
}

void _nvgLineTo_enter( FILE *fp, char *ctxName, float x, float y)
{
   fprintf(fp, "nvgLineTo( %s, %f, %f)\n", ctxName ? ctxName : "vg", x, y);
}

void _nvgBezierTo_enter( FILE *fp, char *ctxName, float c1x, float c1y, float c2x, float c2y, float x, float y)
{
   fprintf(fp, "nvgBezierTo( %s, %f, %f, %f, %f, %f, %f)\n", ctxName ? ctxName : "vg", c1x, c1y, c2x, c2y, x, y);
}

void _nvgQuadTo_enter( FILE *fp, char *ctxName, float cx, float cy, float x, float y)
{
   fprintf(fp, "nvgQuadTo( %s, %f, %f, %f, %f)\n", ctxName ? ctxName : "vg", cx, cy, x, y);
}

void _nvgArcTo_enter( FILE *fp, char *ctxName, float x1, float y1, float x2, float y2, float radius)
{
   fprintf(fp, "nvgArcTo( %s, %f, %f, %f, %f, %f)\n", ctxName ? ctxName : "vg", x1, y1, x2, y2, radius);
}

void _nvgClosePath_enter(FILE *fp, char *ctxName)
{
   fprintf(fp, "nvgClosePath( %s)\n", ctxName ? ctxName : "vg");
}

void _nvgPathWinding_enter( FILE *fp, char *ctxName, int dir)
{
   fprintf(fp, "nvgPathWinding( %s, ", ctxName ? ctxName : "vg");
   nvgSolidityPrint(fp, dir);
   fprintf(fp, ")\n");
}

void _nvgArc_enter( FILE *fp, char *ctxName, float cx, float cy, float r, float a0, float a1, int dir)
{
   fprintf(fp, "nvgArc( %s, %f, %f, %f, %f, %f, ", ctxName ? ctxName : "vg", cx, cy, r, a0, a1);
   nvgWindingPrint(fp, dir);
   fprintf(fp, ")\n");
}

void _nvgRect_enter( FILE *fp, char *ctxName, float x, float y, float w, float h)
{
   fprintf(fp, "nvgRect( %s, %f, %f, %f, %f)\n", ctxName ? ctxName : "vg", x, y, w, h);
}

void _nvgRoundedRect_enter( FILE *fp, char *ctxName, float x, float y, float w, float h, float r)
{
   fprintf(fp, "nvgRoundedRect( %s, %f, %f, %f, %f, %f)\n", ctxName ? ctxName : "vg", x, y, w, h, r);
}

void _nvgRoundedRectVarying_enter( FILE *fp, char *ctxName, float x, float y, float w, float h, float radTopLeft, float radTopRight, float radBottomRight, float radBottomLeft)
{
   fprintf(fp, "nvgRoundedRectVarying( %s, %f, %f, %f, %f, %f, %f, %f, %f)\n", ctxName ? ctxName : "vg", x, y, w, h, radTopLeft, radTopRight, radBottomRight, radBottomLeft);
}

void _nvgEllipse_enter( FILE *fp, char *ctxName, float cx, float cy, float rx, float ry)
{
   fprintf(fp, "nvgEllipse( %s, %f, %f, %f, %f)\n", ctxName ? ctxName : "vg", cx, cy, rx, ry);
}

void _nvgCircle_enter( FILE *fp, char *ctxName, float cx, float cy, float r)
{
   fprintf(fp, "nvgCircle( %s, %f, %f, %f)\n", ctxName ? ctxName : "vg", cx, cy, r);
}

void _nvgFill_enter(FILE *fp, char *ctxName)
{
   fprintf(fp, "nvgFill( %s)\n", ctxName ? ctxName : "vg");
}

void _nvgStroke_enter(FILE *fp, char *ctxName)
{
   fprintf(fp, "nvgStroke( %s)\n", ctxName ? ctxName : "vg");
}

void _nvgPrecompilePath_enter(FILE *fp, char *ctxName)
{
   fprintf(fp, "nvgPrecompilePath( %s)\n", ctxName ? ctxName : "vg");
}

void _nvgCopyPath_enter( FILE *fp, char *ctxName, NVGcontext *path)
{
   fprintf(fp, "nvgCopyPath( %s, %p)\n", ctxName ? ctxName : "vg", path);
}

void _nvgFontSize_enter( FILE *fp, char *ctxName, float size)
{
   fprintf(fp, "nvgFontSize( %s, %f)\n", ctxName ? ctxName : "vg", size);
}

void _nvgFontBlur_enter( FILE *fp, char *ctxName, float blur)
{
   fprintf(fp, "nvgFontBlur( %s, %f)\n", ctxName ? ctxName : "vg", blur);
}

void _nvgTextLetterSpacing_enter( FILE *fp, char *ctxName, float spacing)
{
   fprintf(fp, "nvgTextLetterSpacing( %s, %f)\n", ctxName ? ctxName : "vg", spacing);
}

void _nvgTextLineHeight_enter( FILE *fp, char *ctxName, float lineHeight)
{
   fprintf(fp, "nvgTextLineHeight( %s, %f)\n", ctxName ? ctxName : "vg", lineHeight);
}

void _nvgTextAlign_enter( FILE *fp, char *ctxName, int align)
{
   fprintf(fp, "nvgTextAlign( %s, ", ctxName ? ctxName : "vg");
   nvgAlignPrint(fp, align);
   fprintf(fp, ")\n");
}

void _nvgFontFaceId_enter( FILE *fp, char *ctxName, int font)
{
   fprintf(fp, "nvgFontFaceId( %s, %d)\n", ctxName ? ctxName : "vg", font);
}

void _nvgFontFace_enter( FILE *fp, char *ctxName, const char* font)
{
   fprintf(fp, "nvgFontFace( %s, \"%s\")\n", ctxName ? ctxName : "vg", font ? font : "(null)");
}

void _nvgText_enter( FILE *fp, char *ctxName, float x, float y, const char* string, const char* end)
{
   if (string && end && end > string)
      fprintf(fp, "nvgText( %s, %f, %f, \"%.*s\", +%td)\n", ctxName ? ctxName : "vg", x, y, (int)(end - string), string, end - string);
   else if (end)
      fprintf(fp, "nvgText( %s, %f, %f, \"%s\", +%td)\n", ctxName ? ctxName : "vg", x, y, string ? string : "(null)", end - string);
   else
      fprintf(fp, "nvgText( %s, %f, %f, \"%s\", NULL)\n", ctxName ? ctxName : "vg", x, y, string ? string : "(null)");
}

void _nvgText_leave( FILE *fp, char *ctxName, float x, float y, const char* string, const char* end, float return_value)
{
   fprintf(fp, "nvgText <- %f\n", return_value);
}


void _nvgTextBox_enter( FILE *fp, char *ctxName, float x, float y, float breakRowWidth, const char* string, const char* end)
{
   if (string && end && end > string)
      fprintf(fp, "nvgTextBox( %s, %f, %f, %f, \"%.*s\", +%td)\n", ctxName ? ctxName : "vg", x, y, breakRowWidth, (int)(end - string), string, end - string);
   else if (end)
      fprintf(fp, "nvgTextBox( %s, %f, %f, %f, \"%s\", +%td)\n", ctxName ? ctxName : "vg", x, y, breakRowWidth, string ? string : "(null)", end - string);
   else
      fprintf(fp, "nvgTextBox( %s, %f, %f, %f, \"%s\", NULL)\n", ctxName ? ctxName : "vg", x, y, breakRowWidth, string ? string : "(null)");
}

void _nvgResetFallbackFontsId_enter( FILE *fp, char *ctxName, int baseFont)
{
   fprintf(fp, "nvgResetFallbackFontsId( %s, %d)\n", ctxName ? ctxName : "vg", baseFont);
}

void _nvgResetFallbackFonts_enter( FILE *fp, char *ctxName, const char* baseFont)
{
   fprintf(fp, "nvgResetFallbackFonts( %s, \"%s\")\n", ctxName ? ctxName : "vg", baseFont ? baseFont : "(null)");
}

void _nvgSetScissor_enter( FILE *fp, char *ctxName, NVGscissor *scissor)
{
   fprintf(fp, "nvgSetScissor( %s, %p)\n", ctxName ? ctxName : "vg", scissor);
}


void _nvgCreateImage_enter( FILE *fp, char *ctxName, const char* filename, int imageFlags)
{
   fprintf(fp, "nvgCreateImage( %s, \"%s\", ", ctxName ? ctxName : "vg", filename ? filename : "(null)");
   nvgImageFlagsPrint(fp, imageFlags);
   fprintf(fp, ")\n");
}

void _nvgCreateImage_leave( FILE *fp, char *ctxName, const char* filename, int imageFlags, int return_value)
{
   fprintf(fp, "nvgCreateImage <- %d\n", return_value);
}

void _nvgCreateImageMem_enter( FILE *fp, char *ctxName, int imageFlags, unsigned char* data, int ndata)
{
   fprintf(fp, "nvgCreateImageMem( %s, ", ctxName ? ctxName : "vg");
   nvgImageFlagsPrint(fp, imageFlags);
   fprintf(fp, ", %p, %d )\n", data, ndata);
}

void _nvgCreateImageMem_leave( FILE *fp, char *ctxName, int imageFlags, unsigned char* data, int ndata, int return_value)
{
   fprintf(fp, "nvgCreateImageMem <- %d\n", return_value);
}

void _nvgCreateImageRGBA_enter( FILE *fp, char *ctxName, int w, int h, int imageFlags, const unsigned char* data)
{
   fprintf(fp, "nvgCreateImageRGBA( %s, %d, %d, ", ctxName ? ctxName : "vg", w, h);
   nvgImageFlagsPrint(fp, imageFlags);
   fprintf(fp, ", %p )\n", data);
}

void _nvgCreateImageRGBA_leave( FILE *fp, char *ctxName, int w, int h, int imageFlags, const unsigned char* data, int return_value)
{
   fprintf(fp, "nvgCreateImageRGBA <- %d\n", return_value);
}

void _nvgCreateImageTexture_enter( FILE *fp, char *ctxName, int w, int h, int imageFlags, int type, void **texture)
{
   fprintf(fp, "nvgCreateImageTexture( %s, %d, %d, ", ctxName ? ctxName : "vg", w, h);
   nvgImageFlagsPrint(fp, imageFlags);
   fprintf(fp, ", ");
   nvgTexturePrint(fp, type);
   fprintf(fp, ", %p )\n", texture);
}

void _nvgCreateImageTexture_leave( FILE *fp, char *ctxName, int w, int h, int imageFlags, int type, void **texture, int return_value)
{
   fprintf(fp, "nvgCreateImageTexture <- %d\n", return_value);
}

void _nvgImageTextureInfo_enter( FILE *fp, char *ctxName, int image, int *imageFlags, int *type, void **texture)
{
   fprintf(fp, "nvgImageTextureInfo( %s, %d, %p, %p, %p)\n", ctxName ? ctxName : "vg", image, imageFlags, type, texture);
}

void _nvgImageSize_enter( FILE *fp, char *ctxName, int image, int* w, int* h)
{
   fprintf(fp, "nvgImageSize( %s, %d, %p, %p)", ctxName ? ctxName : "vg", image, w, h);
}


void _nvgImageSize_leave( FILE *fp, char *ctxName, int image, int* w, int* h)
{
   fprintf(fp, "nvgImageSize <- { ");
   if( w)
      fprintf( fp, "w: %d", *w);
   if( h)
   {
      if( w)
         fprintf( fp, ", ");
      fprintf( fp, "h: %d", *h);
   }
   fprintf( fp, "}\n");
}


void _nvgImageTextureInfo_leave( FILE *fp, char *ctxName, int image, int *imageFlags, int *type, void **texture)
{
   fprintf(fp, "nvgImageTextureInfo completed\n");
}

void _nvgLinearGradient_enter( FILE *fp, char *ctxName, float sx, float sy, float ex, float ey, NVGcolor icol, NVGcolor ocol)
{
   fprintf(fp, "nvgLinearGradient( %s, %f, %f, %f, %f, CGColorMake( %.3f,%.3f,%.3f,%.3f), CGColorMake( %.3f,%.3f,%.3f,%.3f))\n",
           ctxName ? ctxName : "vg", sx, sy, ex, ey,
           icol.r, icol.g, icol.b, icol.a,
           ocol.r, ocol.g, ocol.b, ocol.a);
}

void _nvgLinearGradient_leave( FILE *fp, char *ctxName, float sx, float sy, float ex, float ey, NVGcolor icol, NVGcolor ocol, NVGpaint return_value)
{
   fprintf(fp, "nvgLinearGradient <- ");
   NVGpaintPrint( fp, &return_value);
   fprintf( fp, "\n");
}

void _nvgBoxGradient_enter( FILE *fp, char *ctxName, float x, float y, float w, float h, float r, float f, NVGcolor icol, NVGcolor ocol)
{
   fprintf(fp, "nvgBoxGradient( %s, %f, %f, %f, %f, %f, %f, {%.3f,%.3f,%.3f,%.3f}, {%.3f,%.3f,%.3f,%.3f} )\n",
           ctxName ? ctxName : "vg", x, y, w, h, r, f,
           icol.r, icol.g, icol.b, icol.a,
           ocol.r, ocol.g, ocol.b, ocol.a);
}

void _nvgBoxGradient_leave( FILE *fp, char *ctxName, float x, float y, float w, float h, float r, float f, NVGcolor icol, NVGcolor ocol, NVGpaint return_value)
{
   fprintf(fp, "nvgBoxGradient <- ");
   NVGpaintPrint( fp, &return_value);
   fprintf( fp, "\n");   
}

void _nvgRadialGradient_enter( FILE *fp, char *ctxName, float cx, float cy, float inr, float outr, NVGcolor icol, NVGcolor ocol)
{
   fprintf(fp, "nvgRadialGradient( %s, %f, %f, %f, %f, {%.3f,%.3f,%.3f,%.3f}, {%.3f,%.3f,%.3f,%.3f} )\n",
           ctxName ? ctxName : "vg", cx, cy, inr, outr,
           icol.r, icol.g, icol.b, icol.a,
           ocol.r, ocol.g, ocol.b, ocol.a);
}

void _nvgRadialGradient_leave( FILE *fp, char *ctxName, float cx, float cy, float inr, float outr, NVGcolor icol, NVGcolor ocol, NVGpaint return_value)
{
   fprintf(fp, "nvgRadialGradient <- ");
   NVGpaintPrint( fp, &return_value);
   fprintf( fp, "\n");   
}

void _nvgImagePattern_enter( FILE *fp, char *ctxName, float ox, float oy, float ex, float ey, float angle, int image, float alpha)
{
   fprintf(fp, "nvgImagePattern( %s, %f, %f, %f, %f, %f, %d, %f )\n",
           ctxName ? ctxName : "vg", ox, oy, ex, ey, angle, image, alpha);
}

void _nvgImagePattern_leave( FILE *fp, char *ctxName, float ox, float oy, float ex, float ey, float angle, int image, float alpha, NVGpaint return_value)
{
   fprintf(fp, "nvgImagePattern <- ");
   NVGpaintPrint( fp, &return_value);
   fprintf( fp, "\n");    
}

void _nvgCreateFont_enter( FILE *fp, char *ctxName, const char* name, const char* filename)
{
   fprintf(fp, "nvgCreateFont( %s, \"%s\", \"%s\")\n", ctxName ? ctxName : "vg", name ? name : "(null)", filename ? filename : "(null)");
}

void _nvgCreateFont_leave( FILE *fp, char *ctxName, const char* name, const char* filename, int return_value)
{
   fprintf(fp, "nvgCreateFont <- %d\n", return_value);
}

void _nvgCreateFontAtIndex_enter( FILE *fp, char *ctxName, const char* name, const char* filename, const int fontIndex)
{
   fprintf(fp, "nvgCreateFontAtIndex( %s, \"%s\", \"%s\", %d)\n", ctxName ? ctxName : "vg", name ? name : "(null)", filename ? filename : "(null)", fontIndex);
}

void _nvgCreateFontAtIndex_leave( FILE *fp, char *ctxName, const char* name, const char* filename, const int fontIndex, int return_value)
{
   fprintf(fp, "nvgCreateFontAtIndex <- %d\n", return_value);
}

void _nvgCreateFontMem_enter( FILE *fp, char *ctxName, const char* name, unsigned char* data, int ndata, int freeData)
{
   fprintf(fp, "nvgCreateFontMem( %s, \"%s\", %p, %d, %d)\n", ctxName ? ctxName : "vg", name ? name : "(null)", data, ndata, freeData);
}

void _nvgCreateFontMem_leave( FILE *fp, char *ctxName, const char* name, unsigned char* data, int ndata, int freeData, int return_value)
{
   fprintf(fp, "nvgCreateFontMem <- %d\n", return_value);
}

void _nvgCreateFontMemAtIndex_enter( FILE *fp, char *ctxName, const char* name, unsigned char* data, int ndata, int freeData, const int fontIndex)
{
   fprintf(fp, "nvgCreateFontMemAtIndex( %s, \"%s\", %p, %d, %d, %d)\n", ctxName ? ctxName : "vg", name ? name : "(null)", data, ndata, freeData, fontIndex);
}

void _nvgCreateFontMemAtIndex_leave( FILE *fp, char *ctxName, const char* name, unsigned char* data, int ndata, int freeData, const int fontIndex, int return_value)
{
   fprintf(fp, "nvgCreateFontMemAtIndex <- %d\n", return_value);
}

void _nvgFindFont_enter( FILE *fp, char *ctxName, const char* name)
{
   fprintf(fp, "nvgFindFont( %s, \"%s\")\n", ctxName ? ctxName : "vg", name ? name : "(null)");
}

void _nvgFindFont_leave( FILE *fp, char *ctxName, const char* name, int return_value)
{
   fprintf(fp, "nvgFindFont <- %d\n", return_value);
}

void _nvgAddFallbackFontId_enter( FILE *fp, char *ctxName, int baseFont, int fallbackFont)
{
   fprintf(fp, "nvgAddFallbackFontId( %s, %d, %d)\n", ctxName ? ctxName : "vg", baseFont, fallbackFont);
}

void _nvgAddFallbackFontId_leave( FILE *fp, char *ctxName, int baseFont, int fallbackFont, int return_value)
{
   fprintf(fp, "nvgAddFallbackFontId <- %d\n", return_value);
}

void _nvgAddFallbackFont_enter( FILE *fp, char *ctxName, const char* baseFont, const char* fallbackFont)
{
   fprintf(fp, "nvgAddFallbackFont( %s, \"%s\", \"%s\")\n", ctxName ? ctxName : "vg", baseFont ? baseFont : "(null)", fallbackFont ? fallbackFont : "(null)");
}

void _nvgAddFallbackFont_leave( FILE *fp, char *ctxName, const char* baseFont, const char* fallbackFont, int return_value)
{
   fprintf(fp, "nvgAddFallbackFont <- %d\n", return_value);
}

void _nvgGetFontHandle_enter( FILE *fp, char *ctxName, int fontId)
{
   fprintf(fp, "nvgGetFontHandle( %s, %d)\n", ctxName ? ctxName : "vg", fontId);
}

void _nvgGetFontHandle_leave( FILE *fp, char *ctxName, int fontId, void* return_value)
{
   fprintf(fp, "nvgGetFontHandle <- %p\n", return_value);
}

void _nvgTextBounds_enter( FILE *fp, char *ctxName, float x, float y, const char* string, const char* end, float* bounds)
{
   if (string && end && end > string)
      fprintf(fp, "nvgTextBounds( %s, %f, %f, \"%.*s\", +%td, %p)\n", ctxName ? ctxName : "vg", x, y, (int)(end - string), string, end - string, bounds);
   else if (end)
      fprintf(fp, "nvgTextBounds( %s, %f, %f, \"%s\", +%td, %p)\n", ctxName ? ctxName : "vg", x, y, string ? string : "(null)", end - string, bounds);
   else
      fprintf(fp, "nvgTextBounds( %s, %f, %f, \"%s\", NULL, %p)\n", ctxName ? ctxName : "vg", x, y, string ? string : "(null)", bounds);
}

void _nvgTextBounds_leave( FILE *fp, char *ctxName, float x, float y, const char* string, const char* end, float* bounds, float return_value)
{
   fprintf(fp, "nvgTextBounds <- %f\n", return_value);
}

void _nvgTextBoxBounds_enter( FILE *fp, char *ctxName, float x, float y, float breakRowWidth, const char* string, const char* end, float* bounds)
{
   if (string && end && end > string)
      fprintf(fp, "nvgTextBoxBounds( %s, %f, %f, %f, \"%.*s\", +%td, %p)\n", ctxName ? ctxName : "vg", x, y, breakRowWidth, (int)(end - string), string, end - string, bounds);
   else if (end)
      fprintf(fp, "nvgTextBoxBounds( %s, %f, %f, %f, \"%s\", +%td, %p)\n", ctxName ? ctxName : "vg", x, y, breakRowWidth, string ? string : "(null)", end - string, bounds);
   else
      fprintf(fp, "nvgTextBoxBounds( %s, %f, %f, %f, \"%s\", NULL, %p)\n", ctxName ? ctxName : "vg", x, y, breakRowWidth, string ? string : "(null)", bounds);
}

void _nvgTextBoxBounds_leave( FILE *fp, char *ctxName, float x, float y, float breakRowWidth, const char* string, const char* end, float* bounds)
{
   fprintf(fp, "nvgTextBoxBounds completed\n");
}

void _nvgTextGlyphPositions_enter( FILE *fp, char *ctxName, float x, float y, const char* string, const char* end, NVGglyphPosition* positions, int maxPositions)
{
   if (string && end && end > string)
      fprintf(fp, "nvgTextGlyphPositions( %s, %f, %f, \"%.*s\", +%td, %p, %d)\n", ctxName ? ctxName : "vg", x, y, (int)(end - string), string, end - string, positions, maxPositions);
   else if (end)
      fprintf(fp, "nvgTextGlyphPositions( %s, %f, %f, \"%s\", +%td, %p, %d)\n", ctxName ? ctxName : "vg", x, y, string ? string : "(null)", end - string, positions, maxPositions);
   else
      fprintf(fp, "nvgTextGlyphPositions( %s, %f, %f, \"%s\", NULL, %p, %d)\n", ctxName ? ctxName : "vg", x, y, string ? string : "(null)", positions, maxPositions);
}

void _nvgTextGlyphPositions_leave( FILE *fp, char *ctxName, float x, float y, const char* string, const char* end, NVGglyphPosition* positions, int maxPositions, int return_value)
{
   fprintf(fp, "nvgTextGlyphPositions <- %d\n", return_value);
}

void _nvgTextMetrics_enter( FILE *fp, char *ctxName, float* ascender, float* descender, float* lineh)
{
   fprintf(fp, "nvgTextMetrics( %s, %p, %p, %p)\n", ctxName ? ctxName : "vg", ascender, descender, lineh);
}

void _nvgTextVisualMetrics_enter( FILE *fp, char *ctxName, float* ascender, float* descender, float* lineh)
{
   fprintf(fp, "nvgTextVisualMetrics( %s, %p, %p, %p)\n", ctxName ? ctxName : "vg", ascender, descender, lineh);
}

void _nvgTextVisualMetrics_leave( FILE *fp, char *ctxName, float* ascender, float* descender, float* lineh)
{
   fprintf(fp, "nvgTextVisualMetrics <- { ");
   if( ascender)
      fprintf( fp, "ascender: %f", *ascender);
   if( descender)
   {
      if( ascender)
         fprintf( fp, ", ");
      fprintf( fp, "descender: %f", *descender);
   }
   if( lineh)
   {
      if( ascender || descender)
         fprintf( fp, ", ");
      fprintf( fp, "lineh: %f", *lineh);
   }
   fprintf( fp, " }\n");
}

void _nvgTextBreakLines_enter( FILE *fp, char *ctxName, const char* string, const char* end, float breakRowWidth, NVGtextRow* rows, int maxRows)
{
   if (string && end && end > string)
      fprintf(fp, "nvgTextBreakLines( %s, \"%.*s\", +%td, %f, %p, %d)\n", ctxName ? ctxName : "vg", (int)(end - string), string, end - string, breakRowWidth, rows, maxRows);
   else if (end)
      fprintf(fp, "nvgTextBreakLines( %s, \"%s\", +%td, %f, %p, %d)\n", ctxName ? ctxName : "vg", string ? string : "(null)", end - string, breakRowWidth, rows, maxRows);
   else
      fprintf(fp, "nvgTextBreakLines( %s, \"%s\", NULL, %f, %p, %d)\n", ctxName ? ctxName : "vg", string ? string : "(null)", breakRowWidth, rows, maxRows);
}

void _nvgTextBreakLines_leave( FILE *fp, char *ctxName, const char* string, const char* end, float breakRowWidth, NVGtextRow* rows, int maxRows, int return_value)
{
   fprintf(fp, "nvgTextBreakLines <- %d\n", return_value);
}

void _nvgTextMetrics_leave( FILE *fp, char *ctxName, float* ascender, float* descender, float* lineh)
{
   fprintf(fp, "nvgTextMetrics <- { ");
   if( ascender)
      fprintf( fp, "ascender: %f", *ascender);
   if( descender)
   {
      if( ascender)
         fprintf( fp, ", ");
      fprintf( fp, "descender: %f", *descender);
   }
   if( lineh)
   {
      if( ascender || descender)
         fprintf( fp, ", ");
      fprintf( fp, "lineh: %f", *lineh);
   }
   fprintf( fp, " }\n");
}

void _nvgTextVisualBounds_enter( FILE *fp, char *ctxName, float x, float y, const char* string, const char* end, float bounds[6])
{
   if (string && end && end > string)
      fprintf(fp, "nvgTextVisualBounds( %s, %f, %f, \"%.*s\", +%td, %p)\n", ctxName ? ctxName : "vg", x, y, (int)(end - string), string, end - string, bounds);
   else if (end)
      fprintf(fp, "nvgTextVisualBounds( %s, %f, %f, \"%s\", +%td, %p)\n", ctxName ? ctxName : "vg", x, y, string ? string : "(null)", end - string, bounds);
   else
      fprintf(fp, "nvgTextVisualBounds( %s, %f, %f, \"%s\", NULL, %p)\n", ctxName ? ctxName : "vg", x, y, string ? string : "(null)", bounds);
}

void _nvgTextVisualBounds_leave( FILE *fp, char *ctxName, float x, float y, const char* string, const char* end, float bounds[6])
{
   fprintf(fp, "nvgTextVisualBounds <- { ");
   if( bounds)
   {
      fprintf(fp, "bounds: [%f, %f, %f, %f, %f, %f]",
              bounds[0], bounds[1], bounds[2], bounds[3], bounds[4], bounds[5]);
   }
   else
   {
      fprintf(fp, "bounds: (null)");
   }
   fprintf(fp, " }\n");
}

void _nvgGetScissor_enter( FILE *fp, char *ctxName, NVGscissor *scissor)
{
   fprintf(fp, "nvgGetScissor( %s, %p)\n", ctxName ? ctxName : "vg", scissor);
}

void _nvgGetScissor_leave( FILE *fp, char *ctxName, NVGscissor *scissor)
{
   fprintf(fp, "nvgGetScissor( %s, %p) <- void\n", ctxName ? ctxName : "vg", scissor);
}