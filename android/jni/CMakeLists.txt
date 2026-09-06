cmake_minimum_required( VERSION 3.13...99.99)

project( nanovg-android-demo C)

# Build for Android: pass the NDK toolchain file from build.sh, e.g.
#   -DCMAKE_TOOLCHAIN_FILE=${ANDROID_NDK_HOME}/build/cmake/android.toolchain.cmake
#   -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-21

set( CMAKE_C_STANDARD 99)
set( CMAKE_C_STANDARD_REQUIRED ON)

# Fonts use freetype, the mulle-nanovg default: fontstash.h picks
# FONS_USE_FREETYPE when neither it nor FONS_USE_STB_TRUETYPE is defined,
# exactly like the desktop build. stb_truetype is no longer used on Android.

set( NANOVG_SRC  "${CMAKE_CURRENT_SOURCE_DIR}/../../src")
set( DEMO_SRC    "${CMAKE_CURRENT_SOURCE_DIR}/../../demo/src")

# Third-party static libs, cross-compiled per ABI by android/build-libs.sh.
# build.sh passes the per-ABI prefix dirs on the cmake command line.
set( FREETYPE_DIR "" CACHE PATH "prefix dir of cross-compiled freetype (third_party/prefix/freetype/<abi>)")
set( HARFBUZZ_DIR "" CACHE PATH "prefix dir of cross-compiled harfbuzz (third_party/prefix/harfbuzz/<abi>)")

# android_native_app_glue (ships with the NDK)
if( NOT DEFINED ANDROID_NDK_HOME)
   set( ANDROID_NDK_HOME "$ENV{ANDROID_NDK_HOME}")
endif()
set( ANDROID_GLUE "${ANDROID_NDK_HOME}/sources/android/native_app_glue")

add_library( native_app_glue STATIC "${ANDROID_GLUE}/android_native_app_glue.c")
target_include_directories( native_app_glue PUBLIC "${ANDROID_GLUE}")

# mulle-nanovg core (backend-agnostic) + GLES3 backend implementation
add_library( nanovg STATIC
   "${NANOVG_SRC}/nanovg.c"
   "${NANOVG_SRC}/nanovg-trace-functions.c"
)
target_include_directories( nanovg PUBLIC "${NANOVG_SRC}")
# freetype headers (ft2build.h) for the fontstash implementation in nanovg.c
if( FREETYPE_DIR)
   target_include_directories( nanovg PUBLIC "${FREETYPE_DIR}/include/freetype2")
endif()

# The demo executable (port of main-linux-gles.c + shared demo.c)
add_library( nanovg-demo SHARED
   main-android.c
   android_assets.c
   "${DEMO_SRC}/demo.c"
)
target_include_directories( nanovg-demo PRIVATE
   "${CMAKE_CURRENT_SOURCE_DIR}"
   "${DEMO_SRC}"
)
target_compile_definitions( nanovg-demo PRIVATE NVG_ANDROID)

set( THIRD_PARTY_LIBS "")
if( FREETYPE_DIR AND HARFBUZZ_DIR)
   list( APPEND THIRD_PARTY_LIBS
      "${FREETYPE_DIR}/lib/libfreetype.a"
      "${HARFBUZZ_DIR}/lib/libharfbuzz.a"
   )
endif()

target_link_libraries( nanovg-demo
   native_app_glue
   nanovg
   ${THIRD_PARTY_LIBS}
   android
   EGL
   GLESv3
   log
   m
)
