#version 450

layout(constant_id = 0) const uint EDGE_AA = 0;
struct FragmentData {
    mat3 scissorMat;
    mat3 paintMat;
    vec4 innerCol;
    vec4 outerCol;
    vec2 scissorExt;
    vec2 scissorScale;
    vec2 extent;
    float radius;
    float feather;
    float strokeMult;
    float strokeThr;
    int texType;
    int type;
};
layout(std430, set = 0, binding = 0) readonly buffer FragmentBuffer {
    FragmentData data[];
};
layout(set = 1, binding = 1)uniform sampler2D tex;
layout(location = 0) in vec2 ftcoord;
layout(location = 1) in vec2 fpos;
layout(location = 2) in flat int fid;
layout(location = 0) out vec4 outColor;

float sdroundrect(vec2 pt, vec2 ext, float rad) {
    vec2 ext2 = ext - vec2(rad, rad);
    vec2 d = abs(pt) - ext2;
    return min(max(d.x, d.y), 0.0) + length(max(d, 0.0)) - rad;
}

// Scissoring
float scissorMask(vec2 p) {
    vec2 sc = (abs((data[fid].scissorMat * vec3(p, 1.0)).xy) - data[fid].scissorExt);
    sc = vec2(0.5, 0.5) - sc * data[fid].scissorScale;
    return clamp(sc.x, 0.0, 1.0) * clamp(sc.y, 0.0, 1.0);
}

// Stroke - from [0..1] to clipped pyramid, where the slope is 1px.
float strokeMask() {
    return min(1.0, (1.0-abs(ftcoord.x*2.0-1.0))*data[fid].strokeMult) * min(1.0, ftcoord.y);
}

void main(void) {
    vec4 result;
    float scissor = scissorMask(fpos);
    float strokeAlpha = 1.0;
    if (EDGE_AA == 1) {
        strokeAlpha = strokeMask();
        if (strokeAlpha < data[fid].strokeThr) discard;
    }
    if (data[fid].type == 0) { // Gradient
        // Calculate gradient color using box gradient
        vec2 pt = (data[fid].paintMat * vec3(fpos, 1.0)).xy;
        float d = clamp((sdroundrect(pt, data[fid].extent, data[fid].radius) + data[fid].feather*0.5) / data[fid].feather, 0.0, 1.0);
        vec4 color = mix(data[fid].innerCol, data[fid].outerCol, d);
        // Combine alpha
        color *= strokeAlpha * scissor;
        result = color;
    } else if (data[fid].type == 1) { // Image
        // Calculate color fron texture
        vec2 pt = (data[fid].paintMat * vec3(fpos, 1.0)).xy / data[fid].extent;
        vec4 color = texture(tex, pt);
        if (data[fid].texType == 1) color = vec4(color.xyz*color.w, color.w);
        if (data[fid].texType == 2) color = vec4(color.x);
        // Apply color tint and alpha.
        color *= data[fid].innerCol;
        // Combine alpha
        color *= strokeAlpha * scissor;
        result = color;
    } else if (data[fid].type == 2) { // Stencil fill
        result = vec4(1, 1, 1, 1);
    } else if (data[fid].type == 3) { // Textured tris
        vec4 color = texture(tex, ftcoord);
        if (data[fid].texType == 1) color = vec4(color.xyz*color.w, color.w);
        if (data[fid].texType == 2) color = vec4(color.x);
        color *= scissor;
        result = color * data[fid].innerCol;
    } else if (data[fid].type == 4) { // ClearType subpixel text
        vec4 color = texture(tex, ftcoord);
        float deriv = dFdx(ftcoord.x);
        if (deriv < 0.0)
            color.xz = color.zx; // Text horizontally mirrored/rotated 180: flip R and B subpixels.
        else if (deriv == 0.0) // Text rotated 90 degrees: average subpixels, disabling ClearType.
            color = vec4((color.x + color.y + color.z) * (1.0 / 3.0));
        if (color.w * scissor * data[fid].innerCol.w < (1.0 / 256.0))
            discard;
        result.xyz = color.xyz * data[fid].innerCol.xyz + (vec3(data[fid].innerCol.w) - color.xyz) * data[fid].outerCol.xyz;
        result.w = data[fid].innerCol.w;
        result *= scissor;
    } else if (data[fid].type == 5) { // Color emoji
        vec4 color = texture(tex, ftcoord);
        result = color * scissor * data[fid].innerCol.a;
    }
    outColor = result;
}