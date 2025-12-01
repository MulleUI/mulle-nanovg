# Brainstorming Session Summary - 2025-12-01

## Goal
Merge nanovg + Metal + DX11 + OpenGL backends into unified mulle-nanovg library.

## Key Decisions

### ✅ What We're Keeping
- **Core nanovg.h/nanovg.c**: Unchanged, backend-agnostic
- **Trace functions**: Critical debugging infrastructure, must preserve
- **OpenGL backend**: Needed for framebuffers and cross-platform support
- **Backend-specific names**: `nvgCreateGL3()`, `nvgCreateMTL()`, `nvgCreateD3D11()`
- **Platform-specific compilation**: Backends only compile on their target platforms

### ❌ What We're Dropping (for now)
- **nanovg_compat utilities**: Can add back later if needed
- **Unified context creation**: Keep backend-specific functions, clearer intent
- **API abstraction layer**: Let users call backends directly

## Architecture

```
Core (all platforms):
  nanovg.h/c + trace functions + OpenGL backend

Platform-specific:
  macOS/iOS: + Metal backend
  Windows:   + DX11 backend
  Linux:     (just OpenGL)
```

## Framebuffer APIs

**Different but similar**:
- OpenGL: Returns `NVGLUframebuffer*`, bind with NULL to unbind
- Metal: Returns `int` (image handle), bind with 0 to unbind
- DX11: Need to verify if supported

**Decision**: Document differences, expose native APIs per backend.

## File Structure

```
src/
├── nanovg.h/c                    # Core (unchanged)
├── nanovg-trace-functions.h/c    # Trace support (keep!)
├── fontstash.h, stb_*.h          # Dependencies
├── nanovg_gl.h                   # OpenGL (all platforms)
├── nanovg_gl_utils.h             # GL framebuffers
├── nanovg_mtl.h/m                # Metal (macOS/iOS only)
├── nanovg_d3d11.h/c              # DX11 (Windows only)
└── mulle-nanovg.h                # Convenience header
```

## Build Strategy

**CMake conditionals**:
```cmake
# Always compile
add_sources(nanovg.c nanovg-trace-functions.c)

if(APPLE)
  add_sources(nanovg_mtl.m)
  link_frameworks(Metal QuartzCore Foundation)
endif()

if(WIN32)
  add_sources(nanovg_d3d11.c)
  link_libraries(d3d11 dxgi)
endif()
```

## Open Questions

1. **DX11 framebuffers**: Does DX11 backend support framebuffers? Need to check.
2. **OpenGL variants**: Include GL2, GL3, GLES2, GLES3? (Probably all)
3. **nanovg.c duplication**: DX11 includes nanovg.c - will this cause issues?

## Next Actions

When ready to implement:
1. Copy core nanovg files (Phase 1)
2. Copy OpenGL backend (Phase 2)
3. Copy Metal backend (Phase 3)
4. Copy/refactor DX11 backend (Phase 4)
5. Create mulle-nanovg.h (Phase 5)
6. Configure CMake
7. Test on each platform

See **MATCHPLAN.md** for detailed step-by-step instructions.
