#ifndef NANOVG_TRACE_FUNCTIONS_H
#define NANOVG_TRACE_FUNCTIONS_H

#include "nanovg.h"
#include <stdio.h>

typedef struct NVGcontext   NVGcontext;

//// Enum printing functions
//void nvgContextTypePrint(FILE *fp, int value);
//void nvgWindingPrint(FILE *fp, int value);
//void nvgSolidityPrint(FILE *fp, int value);
//void nvgLineCapPrint(FILE *fp, int value);
//void nvgAlignPrint(FILE *fp, int value);
//void nvgBlendFactorPrint(FILE *fp, int value);
//void nvgCompositeOperationPrint(FILE *fp, int value);
//void nvgImageFlagsPrint(FILE *fp, int value);
//void nvgTessPrint(FILE *fp, int value);
//void nvgTexturePrint(FILE *fp, int value);

// Tracing function declarations

// Frame Management Functions
void _nvgBeginFrame_enter(FILE *fp, char *ctxName, float windowWidth, float windowHeight, float devicePixelRatio);
void _nvgCancelFrame_enter(FILE *fp, char *ctxName);
void _nvgChangeFrame_enter(FILE *fp, char *ctxName, float windowWidth, float windowHeight, float devicePixelRatio);
void _nvgFlushFrame_enter(FILE *fp, char *ctxName);
void _nvgEndFrame_enter(FILE *fp, char *ctxName);

// Composite Operation Functions
void _nvgGlobalCompositeOperation_enter(FILE *fp, char *ctxName, int op);
void _nvgGlobalCompositeBlendFunc_enter(FILE *fp, char *ctxName, int sfactor, int dfactor);
void _nvgGlobalCompositeBlendFuncSeparate_enter(FILE *fp, char *ctxName, int srcRGB, int dstRGB, int srcAlpha, int dstAlpha);

// State Management Functions
void _nvgSave_enter(FILE *fp, char *ctxName);
void _nvgRestore_enter(FILE *fp, char *ctxName);
void _nvgReset_enter(FILE *fp, char *ctxName);

// Render Style Functions
void _nvgShapeAntiAlias_enter(FILE *fp, char *ctxName, int enabled);
void _nvgStrokeColor_enter(FILE *fp, char *ctxName, NVGcolor color);
void _nvgStrokePaint_enter(FILE *fp, char *ctxName, NVGpaint paint);
void _nvgFillColor_enter(FILE *fp, char *ctxName, NVGcolor color);
void _nvgClearType_enter(FILE *fp, char *ctxName, int enabled);
void _nvgTextColor_enter(FILE *fp, char *ctxName, NVGcolor color);
void _nvgTextBackgroundColor_enter(FILE *fp, char *ctxName, NVGcolor color);
void _nvgFillPaint_enter(FILE *fp, char *ctxName, NVGpaint paint);
void _nvgMiterLimit_enter(FILE *fp, char *ctxName, float limit);
void _nvgStrokeWidth_enter(FILE *fp, char *ctxName, float size);
void _nvgLineCap_enter(FILE *fp, char *ctxName, int cap);
void _nvgLineJoin_enter(FILE *fp, char *ctxName, int join);
void _nvgGlobalAlpha_enter(FILE *fp, char *ctxName, float alpha);
void _nvgBezierTessellation_enter(FILE *fp, char *ctxName, int tess);

// Transform Functions
void _nvgResetTransform_enter(FILE *fp, char *ctxName);
void _nvgTransform_enter(FILE *fp, char *ctxName, float a, float b, float c, float d, float e, float f);
void _nvgTranslate_enter(FILE *fp, char *ctxName, float x, float y);
void _nvgRotate_enter(FILE *fp, char *ctxName, float angle);
void _nvgSkewX_enter(FILE *fp, char *ctxName, float angle);
void _nvgSkewY_enter(FILE *fp, char *ctxName, float angle);
void _nvgScale_enter(FILE *fp, char *ctxName, float x, float y);

// Image Functions
void _nvgUpdateImage_enter(FILE *fp, char *ctxName, int image, const unsigned char* data);
void _nvgImageSize_enter(FILE *fp, char *ctxName, int image, int* w, int* h);
void _nvgDeleteImage_enter(FILE *fp, char *ctxName, int image);
void _nvgForgetImageTexture_enter(FILE *fp, char *ctxName, int image);

// Scissor Functions
void _nvgScissor_enter(FILE *fp, char *ctxName, float x, float y, float w, float h);
void _nvgIntersectScissor_enter(FILE *fp, char *ctxName, float x, float y, float w, float h);
void _nvgResetScissor_enter(FILE *fp, char *ctxName);

