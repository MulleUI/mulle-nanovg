// Copyright (c) 2017 Ollix
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
//
// Shader-type numbering is UNIFIED across all nanovg backends
// (GL nanovg_gl.h, D3D11 D3D11PixelShader*.hlsl, Metal nanovg_mtl.m):
//   0 = FILLGRAD, 1 = FILLIMG, 2 = SIMPLE (stencil), 3 = IMG,
//   4 = CLEARTYPE, 5 = EMOJI
// Metal's shader has no explicit type-2 (SIMPLE) branch: stencil-only draws
// use a separate pipeline state, not a fragment shader-type.
//
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// ---
// Author: olliwang@ollix.com (Olli Wang)

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

typedef struct {
  float2 pos [[attribute(0)]];
  float2 tcoord [[attribute(1)]];
} Vertex;

typedef struct {
  float4 pos  [[position]];
  float2 fpos;
  float2 ftcoord;
} RasterizerData;

typedef struct  {
  float3x3 scissorMat;
  float3x3 paintMat;
  float4 innerCol;
  float4 outerCol;
  float2 scissorExt;
  float2 scissorScale;
  float2 extent;
  float radius;
  float feather;
  float strokeMult;
  float strokeThr;
  int texType;
  int type;
} Uniforms;

float scissorMask(constant Uniforms& uniforms, float2 p);
float sdroundrect(constant Uniforms& uniforms, float2 pt);
float strokeMask(constant Uniforms& uniforms, float2 ftcoord);

float scissorMask(constant Uniforms& uniforms, float2 p) {
  float2 sc = (abs((uniforms.scissorMat * float3(p, 1.0f)).xy)
                  - uniforms.scissorExt) \
              * uniforms.scissorScale;
  sc = saturate(float2(0.5f) - sc);
  return sc.x * sc.y;
}

float sdroundrect(constant Uniforms& uniforms, float2 pt) {
  float2 ext2 = uniforms.extent - float2(uniforms.radius);
  float2 d = abs(pt) - ext2;
  return min(max(d.x, d.y), 0.0) + length(max(d, 0.0)) - uniforms.radius;
}

float strokeMask(constant Uniforms& uniforms, float2 ftcoord) {
  return min(1.0, (1.0 - abs(ftcoord.x * 2.0 - 1.0)) * uniforms.strokeMult) \
         * min(1.0, ftcoord.y);
}

// Vertex Function
vertex RasterizerData vertexShader(Vertex vert [[stage_in]],
                                   constant float2& viewSize [[buffer(1)]]) {
  RasterizerData out;
  out.ftcoord = vert.tcoord;
  out.fpos = vert.pos;
  out.pos = float4(2.0 * vert.pos.x / viewSize.x - 1.0,
                   1.0 - 2.0 * vert.pos.y / viewSize.y,
                   0, 1);
  return out;
}

// Fragment function (No AA)
fragment float4 fragmentShader(RasterizerData in [[stage_in]],
                               constant Uniforms& uniforms [[buffer(0)]],
                               texture2d<float> texture [[texture(0)]],
                               sampler sampler [[sampler(0)]]) {
  float scissor = scissorMask(uniforms, in.fpos);
  if (scissor == 0)
    return float4(0);

  if (uniforms.type == 0) {  // MNVG_SHADER_FILLGRAD
    float2 pt = (uniforms.paintMat * float3(in.fpos, 1.0)).xy;
    float d = saturate((uniforms.feather * 0.5 + sdroundrect(uniforms, pt))
                       / uniforms.feather);
    float4 color = mix(uniforms.innerCol, uniforms.outerCol, d);
    return color * scissor;
  } else if (uniforms.type == 1) {  // MNVG_SHADER_FILLIMG
    float2 pt = (uniforms.paintMat * float3(in.fpos, 1.0)).xy / uniforms.extent;
    float4 color = texture.sample(sampler, pt);
    if (uniforms.texType == 1)
      color = float4(color.xyz * color.w, color.w);
    else if (uniforms.texType == 2)
      color = float4(color.x);
    color *= scissor;
    return color * uniforms.innerCol;
  } else if (uniforms.type == 4) {  // MNVG_SHADER_CLEARTYPE
    float4 color = texture.sample(sampler, in.ftcoord);
    float deriv = dfdx(in.ftcoord.x);
    if (deriv < 0.0)
      color.xz = color.zx;
    else if (deriv == 0.0)
      color = float4((color.x + color.y + color.z) * (1.0 / 3.0));
    if (color.w * scissor * uniforms.innerCol.w < (1.0 / 256.0))
      discard_fragment();
    float4 result;
    result.xyz = color.xyz * uniforms.innerCol.xyz
               + (float3(uniforms.innerCol.w) - color.xyz) * uniforms.outerCol.xyz;
    result.w = uniforms.innerCol.w;
    result *= scissor;
    return result;
  } else if (uniforms.type == 5) {  // MNVG_SHADER_EMOJI
    float4 color = texture.sample(sampler, in.ftcoord);
    // Color emoji atlas is straight (non-premultiplied) alpha; the nanovg
    // blend expects premultiplied source. Premultiply so antialiased edges
    // and transparent background blend correctly (matches the IMG path).
    color = float4(color.xyz * color.w, color.w);
    return color * scissor * uniforms.innerCol.a;
  } else {  // MNVG_SHADER_IMG (type == 3)
    float4 color = texture.sample(sampler, in.ftcoord);
    if (uniforms.texType == 1)
      color = float4(color.xyz * color.w, color.w);
    else if (uniforms.texType == 2)
      color = float4(color.x);
    color *= scissor;
    return color * uniforms.innerCol;
  }
}

