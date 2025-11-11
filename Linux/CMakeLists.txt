#
#   SPDX-License-Identifier: Apache-2.0
#
#   Copyright (C) 2025 MD S M Sarowar Hossain
#
# This file defines the compilation process for the XenUI shared library,
# manages external dependencies (SDL3, OpenGL, Freetype, etc.), handles custom
# folder layouts for bundled dependencies, and specifies installation rules
# for the framework components.
#

# Specify the minimum required version of CMake to ensure compatibility with modern features.
cmake_minimum_required(VERSION 3.10)
# Define the project name and version, specifying C++ as the primary language.
project(XenUI VERSION 0.9.0 LANGUAGES CXX)

# Enforce C++20 standard compliance for the entire project.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --------------------------------------------------------------------------------
# 1. External Dependency Discovery
#    Find required system-installed dependencies using standard CMake modules.
# --------------------------------------------------------------------------------
find_package(OpenGL REQUIRED)
find_package(Freetype REQUIRED)
find_package(glm REQUIRED)

# Conditionally find Fontconfig, which is necessary on Linux for system font discovery.
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_package(Fontconfig REQUIRED)
    # Define platform-specific libraries to be linked later.
    set(PLATFORM_SPECIFIC_LIBS Fontconfig::Fontconfig)
else()
    # No additional platform-specific libraries are needed on Windows or macOS for font handling.
    set(PLATFORM_SPECIFIC_LIBS "")
endif()

# --------------------------------------------------------------------------------
# 2. Configure Bundled SDL Dependencies
#
#    The framework is configured to locate SDL3, SDL3_ttf, and SDL3_image within a
#    local 'tools/' directory structure. This configuration allows developers to
#    use the framework without requiring a system-wide installation of these SDL libraries.
# --------------------------------------------------------------------------------

# Define root variables pointing to the expected location of the bundled SDL libraries.
# These paths are relative to the project's source directory (CMAKE_SOURCE_DIR).
set(SDL3_ROOT         "${CMAKE_SOURCE_DIR}/tools/SDL3-3.2.16")
set(SDL3_TTF_ROOT     "${CMAKE_SOURCE_DIR}/tools/SDL3_ttf-3.2.2")
set(SDL3_IMAGE_ROOT   "${CMAKE_SOURCE_DIR}/tools/SDL3_image-3.2.4")

# 2.1 Locate SDL3 headers and library
# Search for SDL.h in the specified SDL3 include path.
find_path(
    SDL3_INCLUDE_DIR
    NAMES SDL.h
    PATHS "${SDL3_ROOT}/include/SDL3"
)
# Search for the SDL3 shared or static library in the specified build path.
find_library(
    SDL3_LIBRARY
    NAMES SDL3 SDL3-static
    PATHS "${SDL3_ROOT}/build"
)

# 2.2 Locate SDL3_ttf headers and library
# Search for SDL_ttf.h in the specified SDL3_ttf include path.
find_path(
    SDL3_TTF_INCLUDE_DIR
    NAMES SDL_ttf.h
    PATHS "${SDL3_TTF_ROOT}/include/SDL3_ttf"
)
# Search for the SDL3_ttf shared or static library in the specified build path.
find_library(
    SDL3_TTF_LIBRARY
    NAMES SDL3_ttf SDL3_ttf-static
    PATHS "${SDL3_TTF_ROOT}/build"
)

# 2.3 Locate SDL3_image headers and library
# Search for SDL_image.h in the specified SDL3_image include path.
find_path(
    SDL3_IMAGE_INCLUDE_DIR
    NAMES SDL_image.h
    PATHS "${SDL3_IMAGE_ROOT}/include/SDL3_image"
)
# Search for the SDL3_image shared or static library in the specified build path.
find_library(
    SDL3_IMAGE_LIBRARY
    NAMES SDL3_image SDL3_image-static
    PATHS "${SDL3_IMAGE_ROOT}/build"
)

# --- Dependency Validation ---
# Halt configuration if any required SDL component is not found in the custom 'tools' directories.
if (NOT SDL3_INCLUDE_DIR OR NOT SDL3_LIBRARY)
    message(FATAL_ERROR "Could not find SDL3 in \"${SDL3_ROOT}\". Ensure tools/SDL3-3.2.16/include and tools/SDL3-3.2.16/build exist.")
