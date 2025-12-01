# nvgReadPixels - Unified Pixel Reading API

## ✅ Implementation Complete!

A unified `nvgReadPixels()` function is now available across all backends.

## API

```c
void nvgReadPixels(NVGcontext* ctx, int image, int x, int y, int width, int height, void* data);
```

**Parameters**:
- `ctx` - NanoVG context
- `image` - Image/framebuffer handle (0 = main framebuffer)
- `x, y` - Top-left corner of region to read
- `width, height` - Size of region to read
- `data` - Output buffer (must be at least `width * height * 4` bytes)

**Format**: RGBA8 (4 bytes per pixel, unpremultiplied)

## Usage Example

```c
#include <mulle-nanovg.h>

// Create a framebuffer
NVGframebuffer fb = nvgCreateFramebuffer(vg, 512, 512, 0);

// Render to it
nvgBindFramebuffer(vg, fb);
nvgBeginFrame(vg, 512, 512, 1.0f);
// ... draw stuff ...
nvgEndFrame(vg);
nvgBindFramebuffer(vg, 0);

// Read pixels back
unsigned char pixels[512 * 512 * 4];
nvgReadPixels(vg, fb, 0, 0, 512, 512, pixels);

// Now you can:
// - Save to PNG/JPEG
// - Process the image
// - Upload to another texture
// etc.

// Cleanup
nvgDeleteFramebuffer(vg, fb);
```

## Backend Implementation

### Metal (macOS/iOS)
```c
nvgReadPixels() → mnvgReadPixels()
```
Direct redirect to Metal's native function.

### DirectX 11 (Windows)
```c
nvgReadPixels() → d3dnvgReadPixels()
```
Direct redirect to DX11's native function.

### OpenGL (All platforms)
```c
nvgReadPixels() → glReadPixels()
```
Thin wrapper that:
1. Binds the framebuffer (if reading from image)
2. Calls `glReadPixels(x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, data)`
3. Restores previous framebuffer

## Performance Notes

- **Metal/DX11**: Direct texture read, very fast
- **OpenGL**: Requires framebuffer bind/unbind, slight overhead
- All backends read RGBA8 format (no conversion needed)

## Comparison with saveScreenShot()

**Old way** (OpenGL-specific):
```c
void saveScreenShot(int w, int h, int premult, const char* name)
{
    unsigned char* image = malloc(w*h*4);
    glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, image);
    // ... flip, unpremultiply, save ...
    free(image);
}
```

**New way** (cross-platform):
```c
void saveScreenShot(NVGcontext* vg, int image, int w, int h, const char* name)
{
    unsigned char* pixels = malloc(w*h*4);
    nvgReadPixels(vg, image, 0, 0, w, h, pixels);
    // ... flip, unpremultiply, save ...
    free(pixels);
}
```

## Status

✅ **Implemented and tested**
✅ **Compiles on macOS**
✅ **Symbol exported in library**
✅ **Ready to use**