// Path Functions
void _nvgBeginPath_enter(FILE *fp, char *ctxName);
void _nvgMoveTo_enter(FILE *fp, char *ctxName, float x, float y);
void _nvgLineTo_enter(FILE *fp, char *ctxName, float x, float y);
void _nvgBezierTo_enter(FILE *fp, char *ctxName, float c1x, float c1y, float c2x, float c2y, float x, float y);
void _nvgQuadTo_enter(FILE *fp, char *ctxName, float cx, float cy, float x, float y);
void _nvgArcTo_enter(FILE *fp, char *ctxName, float x1, float y1, float x2, float y2, float radius);
void _nvgClosePath_enter(FILE *fp, char *ctxName);
void _nvgPathWinding_enter(FILE *fp, char *ctxName, int dir);
void _nvgArc_enter(FILE *fp, char *ctxName, float cx, float cy, float r, float a0, float a1, int dir);
void _nvgRect_enter(FILE *fp, char *ctxName, float x, float y, float w, float h);
void _nvgRoundedRect_enter(FILE *fp, char *ctxName, float x, float y, float w, float h, float r);
void _nvgRoundedRectVarying_enter(FILE *fp, char *ctxName, float x, float y, float w, float h, float radTopLeft, float radTopRight, float radBottomRight, float radBottomLeft);
void _nvgEllipse_enter(FILE *fp, char *ctxName, float cx, float cy, float rx, float ry);
void _nvgCircle_enter(FILE *fp, char *ctxName, float cx, float cy, float r);
void _nvgFill_enter(FILE *fp, char *ctxName);
void _nvgStroke_enter(FILE *fp, char *ctxName);
void _nvgPrecompilePath_enter(FILE *fp,char *ctxName);
void _nvgCopyPath_enter(FILE *fp, char *ctxName, NVGcontext *path);

// Font Functions
void _nvgFontSize_enter(FILE *fp, char *ctxName, float size);
void _nvgFontBlur_enter(FILE *fp, char *ctxName, float blur);
void _nvgTextLetterSpacing_enter(FILE *fp, char *ctxName, float spacing);
void _nvgTextLineHeight_enter(FILE *fp, char *ctxName, float lineHeight);
void _nvgTextAlign_enter(FILE *fp, char *ctxName, int align);
void _nvgFontFaceId_enter(FILE *fp, char *ctxName, int font);
void _nvgFontFace_enter(FILE *fp, char *ctxName, const char* font);
void _nvgText_enter(FILE *fp, char *ctxName, float x, float y, const char* string, const char* end);
void _nvgText_leave(FILE *fp, char *ctxName, float x, float y, const char* string, const char* end, float return_value);
void _nvgTextBox_enter(FILE *fp, char *ctxName, float x, float y, float breakRowWidth, const char* string, const char* end);
void _nvgResetFallbackFontsId_enter(FILE *fp, char *ctxName, int baseFont);
void _nvgResetFallbackFonts_enter(FILE *fp, char *ctxName, const char* baseFont);

// Image Creation Functions
void _nvgCreateImage_enter(FILE *fp, char *ctxName, const char* filename, int imageFlags);
void _nvgCreateImage_leave(FILE *fp, char *ctxName, const char* filename, int imageFlags, int return_value);
void _nvgCreateImageMem_enter(FILE *fp, char *ctxName, int imageFlags, unsigned char* data, int ndata);
void _nvgCreateImageMem_leave(FILE *fp, char *ctxName, int imageFlags, unsigned char* data, int ndata, int return_value);
void _nvgCreateImageRGBA_enter(FILE *fp, char *ctxName, int w, int h, int imageFlags, const unsigned char* data);
void _nvgCreateImageRGBA_leave(FILE *fp, char *ctxName, int w, int h, int imageFlags, const unsigned char* data, int return_value);
void _nvgCreateImageTexture_enter(FILE *fp, char *ctxName, int w, int h, int imageFlags, int type, void **texture);
void _nvgCreateImageTexture_leave(FILE *fp, char *ctxName, int w, int h, int imageFlags, int type, void **texture, int return_value);

// Image Creation Functions (continued)
void _nvgImageTextureInfo_enter(FILE *fp, char *ctxName, int image, int *imageFlags, int *type, void **texture);
void _nvgImageTextureInfo_leave(FILE *fp, char *ctxName, int image, int *imageFlags, int *type, void **texture);
void _nvgImageSize_leave( FILE *fp, char *ctxName, int image, int* w, int* h);

// Transform functions
void _nvgCurrentTransform_enter(FILE *fp, char *ctxName, float* xform);
void _nvgCurrentTransform_leave(FILE *fp, char *ctxName, float* xform);

// Paint Functions
void _nvgLinearGradient_enter(FILE *fp, char *ctxName, float sx, float sy, float ex, float ey, NVGcolor icol, NVGcolor ocol);
void _nvgLinearGradient_leave(FILE *fp, char *ctxName, float sx, float sy, float ex, float ey, NVGcolor icol, NVGcolor ocol, NVGpaint return_value);
void _nvgBoxGradient_enter(FILE *fp, char *ctxName, float x, float y, float w, float h, float r, float f, NVGcolor icol, NVGcolor ocol);
void _nvgBoxGradient_leave(FILE *fp, char *ctxName, float x, float y, float w, float h, float r, float f, NVGcolor icol, NVGcolor ocol, NVGpaint return_value);
void _nvgRadialGradient_enter(FILE *fp, char *ctxName, float cx, float cy, float inr, float outr, NVGcolor icol, NVGcolor ocol);
void _nvgRadialGradient_leave(FILE *fp, char *ctxName, float cx, float cy, float inr, float outr, NVGcolor icol, NVGcolor ocol, NVGpaint return_value);
void _nvgImagePattern_enter(FILE *fp, char *ctxName, float ox, float oy, float ex, float ey, float angle, int image, float alpha);
void _nvgImagePattern_leave(FILE *fp, char *ctxName, float ox, float oy, float ex, float ey, float angle, int image, float alpha, NVGpaint return_value);

