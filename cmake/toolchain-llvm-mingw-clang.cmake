# Toolchain file for cross-compiling to Windows via llvm-mingw + Clang

# The target system
set(CMAKE_SYSTEM_NAME Windows)
# You may change this to i686, aarch64, etc
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# Root of your llvm-mingw installation
# Adjust this path to your installation
# Hardcoded default
set(DEFAULT_LLVM_MINGW_ROOT "/opt/mulle-clang-project-windows/21.1.4")

# Check for cache variable override
if(NOT DEFINED LLVM_MINGW_ROOT)
    if(DEFINED ENV{LLVM_MINGW_ROOT})
        set(LLVM_MINGW_ROOT $ENV{LLVM_MINGW_ROOT})
    elseif(EXISTS "${DEFAULT_LLVM_MINGW_ROOT}")
        set(LLVM_MINGW_ROOT "${DEFAULT_LLVM_MINGW_ROOT}")
    else()
        # Auto-detect the highest version under /opt/mulle-clang-project-windows
        file(GLOB ALL_LLVM_MINGW "/opt/mulle-clang-project-windows/*")
        set(HIGHEST_VERSION "")
        foreach(dir ${ALL_LLVM_MINGW})
            if(IS_DIRECTORY "${dir}")
                get_filename_component(basename "${dir}" NAME)
                if(basename MATCHES "^[0-9]+\\.[0-9]+\\.[0-9]+$")
                    if(HIGHEST_VERSION STREQUAL "" OR basename VERSION_GREATER HIGHEST_VERSION)
                        set(HIGHEST_VERSION "${basename}")
                    endif()
                endif()
            endif()
        endforeach()
        if(HIGHEST_VERSION STREQUAL "")
            message(FATAL_ERROR "Could not detect LLVM-Mingw installation. Set -DLLVM_MINGW_ROOT or LLVM_MINGW_ROOT environment variable.")
        endif()
        set(LLVM_MINGW_ROOT "/opt/mulle-clang-project-windows/${HIGHEST_VERSION}")
    endif()
endif()

message(STATUS "Using LLVM-Mingw root: ${LLVM_MINGW_ROOT}")

# Target triplet
set(TRIPLET x86_64-w64-mingw32)

# Compilers
set(CMAKE_C_COMPILER   ${LLVM_MINGW_ROOT}/bin/${TRIPLET}-clang)
set(CMAKE_CXX_COMPILER ${LLVM_MINGW_ROOT}/bin/${TRIPLET}-clang++)
set(CMAKE_RC_COMPILER  ${LLVM_MINGW_ROOT}/bin/${TRIPLET}-windres)

# Root path for finding target libs/includes
set(CMAKE_FIND_ROOT_PATH
    ${LLVM_MINGW_ROOT}
    ${LLVM_MINGW_ROOT}/generic-w64-mingw32
)

# Search modes: programs on host, libs/includes on target
# we also have programs in DEPENDENCY though, so we search both
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)

set( BUILD_SHARED_LIBS OFF CACHE BOOL "Build static libraries for Windows by default too")
set( CMAKE_STATIC_LIBRARY_SUFFIX ".a" CACHE STRING "Build and search static libraries with .a")

# Optional flags
set(CMAKE_CXX_FLAGS_INIT   "-fuse-ld=lld")
set(CMAKE_EXE_LINKER_FLAGS_INIT "-static")
