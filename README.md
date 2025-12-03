# mulle-nanovg

A unified, monolithic build of NanoVG with Metal and DirectX 11 backends.

## What is this?

This project combines:

- **nanovg** - Antialiased 2D vector drawing library (custom fork with trace support)
- **MetalNanoVG** - Metal backend for macOS/iOS
- **nanovg_dx11** - DirectX 11 backend for Windows

Into a single library that can be built on all supported platforms without external dependencies (beyond system graphics APIs).

## Status

🚧 **Work in Progress** 

See [MATCHPLAN.md](MATCHPLAN.md) for detailed merge strategy.

## Supported Platforms

- **macOS/iOS**: Metal backend
- **Windows**: DirectX 11 backend
- **Linux**: TBD (OpenGL ES or Vulkan in future)

> ### Why is there no abstract API for framebuffers ?
> 
> Currently it seems nice to be able to support both vulkan and opengl on the
> same platform. The drawing code is the same, but the framebuffer code 
> diverges. So the selection which code to use should be an API user problem
> at a higher level. (Future direction: update the NVGcontext)
> This also simplifies linking. We require both system libraries
> during development and we can probably in the future just #ifdef out the
> backend we dont want to link against during recompilation)

## Building

This is a mulle-sde project. To build:

```bash
mulle-sde craft
```

## Usage

```c
#include <mulle-nanovg.h>

// Platform-specific context creation
#ifdef __APPLE__
  NVGcontext* vg = nvgCreateMTL(metalLayer, NVG_ANTIALIAS);
#elif _WIN32
  NVGcontext* vg = nvgCreateD3D11(d3dDevice, NVG_ANTIALIAS);
#endif

// Use standard NanoVG API
nvgBeginFrame(vg, width, height, pixelRatio);
// ... drawing commands ...
nvgEndFrame(vg);

// Cleanup
#ifdef __APPLE__
  nvgDeleteMTL(vg);
#elif _WIN32
  nvgDeleteD3D11(vg);
#endif
```

## License

This project combines code from multiple sources:
- nanovg: zlib license (see nanovg/LICENSE.txt)
- MetalNanoVG: MIT license
- nanovg_dx11: zlib license

See individual source files for specific license information.

## Credits

- **NanoVG**: Mikko Mononen
- **MetalNanoVG**: Olli Wang
- **nanovg_dx11**: Chris Maughan
- **Integration**: mulle-kybernetik
