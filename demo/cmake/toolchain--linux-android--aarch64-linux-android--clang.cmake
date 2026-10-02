# Toolchain file for cross-compiling to Android via the Android NDK.
#
# EXPERIMENTAL / REPO-LOCAL: Android is not officially supported by mulle-sde
# yet. This file lives in the project's cmake/ (the editable, project-specific
# location) on purpose, so that `mulle-sde upgrade` does not overwrite it and
# so it can be iterated on without touching any dependency.
#
# Configure with:
#   mulle-sde platform add android
#   mulle-sde platform set android root /path/to/ndk
#   mulle-sde craft --platform android
#
# The NDK root is picked up from (in order):
#   - the CMake cache variable ANDROID_NDK
#   - the environment variable ANDROID_NDK
#   - MULLE_CRAFT_CROSS_COMPILER_ROOT__ANDROID (set by `platform set <p> root`)
#
# ANDROID_ABI and ANDROID_PLATFORM may be supplied through the environment or
# as CMake cache variables. Defaults target current 64-bit Android devices.
#
# Use a MODERN NDK (r29 / clang 21 or newer). Older NDKs (r25 / clang 14)
# reject the integer->_Atomic(pointer) static initializers used in mulle-core.

set(CMAKE_SYSTEM_NAME Android)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

if(NOT DEFINED ANDROID_NDK OR "${ANDROID_NDK}" STREQUAL "")
   if(DEFINED ENV{ANDROID_NDK})
      set(ANDROID_NDK "$ENV{ANDROID_NDK}" CACHE PATH "Android NDK root")
   elseif(DEFINED ENV{MULLE_CRAFT_CROSS_COMPILER_ROOT__ANDROID})
      set(ANDROID_NDK "$ENV{MULLE_CRAFT_CROSS_COMPILER_ROOT__ANDROID}" CACHE PATH "Android NDK root")
   else()
      message(FATAL_ERROR "ANDROID_NDK is required for the Android toolchain")
   endif()
endif()

if(NOT EXISTS "${ANDROID_NDK}/build/cmake/android.toolchain.cmake")
   message(FATAL_ERROR "Invalid Android NDK: ${ANDROID_NDK}")
endif()

# Propagate the Android knobs into CMake's try_compile sub-builds, otherwise a
# -DANDROID_NDK=... (rather than the environment) is lost during compiler
# detection and the toolchain re-runs without a usable NDK root.
list(APPEND CMAKE_TRY_COMPILE_PLATFORM_VARIABLES
     ANDROID_NDK ANDROID_ABI ANDROID_PLATFORM)

set(ANDROID_ABI "${ANDROID_ABI}" CACHE STRING "Android ABI")
if("${ANDROID_ABI}" STREQUAL "")
   set(ANDROID_ABI arm64-v8a CACHE STRING "Android ABI" FORCE)
endif()

set(ANDROID_PLATFORM "${ANDROID_PLATFORM}" CACHE STRING "Android API level")
if("${ANDROID_PLATFORM}" STREQUAL "")
   set(ANDROID_PLATFORM android-24 CACHE STRING "Android API level" FORCE)
endif()

message(STATUS "Using Android NDK: ${ANDROID_NDK}")
message(STATUS "Android ABI: ${ANDROID_ABI}")
message(STATUS "Android platform: ${ANDROID_PLATFORM}")

include("${ANDROID_NDK}/build/cmake/android.toolchain.cmake")

# The NDK toolchain sets CMAKE_FIND_ROOT_PATH_MODE_LIBRARY/INCLUDE to ONLY,
# which confines find_library()/find_path() to the NDK sysroot. But mulle-sde
# builds dependencies (freetype, harfbuzz, SDL3, ...) into the DEPENDENCY dir,
# which lives *outside* the sysroot and is injected via CMAKE_LIBRARY_PATH /
# -isystem. Switch the find modes to BOTH so those mulle-built libs and headers
# are discoverable, exactly like the llvm-mingw (windows) cross toolchain does.
#
# Must come AFTER the NDK include so it overrides the NDK's ONLY defaults.
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)
