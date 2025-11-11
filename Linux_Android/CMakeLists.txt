#   SPDX-License-Identifier: Apache-2.0
#
#   Copyright (C) 2025 MD S M Sarowar Hossain
#
#


cmake_minimum_required(VERSION 3.18)
project(AndroidSDLTest LANGUAGES C CXX)

# Ensure NDK environment variable is set
if (NOT DEFINED ENV{ANDROID_NDK})
  message(FATAL_ERROR "Please export ANDROID_NDK to your NDK install (e.g. export ANDROID_NDK=~/android-ndk-r27c)")
endif()
set(CMAKE_TOOLCHAIN_FILE "$ENV{ANDROID_NDK}/build/cmake/android.toolchain.cmake" CACHE FILE "Android NDK toolchain")

if (NOT DEFINED CMAKE_ANDROID_ARCH_ABI)
    message(WARNING "CMAKE_ANDROID_ARCH_ABI is not defined. Ensure -DANDROID_ABI is passed to CMake during configuration.")
endif()
message(STATUS "CMake: Building for ABI: ${CMAKE_ANDROID_ARCH_ABI}")

set(ANDROID_PLATFORM 21)        # Minimum Android API
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/${CMAKE_ANDROID_ARCH_ABI}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/${CMAKE_ANDROID_ARCH_ABI}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/${CMAKE_ANDROID_ARCH_ABI}")

# SDL3 prebuilt setup
set(SDL3_PREBUILT_ROOT "${CMAKE_SOURCE_DIR}/jni/SDL3" CACHE PATH "Root directory of prebuilt SDL3 libraries")
set(SDL3_INCLUDE_DIR   "${SDL3_PREBUILT_ROOT}/include")
set(SDL3_LIBRARY_FILE  "${SDL3_PREBUILT_ROOT}/lib/android.${CMAKE_ANDROID_ARCH_ABI}/libSDL3.so")

# SDL3_Image prebuilt setup 
set(SDL3_image_INCLUDE_DIR   "${SDL3_PREBUILT_ROOT}/SDL3_image-3.2.4")
set(SDL3_image_LIBRARY_FILE  "${SDL3_PREBUILT_ROOT}/SDL3_image-3.2.4/libs/android.${CMAKE_ANDROID_ARCH_ABI}/libSDL3_image.so")
# set(SDL3MAIN_LIBRARY "")

message(STATUS "CMake: SDL3 include dir: ${SDL3_INCLUDE_DIR}")
message(STATUS "CMake: SDL3 library file: ${SDL3_LIBRARY_FILE}")

message(STATUS "CMake: SDL3_image include dir: ${SDL3_image_INCLUDE_DIR}")
message(STATUS "CMake: SDL3_image library file: ${SDL3_image_LIBRARY_FILE}")

if (NOT EXISTS "${SDL3_INCLUDE_DIR}/SDL3/SDL.h")
  message(FATAL_ERROR "CMake: SDL3 headers not found in ${SDL3_INCLUDE_DIR}/SDL3")
endif()
if (NOT EXISTS "${SDL3_INCLUDE_DIR}/SDL3/SDL_iostream.h")
  message(FATAL_ERROR "CMake: SDL3_iostream header not found in ${SDL3_INCLUDE_DIR}/SDL3")
endif()
if (NOT EXISTS "${SDL3_LIBRARY_FILE}")
  message(FATAL_ERROR "CMake: libSDL3.so not found at ${SDL3_LIBRARY_FILE}")
endif()

if (NOT EXISTS "${SDL3_image_INCLUDE_DIR}/include/SDL3_image/SDL_image.h")
  message(FATAL_ERROR "CMake: SDL3_image headers not found in ${SDL3_image_INCLUDE_DIR}/include/SDL3_image/")
endif()
if (NOT EXISTS "${SDL3_image_LIBRARY_FILE}")
  message(FATAL_ERROR "CMake: libSDL3_image.so not found at ${SDL3_image_LIBRARY_FILE}")
endif()

