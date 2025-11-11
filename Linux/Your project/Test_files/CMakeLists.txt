#
# CMakeLists.txt - Build configuration script for the XenUI executable.
#
#
#
#   SPDX-License-Identifier: Apache-2.0
#
#   Copyright (C) 2025 MD S M Sarowar Hossain
#
#
#
#
# This file configures the build for a test executable that links against the
# XenUI framework and its bundled dependencies (SDL3 components).
# It assumes a specific directory structure relative to the XenUI repository root.
#

# Specify the minimum required version of CMake.
cmake_minimum_required(VERSION 3.10)
# Define the project name.
project(XenUI)

# Enforce C++20 standard compliance for the test application.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Set a reasonable default build type if none is specified by the user.
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Configure AddressSanitizer (ASAN) support only for Debug builds.
# This aids in dynamic memory error detection during development and testing.
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  message(STATUS "Enabling AddressSanitizer (Debug build)")
  # Append ASAN compilation flags for the C++ compiler.
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g")
  # Append ASAN linking flags for the linker.
  set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")
endif()


# --------------------------------------------------------------------------------
# 1. Locate the XenUI Framework Root
#
#    This section defines the paths required to locate the XenUI headers and
#    the compiled shared library, relative to the current example directory.
# --------------------------------------------------------------------------------
# Define the path to the root of the XenUI source tree, assuming two levels up
# from the current directory (e.g., from 'examples/Test_files/' to '/').
set(XENUI_ROOT       "${CMAKE_SOURCE_DIR}/../..")

# Define the directory containing XenUI's public headers. This is the parent
# directory of the "XenUI/" folder (e.g., <XENUI_ROOT>/include).
set(XENUI_INCLUDE_DIR "${XENUI_ROOT}/include")

# Define the directory where the built XenUI shared library file (libXenUI.so/.dll) is located.
# The primary assumption here is that the library is found directly under the 'build' folder.
set(XENUI_LIB_DIR     "${XENUI_ROOT}/build")

# --------------------------------------------------------------------------------
# 2. Locate SDL3 Dependencies
#
#    Define root paths for the bundled SDL libraries relative to the XenUI root,
#    mirroring the structure defined in the main XenUI CMakeLists.txt.
# --------------------------------------------------------------------------------
# Define the root path for SDL3.
set(SDL3_ROOT        "${CMAKE_SOURCE_DIR}/../../tools/SDL3-3.2.16")
# Define the root path for SDL3_ttf.
set(SDL3_TTF_ROOT    "${CMAKE_SOURCE_DIR}/../../tools/SDL3_ttf-3.2.2")
# Define the root path for SDL3_image.
set(SDL3_IMAGE_ROOT  "${CMAKE_SOURCE_DIR}/../../tools/SDL3_image-3.2.4")

# --------------------------------------------------------------------------------
# 3. Configure Include Directories (Headers)
#
#    Add all necessary header paths to the compiler's search list.
# --------------------------------------------------------------------------------
include_directories(
    ${XENUI_INCLUDE_DIR}                 # Allows inclusion of XenUI public headers (e.g., #include <XenUI/Header.h>)
    ${SDL3_ROOT}/include/SDL3            # For standard SDL3 headers
    ${SDL3_TTF_ROOT}/include/SDL3_ttf    # For SDL3_ttf extension headers
    ${SDL3_IMAGE_ROOT}/include/SDL3_image # For SDL3_image extension headers
)

# --------------------------------------------------------------------------------
# 4. Configure Link Directories (Libraries)
#
#    Add directories containing the compiled shared or static libraries to the
#    linker's search list so the -l flags can locate the files.
# --------------------------------------------------------------------------------
link_directories(
    ${XENUI_LIB_DIR}         # Location for the XenUI framework library
    ${SDL3_ROOT}/build       # Location for the core SDL3 library
    ${SDL3_TTF_ROOT}/build   # Location for the SDL3_ttf library
    ${SDL3_IMAGE_ROOT}/build # Location for the SDL3_image library
)

# --------------------------------------------------------------------------------
# 5. Define the Test Executable
# --------------------------------------------------------------------------------
# Create the executable target named 'test' from the specified source file.
add_executable(test linux.cpp)

# --------------------------------------------------------------------------------
# 6. Link Libraries
#
#    Link the 'test' executable against all runtime dependencies.
# --------------------------------------------------------------------------------
target_link_libraries(test
    # Link against the XenUI framework library
    XenUI
    # Link against core SDL3
    SDL3
    # Link against SDL3 extension for font rendering
    SDL3_ttf
    # Link against SDL3 extension for image handling
    SDL3_image
    # Link against system-installed OpenGL
    GL
)
