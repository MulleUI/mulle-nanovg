# Vulkan Backend Analysis

## Overview

Found: `nanovg_vulkan` - A Vulkan backend for NanoVG

**Location**: `./nanovg_vulkan/src/nanovg_vk.h`
**Size**: 2023 lines (header-only implementation)
**Status**: Appears to be a complete, working implementation

## Key Files

```
nanovg_vulkan/
├── src/
│   ├── nanovg_vk.h          # Main Vulkan backend (2023 lines)
│   ├── nanovg.h/c           # Core nanovg
│   ├── nanovg_gl.h          # Also has OpenGL
│   └── shader/              # Vulkan shaders
│       ├── fill.frag        # Fragment shader
│       ├── fill.vert        # Vertex shader
│       ├── fill.frag.inc    # Compiled SPIR-V
│       └── fill.vert.inc    # Compiled SPIR-V
└── example/
    └── example_vulkan.c     # Vulkan demo
```

## API

### Context Creation
```c
typedef struct VKNVGCreateInfo {
  VkPhysicalDevice gpu;
  VkDevice device;
  VkRenderPass renderpass;
  VkCommandBuffer *cmdBuffer;
  uint32_t swapchainImageCount;
  uint32_t *currentFrame;
  const VkAllocationCallbacks *allocator;
  VkNvgExt ext;  // Extension support flags
} VKNVGCreateInfo;

NVGcontext* nvgCreateVk(VKNVGCreateInfo createInfo, int flags, VkQueue queue);
void nvgDeleteVk(NVGcontext* ctx);
```

### Create Flags (Same as others!)
```c
enum NVGcreateFlags {
  NVG_ANTIALIAS = 1 << 0,
  NVG_STENCIL_STROKES = 1 << 1,
  NVG_DEBUG = 1 << 2,
};
```

## Comparison with Other Backends

### Similarities
✅ Same `NVGcreateFlags` enum (will conflict!)
✅ Header-only implementation (like DX11)
✅ Uses compiled shaders (SPIR-V, like DX11 uses HLSL bytecode)
✅ Standard NanoVG API once context is created

### Differences
⚠️ **Complex initialization** - needs VkDevice, VkRenderPass, VkQueue, etc.
⚠️ **No framebuffer API** - doesn't appear to have create/bind framebuffer functions
⚠️ **No ReadPixels** - no pixel reading function found
⚠️ **Render pass dependent** - tied to a specific VkRenderPass

## Integration Complexity

### Easy Parts (3/10)
- ✅ Copy `nanovg_vk.h` to `src/`
- ✅ Copy shader files to `src/vk_shaders/`
- ✅ Add `#ifdef __linux__` guards (Vulkan primarily for Linux)
- ✅ Add to `mulle-nanovg.h` conditionally

### Moderate Parts (5/10)
- ⚠️ **Enum conflicts**: `NVGcreateFlags` redefined (same as GL/Metal/DX11)
- ⚠️ **Header-only**: Need to extract implementation like we did for DX11
- ⚠️ **Shader compilation**: SPIR-V shaders need to be embedded

### Hard Parts (7/10)
- ⚠️ **No framebuffer API**: Would need to implement `vknvgCreateFramebuffer()`, `vknvgBindFramebuffer()`
- ⚠️ **No ReadPixels**: Would need to implement `vknvgReadPixels()` using Vulkan image copy
- ⚠️ **Complex initialization**: Users need to set up Vulkan context first

## Framebuffer Support

**Current**: ❌ None found
**Would need**:
- Create VkImage with VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
- Create VkImageView
- Create VkFramebuffer
- Implement bind/unbind logic
- **Complexity**: 7/10 (moderate Vulkan knowledge required)

## ReadPixels Support

**Current**: ❌ None found
**Would need**:
- Create staging buffer
- Copy image to buffer (vkCmdCopyImageToBuffer)
- Map buffer memory
- Copy to user buffer
- **Complexity**: 6/10 (standard Vulkan pattern)

## Recommendation

### Option 1: Add Vulkan Now (Moderate Effort)
**Pros**:
- Complete cross-platform coverage (GL/Metal/DX11/Vulkan)
- Linux gets modern backend option
- Future-proof

**Cons**:
- Need to implement framebuffer API (~200 lines)
- Need to implement ReadPixels (~150 lines)
- Need to resolve enum conflicts
- Extract header-only implementation
- **Total effort**: ~500 lines of code

**Complexity**: 6/10

### Option 2: Add Vulkan Later (Recommended)
**Pros**:
- ✅ Current setup already works (GL/Metal/DX11)
- ✅ Can add when actually needed
- ✅ OpenGL works fine on Linux for now

**Cons**:
- No modern Linux backend yet
- Missing Vulkan's performance benefits

**Complexity**: 0/10 (do nothing now)

## If We Add Vulkan

### File Structure
```
src/
├── nanovg_vk.h              # Declarations only
├── nanovg_vk.c              # Implementation (extracted)
├── vk_shaders/
│   ├── fill.frag.inc        # SPIR-V bytecode
│   └── fill.vert.inc        # SPIR-V bytecode
```

### API to Add
```c
// In nanovg_vk.h
NVGcontext* nvgCreateVk(VKNVGCreateInfo createInfo, int flags, VkQueue queue);
void nvgDeleteVk(NVGcontext* ctx);

// Framebuffer support (NEW)
int vknvgCreateFramebuffer(NVGcontext* ctx, int w, int h, int flags);
void vknvgBindFramebuffer(NVGcontext* ctx, int image);

// ReadPixels support (NEW)
void vknvgReadPixels(NVGcontext* ctx, int image, int x, int y, int width, int height, void* data);
```

### Integration Steps
1. Copy `nanovg_vk.h` → extract to `.h` + `.c`
2. Copy shader files
3. Implement framebuffer API
4. Implement ReadPixels API
5. Add to `mulle-nanovg.h` with `#ifdef VULKAN` or `#ifdef __linux__`
6. Update unified framebuffer wrapper

## Conclusion

**Vulkan backend exists and is complete**, but:
- Missing framebuffer API (need to add)
- Missing ReadPixels API (need to add)
- More complex than other backends
- Primarily useful for Linux

**Recommendation**: **Add later when needed**. Current GL/Metal/DX11 coverage is solid.

**If adding now**: Budget ~500 lines of code and 1-2 days of work.