add_library(SDL3::SDL3 SHARED IMPORTED)
set_target_properties(SDL3::SDL3 PROPERTIES
  IMPORTED_LOCATION "${SDL3_LIBRARY_FILE}"
  INTERFACE_INCLUDE_DIRECTORIES "${SDL3_INCLUDE_DIR}"
)

# — SDL3_ttf prebuilt —  
set(SDL3TTF_INCLUDE_DIR   "${SDL3_PREBUILT_ROOT}/include/SDL3_ttf")
set(SDL3TTF_LIBRARY_FILE  "${SDL3_PREBUILT_ROOT}/SDL3_ttf/libs/android.${CMAKE_ANDROID_ARCH_ABI}/libSDL3_ttf.so")

add_library(SDL3_ttf::SDL3_ttf SHARED IMPORTED)
set_target_properties(SDL3_ttf::SDL3_ttf PROPERTIES
  IMPORTED_LOCATION           "${SDL3TTF_LIBRARY_FILE}"
  INTERFACE_INCLUDE_DIRECTORIES "${SDL3TTF_INCLUDE_DIR}"
)

# — SDL3_image prebuilt —
add_library(SDL3_image::SDL3_image SHARED IMPORTED)
set_target_properties(SDL3_image::SDL3_image PROPERTIES
  IMPORTED_LOCATION            "${SDL3_image_LIBRARY_FILE}"
  INTERFACE_INCLUDE_DIRECTORIES "${SDL3_image_INCLUDE_DIR}/include"
)

# Your app's native library
add_library(main SHARED 
  jni/src/main.cpp
  jni/src/Anchor.cpp
  jni/src/Button.cpp
  jni/src/CheckBox.cpp
  jni/src/Dropdown.cpp
  jni/src/Image.cpp
  jni/src/InputBox.cpp
  jni/src/Label.cpp
  jni/src/RadioButton.cpp
  jni/src/ScrollView.cpp
  jni/src/Shape.cpp
  jni/src/Slider.cpp
  jni/src/Switch.cpp
  jni/src/TextRenderer.cpp
  jni/src/WindowUtil.cpp
  
  

  
  
 
 

)

target_include_directories(main PRIVATE
  "${SDL3_INCLUDE_DIR}"
  "${SDL3_image_INCLUDE_DIR}/include"
  "${CMAKE_SOURCE_DIR}/jni/src"
)

target_link_libraries(main PRIVATE
  SDL3::SDL3
  SDL3_ttf::SDL3_ttf
  SDL3_image::SDL3_image
  log
  android
  EGL
  GLESv2      
  OpenSLES
)

# --- Optimizations for 'main' library (Release builds) ---
target_compile_options(main PRIVATE
    $<$<CONFIG:Release>:-Os>                 # Optimize for size
    $<$<CONFIG:Release>:-flto>               # LTO (compile)
    $<$<CONFIG:Release>:-fvisibility=hidden> # Hide symbols by default
    $<$<CONFIG:Release>:-ffunction-sections> # Separate functions for GC
    $<$<CONFIG:Release>:-fdata-sections>     # Separate data for GC
)

target_link_options(main PRIVATE
    $<$<CONFIG:Release>:-flto>                # LTO (link)
    $<$<CONFIG:Release>:-Wl,--gc-sections>    # Remove unreferenced sections
)

# --- Stripping for 'main' library (Release builds) ---
if(CMAKE_STRIP AND CMAKE_BUILD_TYPE STREQUAL "Release")
  add_custom_command(TARGET main POST_BUILD
    COMMAND ${CMAKE_STRIP} --strip-unneeded $<TARGET_FILE:main>
    COMMENT "Stripping unneeded symbols from $<TARGET_FILE_NAME:main>"
    VERBATIM
  )
elseif(NOT CMAKE_STRIP AND CMAKE_BUILD_TYPE STREQUAL "Release")
  message(WARNING "CMAKE_STRIP is not defined for Release build. 'main' library will not be stripped.")
endif()





