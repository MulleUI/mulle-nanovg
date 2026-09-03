// NanoVG D3D11 non-anti-aliased pixel shader (fill.frag equivalent, no EDGE_AA).
//
// Regenerate the bytecode header on Windows with fxc:
//   fxc /T ps_4_0 /E main /Fh D3D11PixelShader.h /Vn g_D3D11PixelShader_Main D3D11PixelShader.hlsl
//
// Shader types (must match enum D3DNVGshaderType in nanovg_d3d11.c):
//   0 = FILLGRAD, 1 = FILLIMG, 2 = SIMPLE, 3 = IMG, 4 = CLEARTYPE, 5 = EMOJI

cbuffer PS_CONSTANTS : register(b0)
{
    float4x4 scissorMat;
    float4   scissorExt;
    float4   scissorScale;
    float4x4 paintMat;
    float4   extent;
    float4   radius;
    float4   feather;
    float4   innerCol;
    float4   outerCol;
    float4   strokeMult;   // x = strokeMult, y = strokeThr
    int      texType;
    int      type;
};

SamplerState g_sampler : register(s0);
Texture2D    g_texture : register(t0);

struct PS_INPUT
{
    float4 pos     : SV_Position;
    float2 ftcoord : TEXCOORD0;
    float2 fpos    : TEXCOORD1;
};

float sdroundrect(float2 pt, float2 ext, float rad)
{
    float2 ext2 = ext - float2(rad, rad);
    float2 d = abs(pt) - ext2;
    return min(max(d.x, d.y), 0.0) + length(max(d, 0.0)) - rad;
}

float scissorMask(float2 p)
{
    float2 sc = (abs(mul((float3x3)scissorMat, float3(p, 1.0)).xy) - scissorExt.xy);
    sc = float2(0.5, 0.5) - sc * scissorScale.xy;
    return clamp(sc.x, 0.0, 1.0) * clamp(sc.y, 0.0, 1.0);
}

float4 main(PS_INPUT input) : SV_TARGET
{
    float4 result;
    float scissor = scissorMask(input.fpos);
    float strokeAlpha = 1.0;

    if (type == 0)              // FILLGRAD
    {
        float2 pt = mul((float3x3)paintMat, float3(input.fpos, 1.0)).xy;
        float d = clamp((sdroundrect(pt, extent.xy, radius.x) + feather.x * 0.5) / feather.x, 0.0, 1.0);
        float4 color = lerp(innerCol, outerCol, d);
        color *= strokeAlpha * scissor;
        result = color;
    }
    else if (type == 1)         // FILLIMG
    {
        float2 pt = mul((float3x3)paintMat, float3(input.fpos, 1.0)).xy / extent.xy;
        float4 color = g_texture.Sample(g_sampler, pt);
        if (texType == 1) color = float4(color.xyz * color.w, color.w);
        if (texType == 2) color = float4(color.x, color.x, color.x, color.x);
        color *= innerCol;
        color *= strokeAlpha * scissor;
        result = color;
    }
    else if (type == 2)         // SIMPLE (stencil)
    {
        result = float4(1, 1, 1, 1);
    }
    else if (type == 3)         // IMG (textured tris)
    {
        float4 color = g_texture.Sample(g_sampler, input.ftcoord);
        if (texType == 1) color = float4(color.xyz * color.w, color.w);
        if (texType == 2) color = float4(color.x, color.x, color.x, color.x);
        color *= scissor;
        result = color * innerCol;
    }
    else if (type == 4)         // CLEARTYPE (subpixel text)
    {
        float4 color = g_texture.Sample(g_sampler, input.ftcoord);
        float deriv = ddx(input.ftcoord.x);
        if (deriv < 0.0)
            color.xz = color.zx;
        else if (deriv == 0.0)
            color = float4((color.x + color.y + color.z) * (1.0 / 3.0),
                           (color.x + color.y + color.z) * (1.0 / 3.0),
                           (color.x + color.y + color.z) * (1.0 / 3.0),
                           (color.x + color.y + color.z) * (1.0 / 3.0));
        if (color.w * scissor * innerCol.w < (1.0 / 256.0))
            discard;
        result.xyz = color.xyz * innerCol.xyz + (float3(innerCol.w, innerCol.w, innerCol.w) - color.xyz) * outerCol.xyz;
        result.w = innerCol.w;
        result *= scissor;
    }
    else                        // EMOJI (type == 5): color glyph
    {
        float4 color = g_texture.Sample(g_sampler, input.ftcoord);
        result = color * scissor * innerCol.a;
    }
    return result;
}
