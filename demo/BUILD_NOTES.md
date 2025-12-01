# Demo Build Notes

## Status
✅ Code compiles successfully
⚠️ Linker needs frameworks

## Missing Frameworks

The demo needs these macOS frameworks linked:
- `CoreGraphics.framework` (for GLFW display functions)
- `Cocoa.framework` (for NSWindow/NSView)
- `Metal.framework` (for Metal rendering)
- `QuartzCore.framework` (for CAMetalLayer)
- `IOKit.framework` (for GLFW)
- `CoreVideo.framework` (for GLFW)

## CMake Fix Needed

Add to demo's CMakeLists.txt:

```cmake
if(APPLE)
  target_link_libraries(demo
    "-framework Cocoa"
    "-framework Metal"
    "-framework QuartzCore"
    "-framework CoreGraphics"
    "-framework IOKit"
    "-framework CoreVideo"
  )
endif()
```

## What the Demo Does

1. Creates GLFW window (no OpenGL context)
2. Gets the NSView from GLFW
3. Creates CAMetalLayer and attaches to view
4. Creates NanoVG Metal context
5. Loads demo data (fonts, images)
6. Render loop:
   - Updates Metal layer size
   - Calls `nvgBeginFrame()`
   - Calls `renderDemo()` with all the UI widgets
   - Calls `nvgEndFrame()`
7. Cleanup

## Running

Once frameworks are linked:
```bash
mulle-sde run demo
```

Should show a window with the full NanoVG demo UI!