endif()
if (NOT SDL3_TTF_INCLUDE_DIR OR NOT SDL3_TTF_LIBRARY)
    message(FATAL_ERROR "Could not find SDL3_ttf in \"${SDL3_TTF_ROOT}\". Ensure tools/SDL3_ttf-3.2.4/include and tools/SDL3_ttf-3.2.4/build exist.")
endif()
if (NOT SDL3_IMAGE_INCLUDE_DIR OR NOT SDL3_IMAGE_LIBRARY)
    message(FATAL_ERROR "Could not find SDL3_image in \"${SDL3_IMAGE_ROOT}\". Ensure tools/SDL3_image-3.2.4/include and tools/SDL3_image-3.2.4/build exist.")
endif()

# --------------------------------------------------------------------------------
# 3. Project Directories and Headers
# --------------------------------------------------------------------------------
# Define internal variables for public headers and source files locations.
set(XENUI_PUBLIC_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")
set(XENUI_SOURCE_DIR        "${PROJECT_SOURCE_DIR}/src")

# Use GLOB to collect all public header files required for installation.
file(GLOB XENUI_PUBLIC_HEADERS
    "${XENUI_PUBLIC_INCLUDE_DIR}/XenUI/*.h"
)

# --------------------------------------------------------------------------------
# 4. Shared Library Definition (XenUI)
# --------------------------------------------------------------------------------
# Define the XenUI target as a SHARED library and list all its source files.
add_library(XenUI SHARED
    ${XENUI_SOURCE_DIR}/TextRenderer.cpp
    ${XENUI_SOURCE_DIR}/Label.cpp
    ${XENUI_SOURCE_DIR}/Anchor.cpp
    ${XENUI_SOURCE_DIR}/WindowUtil.cpp
    ${XENUI_SOURCE_DIR}/Shape.cpp
    ${XENUI_SOURCE_DIR}/Image.cpp
    ${XENUI_SOURCE_DIR}/Button.cpp
    ${XENUI_SOURCE_DIR}/InputBox.cpp
    ${XENUI_SOURCE_DIR}/RadioButton.cpp
    ${XENUI_SOURCE_DIR}/CheckBox.cpp
    ${XENUI_SOURCE_DIR}/Switch.cpp
    ${XENUI_SOURCE_DIR}/Slider.cpp
    ${XENUI_SOURCE_DIR}/Dropdown.cpp
    ${XENUI_SOURCE_DIR}/ScrollView.cpp
)

# Set properties for library versioning and identify the public headers for installation.
set_target_properties(XenUI PROPERTIES
    VERSION     ${PROJECT_VERSION}
    SOVERSION   1
    PUBLIC_HEADER "${XENUI_PUBLIC_HEADERS}"
)

# Configure include directories for the target.
target_include_directories(XenUI
    PUBLIC
        # Installation Interface: Where consumers will find headers after installation.
        $<INSTALL_INTERFACE:include>
        # Build Interface: Where the compiler finds headers during the build.
        $<BUILD_INTERFACE:${XENUI_PUBLIC_INCLUDE_DIR}>
    PRIVATE
        # Private includes for internal source files.
        ${XENUI_SOURCE_DIR}
        ${SDL3_INCLUDE_DIR}
        ${SDL3_TTF_INCLUDE_DIR}
        ${SDL3_IMAGE_INCLUDE_DIR}
        ${FREETYPE_INCLUDE_DIRS}
        ${OPENGL_INCLUDE_DIR}
)

# --------------------------------------------------------------------------------
# 5. Define Fallback Font Path Logic
#
#    This section defines a macro that passes the *installed* location of a bundled
#    font to the C++ compiler. The TextRenderer class uses this path at runtime
#    to load the default font asset.
# --------------------------------------------------------------------------------
# Define the source location of the bundled font.
set(FALLBACK_FONT_SOURCE    "${CMAKE_SOURCE_DIR}/third_party/dejavu-fonts-ttf-2.37/ttf/DejaVuSans.ttf")
# Include module to access standard installation paths (like CMAKE_INSTALL_DATADIR).
include(GNUInstallDirs)
# Define the final installation directory for XenUI's fonts.
set(XENUI_FONT_INSTALL_DIR  "${CMAKE_INSTALL_DATADIR}/fonts/XenUI")
# Construct the full path where the font will reside after installation.
set(FALLBACK_FONT_INSTALLED_PATH "${XENUI_FONT_INSTALL_DIR}/DejaVuSans.ttf")

# Pass the final installed font path as a preprocessor definition (XENUI_FALLBACK_FONT_PATH)
# to the C++ compiler. The definition uses C-style string quoting for the path.
target_compile_definitions(XenUI PRIVATE
    "XENUI_FALLBACK_FONT_PATH=\"${FALLBACK_FONT_INSTALLED_PATH}\""
)

# --------------------------------------------------------------------------------
# 6. Link Libraries
# --------------------------------------------------------------------------------
# Link all dependencies to the XenUI target privately. Private linking ensures
# that consumers of XenUI do not automatically inherit dependencies like SDL3
# unless explicitly needed.
target_link_libraries(XenUI PRIVATE
    OpenGL::GL
    Freetype::Freetype
    glm::glm
    ${SDL3_LIBRARY}
    ${SDL3_TTF_LIBRARY}
    ${SDL3_IMAGE_LIBRARY}
    ${PLATFORM_SPECIFIC_LIBS} # Includes Fontconfig on Linux
)

# --------------------------------------------------------------------------------
# 7. Post-Build Step for Development Convenience
#
#    This command copies the built shared library artifact into a predictable
#    'build/lib' directory inside the binary path after compilation. This facilitates
#    easy linking by example applications without requiring an 'install' step.
# --------------------------------------------------------------------------------
add_custom_command(TARGET XenUI POST_BUILD
    # Create the destination directory
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/lib"
    # Copy the target shared library file
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:XenUI> "${CMAKE_BINARY_DIR}/lib"
    COMMENT "Copying XenUI shared library into ${CMAKE_BINARY_DIR}/lib"
)

# --------------------------------------------------------------------------------
# 8. Installation Rules and Package Configuration
# --------------------------------------------------------------------------------

# Install the built target (shared library) to standard library and runtime locations.
install(TARGETS XenUI
    EXPORT           XenUITargets
    RUNTIME          DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY          DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE          DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/XenUI
)

# Install the public header directory structure itself.
install(DIRECTORY ${XENUI_PUBLIC_INCLUDE_DIR}/XenUI/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/XenUI
)

# Install the required fallback font file to the predefined font installation directory.
install(FILES "${FALLBACK_FONT_SOURCE}"
    DESTINATION "${XENUI_FONT_INSTALL_DIR}"
)

# --- CMake Package Configuration Generation ---

# Include the module for generating package configuration files.
include(CMakePackageConfigHelpers)

# Generate a version file for the package configuration.
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/XenUIConfigVersion.cmake"
    VERSION     ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

# Configure the main package configuration file from a template (XenUIConfig.cmake.in).
# This file will help consuming projects locate XenUI after installation.
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/XenUIConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/XenUIConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/XenUI
    PATH_VARS          CMAKE_INSTALL_INCLUDEDIR CMAKE_INSTALL_LIBDIR XENUI_FONT_INSTALL_DIR
)

# Install the generated configuration files.
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/XenUIConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/XenUIConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/XenUI
)

# Export the XenUI target for consumers using 'find_package'.
install(EXPORT XenUITargets
    FILE      XenUITargets.cmake
    NAMESPACE XenUI::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/XenUI
)

# Output summary information to the console after configuration is complete.
message(STATUS "Configured XenUI Framework Library ${PROJECT_VERSION}")
message(STATUS "  Install Prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  Install Library Dir: ${CMAKE_INSTALL_LIBDIR}")
message(STATUS "  Install Include Dir: ${CMAKE_INSTALL_INCLUDEDIR}")
message(STATUS "  Install Font Dir: ${XENUI_FONT_INSTALL_DIR}")