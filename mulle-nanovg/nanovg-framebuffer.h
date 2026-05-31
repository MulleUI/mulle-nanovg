//
// nanovg-framebuffer.h - Base framebuffer type definition
//

#ifndef NANOVG_FRAMEBUFFER_H
#define NANOVG_FRAMEBUFFER_H

// Unified framebuffer type - opaque handle used by all backends
// Backends cast this to their specific types internally
typedef void* NVGframebuffer;

#endif // NANOVG_FRAMEBUFFER_H
