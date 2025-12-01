# Merge Checklist

Quick reference for copying files from source projects to mulle-nanovg.

## Phase 1: Core nanovg

Source: `../nanovg/src/`

- [ ] `nanovg.h` → `src/nanovg.h`
- [ ] `nanovg.c` → `src/nanovg.c`
- [ ] `nanovg-trace-functions.h` → `src/nanovg-trace-functions.h`
- [ ] `nanovg-trace-functions.c` → `src/nanovg-trace-functions.c`
- [ ] `nanovg-trace-support.inc` → `src/nanovg-trace-support.inc`
- [ ] `fontstash.h` → `src/fontstash.h`
- [ ] `stb_image.h` → `src/stb_image.h`
- [ ] `stb_truetype.h` → `src/stb_truetype.h`

## Phase 2: OpenGL Backend

Source: `../nanovg/src/`

- [ ] `nanovg_gl.h` → `src/nanovg_gl.h`
- [ ] `nanovg_gl_utils.h` → `src/nanovg_gl_utils.h`

## Phase 3: Metal Backend

Source: `../MetalNanoVG/src/`

- [ ] `nanovg_mtl.h` → `src/nanovg_mtl.h`
- [ ] `nanovg_mtl.m` → `src/nanovg_mtl.m`
- [ ] `nanovg_mtl_shaders.metal` → `src/nanovg_mtl_shaders.metal`
- [ ] `mnvg_bitcode/ios.h` → `src/mnvg_bitcode/ios.h`
- [ ] `mnvg_bitcode/macos.h` → `src/mnvg_bitcode/macos.h`
- [ ] `mnvg_bitcode/simulator.h` → `src/mnvg_bitcode/simulator.h`
- [ ] `mnvg_bitcode/tvos.h` → `src/mnvg_bitcode/tvos.h`

## Phase 4: DirectX 11 Backend

Source: `../nanovg_dx11/src/`

- [ ] `nanovg_d3d11.h` → `src/nanovg_d3d11.h` (extract impl to .c)
- [ ] Create `src/nanovg_d3d11.c` (implementation from header)
- [ ] `D3D11VertexShader.h` → `src/d3d11_shaders/D3D11VertexShader.h`
- [ ] `D3D11PixelShader.h` → `src/d3d11_shaders/D3D11PixelShader.h`
- [ ] `D3D11PixelShaderAA.h` → `src/d3d11_shaders/D3D11PixelShaderAA.h`

**Framebuffer Support** (from `../nanovg_compat/src/nanovg_compat.c`):

- [ ] Extract `D3DNVGdevice` struct → add to `src/nanovg_d3d11.h`
- [ ] Extract `d3dnvgCreateFramebuffer()` → add to `src/nanovg_d3d11.c`
- [ ] Extract `d3dnvgBindFramebuffer()` → add to `src/nanovg_d3d11.c`

**Optional Utilities** (from nanovg_compat):
- [ ] Extract `d3dnvgCopyImage()` → add to `src/nanovg_d3d11.c`
- [ ] Extract `d3dnvgWriteImage()` → add to `src/nanovg_d3d11.c`
- [ ] Extract `d3dnvgReadPixels()` → add to `src/nanovg_d3d11.c`

**Note**: Base nanovg_dx11 has NO framebuffer support. Must extract from nanovg_compat.
See `DX11_FRAMEBUFFER_RESEARCH.md` for details.

## Phase 5: Unified Header

- [ ] Create `src/mulle-nanovg.h` with platform conditionals
- [ ] Create `src/nanovg_framebuffer.c` with unified framebuffer wrappers

## CMake Configuration

- [ ] Update `CMakeLists.txt` with source files
- [ ] Add platform conditionals for Metal (APPLE)
- [ ] Add platform conditionals for DX11 (WIN32)
- [ ] Add framework links for Metal
- [ ] Add library links for DX11
- [ ] Configure OpenGL linking

## Testing

- [ ] Build on macOS (Core + OpenGL + Metal)
- [ ] Build on Windows (Core + OpenGL + DX11)
- [ ] Build on Linux (Core + OpenGL)
- [ ] Test trace functions work
- [ ] Test framebuffer APIs
- [ ] Port demo.c example

## Documentation

- [ ] Update README.md with build instructions
- [ ] Document framebuffer API differences
- [ ] Add usage examples
- [ ] Document trace functionality