// Fragment function (AA)
fragment float4 fragmentShaderAA(RasterizerData in [[stage_in]],
                                 constant Uniforms& uniforms [[buffer(0)]],
                                 texture2d<float> texture [[texture(0)]],
                                 sampler sampler [[sampler(0)]]) {
  float scissor = scissorMask(uniforms, in.fpos);
  if (scissor == 0)
    return float4(0);

  if (uniforms.type == 3) {  // MNVG_SHADER_IMG
    float4 color = texture.sample(sampler, in.ftcoord);
    if (uniforms.texType == 1)
      color = float4(color.xyz * color.w, color.w);
    else if (uniforms.texType == 2)
      color = float4(color.x);
    color *= scissor;
    return color * uniforms.innerCol;
  }

  if (uniforms.type == 4) {  // MNVG_SHADER_CLEARTYPE
    float4 color = texture.sample(sampler, in.ftcoord);
    float deriv = dfdx(in.ftcoord.x);
    if (deriv < 0.0)
      color.xz = color.zx;
    else if (deriv == 0.0)
      color = float4((color.x + color.y + color.z) * (1.0 / 3.0));
    if (color.w * scissor * uniforms.innerCol.w < (1.0 / 256.0))
      discard_fragment();
    float4 result;
    result.xyz = color.xyz * uniforms.innerCol.xyz
               + (float3(uniforms.innerCol.w) - color.xyz) * uniforms.outerCol.xyz;
    result.w = uniforms.innerCol.w;
    result *= scissor;
    return result;
  }

  if (uniforms.type == 5) {  // MNVG_SHADER_EMOJI
    float4 color = texture.sample(sampler, in.ftcoord);
    // Color emoji atlas is straight (non-premultiplied) alpha; the nanovg
    // blend expects premultiplied source. Premultiply so antialiased edges
    // and transparent background blend correctly (matches the IMG path).
    color = float4(color.xyz * color.w, color.w);
    return color * scissor * uniforms.innerCol.a;
  }

  float strokeAlpha = strokeMask(uniforms, in.ftcoord);
  if (strokeAlpha < uniforms.strokeThr) {
    return float4(0);
  }

  if (uniforms.type == 0) {  // MNVG_SHADER_FILLGRAD
    float2 pt = (uniforms.paintMat * float3(in.fpos, 1.0)).xy;
    float d = saturate((uniforms.feather * 0.5 + sdroundrect(uniforms, pt))
                        / uniforms.feather);
    float4 color = mix(uniforms.innerCol, uniforms.outerCol, d);
    color *= scissor;
    color *= strokeAlpha;
    return color;
  } else {  // MNVG_SHADER_FILLIMG
    float2 pt = (uniforms.paintMat * float3(in.fpos, 1.0)).xy / uniforms.extent;
    float4 color = texture.sample(sampler, pt);
    if (uniforms.texType == 1)
      color = float4(color.xyz * color.w, color.w);
    else if (uniforms.texType == 2)
      color = float4(color.x);
    color *= scissor;
    color *= strokeAlpha;
    return color * uniforms.innerCol;
  }
}
