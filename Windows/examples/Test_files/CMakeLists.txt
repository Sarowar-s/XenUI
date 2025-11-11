# SPDX-License-Identifier: Apache-2.0
#
#  Copyright (C) 2025 MD S M Sarowar Hossain
#
#
#
#
# Description:
# Configuration file for building the 'xenui_test' executable. This project
# consumes the XenUI shared library and relies on local, relative paths
# to find the XenUI build directory and its dependencies (SDL3, SDL3_ttf, SDL3_image).
#
# NOTE: This configuration avoids deprecated commands like link_directories()
# and include_directories() in favor of modern target properties.
#

cmake_minimum_required(VERSION 3.10)
project(TestXenUI VERSION 0.9.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --------------------------------------------------------------------------------
# 1. Setup Paths and Find Libraries
#
#    Defines the relative paths to the built XenUI library and its bundled
#    SDL dependencies for a non-installed, local development environment.
# --------------------------------------------------------------------------------
# Define the root of the XenUI source/build and SDL toolchains.
set(XENUI_ROOT 	 	 "${CMAKE_SOURCE_DIR}/../..")
set(SDL3_ROOT 	 	 "${CMAKE_SOURCE_DIR}/../../tools/SDL3-3.2.16")
set(SDL3_TTF_ROOT 	 "${CMAKE_SOURCE_DIR}/../../tools/SDL3_ttf-3.2.2")
set(SDL3_IMAGE_ROOT 	 "${CMAKE_SOURCE_DIR}/../../tools/SDL3_image-3.2.4")

# Manually find the XenUI library's full path. This replaces the need for 'link_directories'.
find_library(
    XENUI_LIBRARY
    NAMES XenUI libXenUI
    HINTS "${XENUI_ROOT}/build"
    REQUIRED # Ensure the library is found
)
message(STATUS "Found XenUI Library: ${XENUI_LIBRARY}")

# Manually find the full path for required SDL libraries.
# Since SDL libraries were linked PRIVATE in XenUI, the consumer must link them explicitly.
find_library(
    SDL3_LIB
    NAMES SDL3
    HINTS "${SDL3_ROOT}/lib/x64" "${SDL3_ROOT}/build"
    REQUIRED
)
find_library(
    SDL3_TTF_LIB
    NAMES SDL3_ttf
    HINTS "${SDL3_TTF_ROOT}/lib/x64" "${SDL3_TTF_ROOT}/build"
    REQUIRED
)
find_library(
    SDL3_IMAGE_LIB
    NAMES SDL3_image
    HINTS "${SDL3_IMAGE_ROOT}/lib/x64" "${SDL3_IMAGE_ROOT}/build"
    REQUIRED
)


# --------------------------------------------------------------------------------
# 2. Build the Executable
# --------------------------------------------------------------------------------
# Add the executable target, compiling the source files.
add_executable(xenui_test windows.cpp app_icon.rc)

# --------------------------------------------------------------------------------
# 3. Define Target Properties: Includes and Linking
#
#    Uses modern CMake commands to define dependencies.
# --------------------------------------------------------------------------------

# A. Specify required include directories for the executable.
# This replaces the deprecated 'include_directories()' command.
target_include_directories(xenui_test PRIVATE
    # XenUI public headers
    "${XENUI_ROOT}/include"
    # SDL dependencies headers (needed if XenUI's public API exposes SDL types)
    "${SDL3_ROOT}/include"
    "${SDL3_ROOT}/include/SDL3"
    "${SDL3_TTF_ROOT}/include"
    "${SDL3_TTF_ROOT}/include/SDL3_ttf"
    "${SDL3_IMAGE_ROOT}/include"
    "${SDL3_IMAGE_ROOT}/include/SDL3_image"
)

# B. Link the executable against the required libraries.
target_link_libraries(xenui_test PRIVATE
    ${XENUI_LIBRARY}        # The XenUI shared library (pulls in GLM/OpenGL headers automatically)
    ${SDL3_LIB}             # SDL3 Core runtime
    ${SDL3_TTF_LIB}         # SDL3 TTF (Font handling)
    ${SDL3_IMAGE_LIB}       # SDL3 Image (Texture/Image loading)
    opengl32                # System OpenGL library
)


# --------------------------------------------------------------------------------
# 4. Windows-specific Configuration
# --------------------------------------------------------------------------------
if (WIN32)
    # Configure the executable as a Windows GUI application (no console window).
    # This is the preferred modern way to achieve the linker flag '-mwindows'.
    set_target_properties(xenui_test PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
    # The original file used this for MinGW/GCC compatibility:
    # set_target_properties(xenui_test PROPERTIES LINK_FLAGS "-mwindows")
endif()


# --------------------------------------------------------------------------------
# 5. Post-build: Copy Runtime DLLs
#
#    This step is crucial for Windows. It copies all necessary shared libraries
#    (DLLs) into the executable's output directory so the application can run.
# --------------------------------------------------------------------------------
# Path to MinGW runtime DLLs (update this path as needed for your specific setup).
set(MINGW_DLL_DIR "C:/msys64/mingw64/bin")

add_custom_command(TARGET xenui_test POST_BUILD
    # Copy the built XenUI DLL (assuming it's named XenUI.dll in the build folder)
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${XENUI_ROOT}/build/libXenUI.dll" "$<TARGET_FILE_DIR:xenui_test>"
    # Copy all necessary SDL runtime DLLs (assuming they are in the specified build folders)
	COMMAND ${CMAKE_COMMAND} -E copy_if_different "${SDL3_ROOT}/build/SDL3.dll" "$<TARGET_FILE_DIR:xenui_test>"
	COMMAND ${CMAKE_COMMAND} -E copy_if_different "${SDL3_TTF_ROOT}/build/SDL3_ttf.dll" "$<TARGET_FILE_DIR:xenui_test>"
	COMMAND ${CMAKE_COMMAND} -E copy_if_different "${SDL3_IMAGE_ROOT}/build/SDL3_image.dll" "$<TARGET_FILE_DIR:xenui_test>"

    # Copy MinGW runtime DLLs (essential for MinGW/GCC builds on Windows)
	COMMAND ${CMAKE_COMMAND} -E copy_if_different "${MINGW_DLL_DIR}/libgcc_s_seh-1.dll" "$<TARGET_FILE_DIR:xenui_test>"
	COMMAND ${CMAKE_COMMAND} -E copy_if_different "${MINGW_DLL_DIR}/libstdc++-6.dll" "$<TARGET_FILE_DIR:xenui_test>"
	COMMAND ${CMAKE_COMMAND} -E copy_if_different "${MINGW_DLL_DIR}/libwinpthread-1.dll" "$<TARGET_FILE_DIR:xenui_test>"

	COMMENT "Copying required DLLs for runtime execution."
)

















