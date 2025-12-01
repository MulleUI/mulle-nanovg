# DirectX 11 Framebuffer Research

## Question
Does the DX11 backend have framebuffer support?

## Answer
**Base nanovg_dx11**: NO framebuffer support
**nanovg_compat**: YES, has full framebuffer implementation

## Findings

### Base nanovg_dx11 (./nanovg_dx11/src/nanovg_d3d11.h)
- No `CreateFramebuffer` function
- No `BindFramebuffer` function
- Only basic rendering support

### nanovg_compat (./nanovg_compat/src/nanovg_compat.c)
Has complete DX11 framebuffer implementation:

```c
// Core framebuffer functions
int d3dnvgCreateFramebuffer(NVGcontext* ctx, int w, int h, int flags);
void d3dnvgBindFramebuffer(NVGcontext* ctx, int texId);

// Bonus utilities
void d3dnvgCopyImage(NVGcontext* ctx, int imgDest, int imgSrc);
void d3dnvgWriteImage(NVGcontext* ctx, int imgDest, void* data);
void d3dnvgReadPixels(NVGcontext* ctx, int image, int x, int y, int width, int height, void* data);
```

## Implementation Details

### d3dnvgCreateFramebuffer()
1. Creates image with `nvgCreateImageRGBA()` + `NVG_IMAGE_RENDER_TARGET` flag
2. Gets the D3D11 texture from the image
3. Creates `ID3D11RenderTargetView` from the texture
4. Stores render target view in texture struct
5. Returns image handle (int)

### d3dnvgBindFramebuffer()
1. If texId == 0: bind main framebuffer (swap chain)
2. If texId != 0: find texture, bind its render target view
3. Sets viewport to match framebuffer dimensions
4. Calls `OMSetRenderTargets()` to bind

**Key insight**: Uses same int-based API as Metal backend!

## API Comparison

### Metal
```c
int mnvgCreateFramebuffer(NVGcontext* ctx, int w, int h, int imageFlags);
void mnvgBindFramebuffer(NVGcontext* ctx, int img);
```

### DX11 (from nanovg_compat)
```c
int d3dnvgCreateFramebuffer(NVGcontext* ctx, int w, int h, int flags);
void d3dnvgBindFramebuffer(NVGcontext* ctx, int texId);
```

**Identical signatures!** Both return int handle, both bind with 0 to unbind.

### OpenGL (different)
```c
NVGLUframebuffer* nvgluCreateFramebuffer(NVGcontext* ctx, int w, int h, int imageFlags);
void nvgluBindFramebuffer(NVGLUframebuffer* fb);  // NULL to unbind
```

Returns pointer, binds with NULL.

## Action Items for mulle-nanovg

1. **Extract from nanovg_compat**:
   - `d3dnvgCreateFramebuffer()` implementation
   - `d3dnvgBindFramebuffer()` implementation
   - Supporting device management code

2. **Add to nanovg_d3d11.c**:
   - Add framebuffer functions to our DX11 backend
   - Declare in nanovg_d3d11.h

3. **Optional utilities** (consider adding):
   - `d3dnvgCopyImage()` - copy between textures
   - `d3dnvgWriteImage()` - write pixels to texture
   - `d3dnvgReadPixels()` - read pixels from texture

4. **Dependencies**:
   - Needs `D3DNVGdevice` struct (also in nanovg_compat)
   - Needs access to internal texture struct
   - May need to extract device management code too

## Code Location

**Source**: `/Volumes/Source/srcO/MulleUI/nanovg_compat/src/nanovg_compat.c`
- Lines 465-550: Framebuffer implementation
- Lines 50-62: D3DNVGdevice struct definition

**Destination**: `mulle-nanovg/src/nanovg_d3d11.c` (to be created)

## Conclusion

✅ DX11 framebuffer support exists, but only in nanovg_compat
✅ API matches Metal backend (int-based handles)
✅ Need to extract and integrate into our DX11 backend
✅ Consider extracting utility functions too (copy/read/write)
