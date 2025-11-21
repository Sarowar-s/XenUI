
#   SPDX-License-Identifier: Apache-2.0
#
#   Copyright (C) 2025 MD S M Sarowar Hossain
#
#





# Minimum required CMake version
cmake_minimum_required(VERSION 3.22...3.27)

# Project name and language
project("XenonUIApp" LANGUAGES CXX)

# -----------------------------------------------------------------------------
# Pick up ABI from the command line or environment, else default to arm64-v8a
if(NOT DEFINED ANDROID_ABI)
    if(DEFINED ENV{ANDROID_ABI})
        set(ANDROID_ABI $ENV{ANDROID_ABI})
    else()
        set(ANDROID_ABI "arm64-v8a")
    endif()
endif()

# Minimum API level 
set(ANDROID_NATIVE_API_LEVEL 21)
# -----------------------------------------------------------------------------

# Paths to SDL3
set(SDL3_PATH "${CMAKE_SOURCE_DIR}/lib")

# Headers path :
#   sdl3_include/include
#   sdl3_image/include
#   sdl3_ttf/include
set(SDL3_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/sdl3_include/include")
set(SDL3_IMAGE_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/sdl3_image/include")
set(SDL3_TTF_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/sdl3_ttf/include")

# Add include directories
include_directories(
    ${SDL3_INCLUDE_DIR}
    ${SDL3_IMAGE_INCLUDE_DIR}
    ${SDL3_TTF_INCLUDE_DIR}
)

# Shared library target
add_library(main SHARED
    ../src/Anchor.cpp
    ../src/Button.cpp
    ../src/CheckBox.cpp
    ../src/Dropdown.cpp
    ../src/Image.cpp
    ../src/InputBox.cpp
    ../src/Label.cpp
    src/main/cpp/main.cpp
    ../src/RadioButton.cpp
    ../src/ScrollView.cpp
    ../src/Shape.cpp    
    ../src/Slider.cpp    
    ../src/Switch.cpp    
    src/main/cpp/TextRenderer.cpp
    ../src/WindowUtil.cpp
)

# Properties
set_target_properties(main PROPERTIES
    OUTPUT_NAME "main"
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED TRUE
)

# Link libraries
target_link_libraries(main
    # SDL3 core library
    ${SDL3_PATH}/android/android.${ANDROID_ABI}/libSDL3.so

    # SDL3_image and SDL3_ttf prebuilt libraries
    ${SDL3_PATH}/SDL3_image/libs/android.${ANDROID_ABI}/libSDL3_image.so
    ${SDL3_PATH}/SDL3_ttf/libs/android.${ANDROID_ABI}/libSDL3_ttf.so

    # Android native libraries
    android log GLESv2
)


