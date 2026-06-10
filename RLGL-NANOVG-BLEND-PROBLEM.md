# RLGL-NANOVG-BLEND-PROBLEM.md — Why rlgl's blend API destroys nanovg rendering

## The Problem

Calling `rlSetBlendMode()` from within nanovg's render flush silently destroys
all GL state that nanovg has set up, causing draw calls to produce no visible
output.

## The Mechanism

### rlgl's blend API has a hidden side effect

```c
void rlSetBlendMode(int mode)
{
    if (mode != currentMode || customModified)
    {
        rlDrawRenderBatch(RLGL.currentBatch);  // ← HERE IS THE PROBLEM
        // ... then sets blend state ...
    }
}
```

`rlSetBlendMode()` calls `rlDrawRenderBatch()` internally to flush rlgl's own
batched geometry before changing blend state. This is correct behavior for
rlgl's immediate-mode API — but catastrophic when called from code that manages
its own GL state.

### What rlDrawRenderBatch() does unconditionally

At the end of `rlDrawRenderBatch()`, regardless of whether there was any
batched geometry to draw:

```c
glBindVertexArray(0);   // unbinds ANY bound VAO
glUseProgram(0);        // unbinds ANY bound shader
```

### The sequence of destruction

In `rlglnvg__renderFlush()`:

```
1. rlEnableShader(nanovgShader)          ← shader bound ✓
2. rlEnableVertexArray(nanovgVAO)        ← VAO bound ✓
3. rlEnableVertexBuffer(nanovgVBO)       ← VBO bound ✓
4. upload vertices, set attributes       ← all configured ✓
5. set uniforms (viewSize, tex)          ← all configured ✓
6. ── first draw call ──
7.   rlglnvg__blendFuncSeparate()
8.     rlSetBlendMode(RL_BLEND_CUSTOM_SEPARATE)
9.       rlDrawRenderBatch()             ← UNBINDS VAO + SHADER!
10.  rlglnvg__convexFill()
11.    glDrawArrays(GL_TRIANGLE_FAN...)  ← draws with NO VAO, NO SHADER
12.                                         → produces nothing, GL errors
```

### Why it appeared to work in some demos

The `if` condition in `rlSetBlendMode` is:

```c
if (currentBlendMode != mode || customModified)
```

If the blend mode was already `RL_BLEND_CUSTOM_SEPARATE` from a previous frame
AND `glCustomBlendModeModified` was false, the function short-circuits and
doesn't call `rlDrawRenderBatch()`. This means:

- **Frame 1**: Always broken (mode transitions from default RL_BLEND_ALPHA)
- **Frame 2+**: May work if blend params don't change between frames
- **FBO readpixels test**: Always broken (only renders one frame)
- **On-screen demos**: First frame is blank but immediately overwritten, so
  it looks fine. Or the blend state happens to match.

## The Fix

Bypass rlgl's blend API entirely. Call `glBlendFuncSeparate()` directly:

```c
static void rlglnvg__blendFuncSeparate(RLGLNVGcontext* gl, const RLGLNVGblend* blend)
{
   if( gl->blendFunc.srcRGB   != blend->srcRGB   ||
       gl->blendFunc.dstRGB   != blend->dstRGB   ||
       gl->blendFunc.srcAlpha != blend->srcAlpha  ||
       gl->blendFunc.dstAlpha != blend->dstAlpha)
   {
      gl->blendFunc = *blend;
      glBlendFuncSeparate( blend->srcRGB, blend->dstRGB,
                           blend->srcAlpha, blend->dstAlpha);
   }
}
```

This matches what the GL backend (`nanovg_gl.h`) does — it never goes through
an intermediate abstraction layer for blend state.

## The General Rule

**Never call rlgl state-management functions from within nanovg's render flush.**

Many rlgl functions internally call `rlDrawRenderBatch()` to flush pending
state before making changes. The complete list (as of rlgl 5.0/raylib 5.5):

| Function | Why it flushes |
|----------|---------------|
| `rlSetBlendMode()` | Flushes before changing blend state |
| `rlSetTexture()` | Flushes when texture changes between draw calls |
| `rlBegin()` | Flushes when draw mode changes or batch is full |
| `rlSetShader()` | Flushes before switching shader program |
| `rlDrawRenderBatchActive()` | Explicit flush (this one is intentional) |
| `rlSetRenderBatchActive()` | Flushes current batch before switching |
| `rlCheckRenderBatchLimit()` | Flushes if batch buffer is near capacity |
| `rlVertex3f()` | Calls `rlCheckRenderBatchLimit()` internally |

Additionally, these wrapper functions call the dangerous ones above:

| Function | Calls |
|----------|-------|
| `rlSetBlendFactors()` | Only stores state, but typically paired with `rlSetBlendMode()` |
| `rlSetBlendFactorsSeparate()` | Same — only stores state, but useless without `rlSetBlendMode()` |

Any of these will unbind the active VAO and shader program. Since nanovg's
render flush sets up its own shader, VAO, and VBO at the start and expects them
to remain bound for all draw calls, calling any of these functions mid-flush
will silently break all subsequent rendering.

**Note:** `rlSetBlendFactors()` and `rlSetBlendFactorsSeparate()` themselves
do NOT flush. They only store values into `RLGL.State`. But they are useless
without calling `rlSetBlendMode()` first (which triggers the flush and reads
the stored values). This is an API ordering trap — you must set factors before
mode, but the mode call is what destroys your state.

### Safe rlgl functions (thin GL wrappers, no batch flush)

These are safe because they map 1:1 to GL calls with no side effects:

- `rlEnableShader()` / `rlDisableShader()`
- `rlEnableVertexArray()` / `rlDisableVertexArray()`
- `rlEnableVertexBuffer()` / `rlDisableVertexBuffer()`
- `rlEnableBackfaceCulling()` / `rlDisableBackfaceCulling()`
- `rlEnableDepthTest()` / `rlDisableDepthTest()`
- `rlEnableScissorTest()` / `rlDisableScissorTest()`
- `rlEnableColorBlend()` / `rlDisableColorBlend()`
- `rlColorMask()`
- `rlActiveTextureSlot()`
- `rlEnableFramebuffer()` / `rlDisableFramebuffer()`
- `rlViewport()`
- `rlSetVertexAttribute()` / `rlEnableVertexAttribute()` / `rlDisableVertexAttribute()`
- `rlSetUniform()`
- `rlDrawVertexArray()`
- `rlFinish()`

### Functions in mulle-rlgl.h (all safe, direct GL calls)

- `rlStencilFunc()`, `rlStencilOp()`, `rlStencilMask()`
- `rlStencilOpSeparate()`
- `rlEnableStencilTest()` / `rlDisableStencilTest()`
- `rlDrawVertexArrayFan()`, `rlDrawVertexArrayStrip()`
- `rlReadPixels()`
- `rlLoadRenderbuffer()` / `rlUnloadRenderbuffer()`
- `rlUploadVertexBuffer()`

## How to Audit for This

Search for any rlgl call inside `rlglnvg__renderFlush` or any function it calls
(fill, convexFill, stroke, triangles, setUniforms, blendFuncSeparate). If the
rlgl function's implementation contains `rlDrawRenderBatch`, it will break
nanovg.

```bash
grep -n 'rlDrawRenderBatch' rlgl.h
```

Cross-reference that with any `rl*` calls in `nanovg_rlgl.h` that happen during
the render flush path.