// Font Functions (continued)
void _nvgCreateFont_enter(FILE *fp, char *ctxName, const char* name, const char* filename);
void _nvgCreateFont_leave(FILE *fp, char *ctxName, const char* name, const char* filename, int return_value);
void _nvgCreateFontAtIndex_enter(FILE *fp, char *ctxName, const char* name, const char* filename, const int fontIndex);
void _nvgCreateFontAtIndex_leave(FILE *fp, char *ctxName, const char* name, const char* filename, const int fontIndex, int return_value);
void _nvgCreateFontMem_enter(FILE *fp, char *ctxName, const char* name, unsigned char* data, int ndata, int freeData);
void _nvgCreateFontMem_leave(FILE *fp, char *ctxName, const char* name, unsigned char* data, int ndata, int freeData, int return_value);
void _nvgCreateFontMemAtIndex_enter(FILE *fp, char *ctxName, const char* name, unsigned char* data, int ndata, int freeData, const int fontIndex);
void _nvgCreateFontMemAtIndex_leave(FILE *fp, char *ctxName, const char* name, unsigned char* data, int ndata, int freeData, const int fontIndex, int return_value);
void _nvgFindFont_enter(FILE *fp, char *ctxName, const char* name);
void _nvgFindFont_leave(FILE *fp, char *ctxName, const char* name, int return_value);
void _nvgAddFallbackFontId_enter(FILE *fp, char *ctxName, int baseFont, int fallbackFont);
void _nvgAddFallbackFontId_leave(FILE *fp, char *ctxName, int baseFont, int fallbackFont, int return_value);
void _nvgAddFallbackFont_enter(FILE *fp, char *ctxName, const char* baseFont, const char* fallbackFont);
void _nvgAddFallbackFont_leave(FILE *fp, char *ctxName, const char* baseFont, const char* fallbackFont, int return_value);
void _nvgGetFontHandle_enter(FILE *fp, char *ctxName, int fontId);
void _nvgGetFontHandle_leave(FILE *fp, char *ctxName, int fontId, void* return_value);
void _nvgTextBounds_enter(FILE *fp, char *ctxName, float x, float y, const char* string, const char* end, float* bounds);
void _nvgTextBounds_leave(FILE *fp, char *ctxName, float x, float y, const char* string, const char* end, float* bounds, float return_value);
void _nvgTextVisualBounds_enter(FILE *fp, char *ctxName, float x, float y, const char* string, const char* end, float bounds[6]);
void _nvgTextVisualBounds_leave(FILE *fp, char *ctxName, float x, float y, const char* string, const char* end, float bounds[6]);
void _nvgTextBoxBounds_enter(FILE *fp, char *ctxName, float x, float y, float breakRowWidth, const char* string, const char* end, float* bounds);
void _nvgTextBoxBounds_leave(FILE *fp, char *ctxName, float x, float y, float breakRowWidth, const char* string, const char* end, float* bounds);
void _nvgTextGlyphPositions_enter(FILE *fp, char *ctxName, float x, float y, const char* string, const char* end, NVGglyphPosition* positions, int maxPositions);
void _nvgTextGlyphPositions_leave(FILE *fp, char *ctxName, float x, float y, const char* string, const char* end, NVGglyphPosition* positions, int maxPositions, int return_value);
void _nvgTextMetrics_enter(FILE *fp, char *ctxName, float* ascender, float* descender, float* lineh);
void _nvgTextMetrics_leave(FILE *fp, char *ctxName, float* ascender, float* descender, float* lineh);
void _nvgTextVisualMetrics_enter(FILE *fp, char *ctxName, float* ascender, float* descender, float* lineh);
void _nvgTextVisualMetrics_leave(FILE *fp, char *ctxName, float* ascender, float* descender, float* lineh);
void _nvgTextBreakLines_enter(FILE *fp, char *ctxName, const char* string, const char* end, float breakRowWidth, NVGtextRow* rows, int maxRows);
void _nvgTextBreakLines_leave(FILE *fp, char *ctxName, const char* string, const char* end, float breakRowWidth, NVGtextRow* rows, int maxRows, int return_value);

// Internal/Utility Functions
void _nvgSetScissor_enter(FILE *fp, char *ctxName, NVGscissor *scissor);

void _nvgGetScissor_enter(FILE *fp, char *ctxName, NVGscissor *scissor);
void _nvgGetScissor_leave(FILE *fp, char *ctxName, NVGscissor *scissor);

// Tracing Utilities
void _nvgTraceComment_enter(FILE *fp, char *ctxName, const char *text);

#endif // NANOVG_TRACE_H