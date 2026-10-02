#
# You rarely need to edit this file. To add or remove files in your
# project edit the "src" folders contents and run `mulle-sde reflect`
#
#
# A convenient way to add a new source file is:
#    mulle-sde add src/foo.c
#
cmake_minimum_required( VERSION 3.13...99.99)

project( mulle-nanovg VERSION 0.1.1 LANGUAGES C)


### mulle-sde environment

# add cmake module paths to search path
list( INSERT CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/cmake/share")
list( INSERT CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/cmake/reflect")
list( INSERT CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include( Environment)

include( Files)


### Platform-specific backend sources
#
# The reflected source variables (see cmake/reflect/_Sources.cmake) keep the
# portable code in SOURCES and route backend-specific sources into their own
# variables via patternfiles. This is "set and forget": a new Darwin source is
# any *.m file, a new Windows source is any *_d3d11.c / *_dx11.c, etc. Just add
# the file and `mulle-sde reflect` — no edits needed here.
#
#   DARWIN_SOURCES   : *.m          -> Metal backend (nanovg_mtl.m)
#   WINDOWS_SOURCES  : *_d3d11.c    -> DirectX 11 backend (nanovg_d3d11.c)
#   VULKAN_SOURCES   : *_vk.c       -> Vulkan backend (dormant; header-only today)
#
if( APPLE)
   list( APPEND SOURCES ${DARWIN_SOURCES})
endif()

if( WIN32)
   list( APPEND SOURCES ${WINDOWS_SOURCES})
endif()

option( NANOVG_VULKAN "Build the Vulkan backend sources" OFF)
if( NANOVG_VULKAN)
   list( APPEND SOURCES ${VULKAN_SOURCES})
endif()


### Library

#
# Backend public headers are classified into per-backend variables by
# patternfiles (see cmake/reflect/_Headers.cmake):
#
#   GL_HEADERS      : *_gl.h  *_gl_*.h      -> OpenGL / GLES
#   RLGL_HEADERS    : *_rlgl.h rlgl.h ...   -> rlgl (raylib GL abstraction)
#   METAL_HEADERS   : *_mtl.h               -> Metal (Darwin)
#   D3D11_HEADERS   : *_d3d11.h *_dx11.h    -> DirectX 11 (Windows)
#   VULKAN_HEADERS  : *_vk.h  *_vk_*.h      -> Vulkan
#
# Today we install *all* backend headers on every platform (current behaviour).
# To gate installation per platform in the future, move the relevant lines into
# the matching if( APPLE)/if( WIN32)/option() branches below — the variables are
# already set and forget, so no patternfile or reflect changes are needed.
#
set( INSTALL_BACKEND_HEADERS
   ${GL_HEADERS}
   ${RLGL_HEADERS}
   ${METAL_HEADERS}
   ${D3D11_HEADERS}
   ${VULKAN_HEADERS}
)

# FUTURE: platform-gated install would look like:
#   if( APPLE)
#      list( APPEND INSTALL_BACKEND_HEADERS ${METAL_HEADERS})
#   elseif( WIN32)
#      list( APPEND INSTALL_BACKEND_HEADERS ${D3D11_HEADERS})
#   endif()
#   # GL / RLGL / VULKAN are portable and would stay unconditional.

if( HEADERS_PHASE)
   install( FILES ${INSTALL_PUBLIC_HEADERS}    DESTINATION "include/mulle-nanovg")
   install( FILES ${INSTALL_BACKEND_HEADERS}   DESTINATION "include/mulle-nanovg")
   install( FILES ${INSTALL_PRIVATE_HEADERS}   DESTINATION "include/mulle-nanovg")
   install( FILES ${INSTALL_CMAKE_INCLUDES}    DESTINATION "include/mulle-nanovg/cmake")

   # short cut out
   if( NOT COMPILE_PHASE AND NOT LINK_PHASE)
      return()
   endif()
endif()

if( LINK_PHASE)
   include( Dependencies OPTIONAL)
endif()

include( Library)

# use LIBRARY_COMPILE_TARGET for compiler options
target_compile_definitions( "${LIBRARY_COMPILE_TARGET}" PUBLIC)
target_compile_options( "${LIBRARY_COMPILE_TARGET}" PUBLIC)
if( NOT MSVC)
   target_compile_options( "${LIBRARY_COMPILE_TARGET}" PUBLIC -Wunused)
endif()

# use LIBRARY_LINK_TARGET for linker options
if( LINK_PHASE)
   target_link_options( "${LIBRARY_LINK_TARGET}" PUBLIC)
endif()

include( InstallLibrary)

if( LINK_PHASE)
   include( AddSubdirectorySupport)
   include( FindPackageSupport)
endif()

include( FinalOutput OPTIONAL)
