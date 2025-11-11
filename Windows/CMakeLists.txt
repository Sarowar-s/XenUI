#
# 
#
# SPDX-License-Identifier: Apache-2.0
#
#  Copyright (C) 2025 MD S M Sarowar Hossain
# 
# 
# 
# 
#
# 
#
# Description:
# This file configures the build process for the XenUI shared library.
# It handles finding external dependencies (OpenGL, Freetype, GLM, and bundled SDL3
# libraries), defining the source files, setting target properties, and creating
# installation rules and package configuration files.
#

cmake_minimum_required(VERSION 3.10)
# Define the project, including its version and required language.
project(XenUI VERSION 0.9.0 LANGUAGES CXX)

# Enforce C++20 standard compliance for the XenUI library.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --------------------------------------------------------------------------------
# 1. Find System Dependencies (OpenGL, Freetype, GLM)
#
#    These libraries are generally expected to be installed on the system
#    or found via CMAKE_PREFIX_PATH hints.
# --------------------------------------------------------------------------------
# Find system-installed OpenGL and Freetype libraries.
find_package(OpenGL REQUIRED)
find_package(Freetype REQUIRED)

# --- Find GLM Manually (More Robust) ---
# GLM is a header-only library, but many installations don't provide a
# standard 'glmConfig.cmake' file for find_package(glm). We find the headers
# manually and create a modern INTERFACE target.
find_path(
    GLM_INCLUDE_DIR
    NAMES glm/glm.hpp
    # Check the user-provided CMAKE_PREFIX_PATH hint first, then common paths.
    HINTS "${CMAKE_PREFIX_PATH}/include"
    # Search within a 'glm' subdirectory in the standard include paths.
    PATH_SUFFIXES glm
)

if(NOT GLM_INCLUDE_DIR)
    message(FATAL_ERROR "Could not find the GLM headers (glm/glm.hpp). "
                        "Please ensure GLM is installed and provide its location "
                        "via CMAKE_PREFIX_PATH, or set GLM_INCLUDE_DIR manually.")
endif()

# Create a modern INTERFACE target for the header-only GLM library.
# This allows the use of 'target_link_libraries(XenUI PRIVATE glm::glm)'
# which automatically adds the include directory.
add_library(glm::glm INTERFACE IMPORTED)
set_target_properties(glm::glm PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${GLM_INCLUDE_DIR}"
)
message(STATUS "Found GLM headers at: ${GLM_INCLUDE_DIR}")
# --- End of Manual GLM Find ---


# --------------------------------------------------------------------------------
# 2. Locate SDL Libraries from the "tools" Folder
#
#    This section assumes SDL3 and its extensions are pre-built and placed
#    in subdirectories under the project's 'tools/' folder.
# --------------------------------------------------------------------------------
# Define the relative paths to the SDL dependency roots.
set(SDL3_ROOT 	 	 "${CMAKE_SOURCE_DIR}/tools/SDL3-3.2.16")
set(SDL3_TTF_ROOT 	 "${CMAKE_SOURCE_DIR}/tools/SDL3_ttf-3.2.2")
set(SDL3_IMAGE_ROOT 	 "${CMAKE_SOURCE_DIR}/tools/SDL3_image-3.2.4")


# 2.1 Locate SDL3 headers, static/shared library, and runtime DLL (for Windows)
find_path(
    SDL3_INCLUDE_DIR
    NAMES SDL3/SDL.h
    HINTS "${SDL3_ROOT}/include"
)
find_library(
    SDL3_LIBRARY
    NAMES SDL3
    HINTS "${SDL3_ROOT}/lib/x64" "${SDL3_ROOT}/lib" "${SDL3_ROOT}/build"
)
# Specifically find the DLL for Windows runtime deployment.
find_file(
    SDL3_DLL
    NAMES SDL3.dll
    HINTS "${SDL3_ROOT}/lib/x64" "${SDL3_ROOT}/lib" "${SDL3_ROOT}/build"
)


# 2.2 Locate SDL3_ttf headers, library, and DLL
find_path(
    SDL3_TTF_INCLUDE_DIR
    NAMES SDL3_ttf/SDL_ttf.h
    HINTS "${SDL3_TTF_ROOT}/include"
)
find_library(
    SDL3_TTF_LIBRARY
    NAMES SDL3_ttf
    HINTS "${SDL3_TTF_ROOT}/lib/x64" "${SDL3_TTF_ROOT}/lib" "${SDL3_TTF_ROOT}/build"
)
find_file(
    SDL3_TTF_DLL
    NAMES SDL3_ttf.dll
    HINTS "${SDL3_TTF_ROOT}/lib/x64" "${SDL3_TTF_ROOT}/lib" "${SDL3_TTF_ROOT}/build"
)

# 2.3 Locate SDL3_image headers, library, and DLL
find_path(
    SDL3_IMAGE_INCLUDE_DIR
    NAMES SDL3_image/SDL_image.h
    HINTS "${SDL3_IMAGE_ROOT}/include"
)
find_library(
    SDL3_IMAGE_LIBRARY
    NAMES SDL3_image
    HINTS "${SDL3_IMAGE_ROOT}/lib/x64" "${SDL3_IMAGE_ROOT}/lib" "${SDL3_IMAGE_ROOT}/build"
)
find_file(
    SDL3_IMAGE_DLL
    NAMES SDL3_image.dll
    HINTS "${SDL3_IMAGE_ROOT}/lib/x64" "${SDL3_IMAGE_ROOT}/lib" "${SDL3_IMAGE_ROOT}/build"
)


# 2.4 Validate that all required SDL components were found
if (NOT SDL3_INCLUDE_DIR OR NOT SDL3_LIBRARY)
    message(FATAL_ERROR "Could not find SDL3 in \"${SDL3_ROOT}\". Make sure the include (.h) and library (.dll) files exist.")
endif()
if (NOT SDL3_TTF_INCLUDE_DIR OR NOT SDL3_TTF_LIBRARY)
    message(FATAL_ERROR "Could not find SDL3_ttf in \"${SDL3_TTF_ROOT}\". Make sure the include (.h) and library (.dll) files exist.")
endif()
if (NOT SDL3_IMAGE_INCLUDE_DIR OR NOT SDL3_IMAGE_LIBRARY)
    message(FATAL_ERROR "Could not find SDL3_image in \"${SDL3_IMAGE_ROOT}\". Make sure the include (.h) and library (.dll) files exist.")
endif()

# --------------------------------------------------------------------------------
# 3. Project Directories and Source File Grouping
# --------------------------------------------------------------------------------
set(XENUI_PUBLIC_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")
set(XENUI_SOURCE_DIR 		 "${PROJECT_SOURCE_DIR}/src")

# Gather all public headers for documentation and installation purposes.
file(GLOB XENUI_PUBLIC_HEADERS
    "${XENUI_PUBLIC_INCLUDE_DIR}/XenUI/*.h"
)

# --------------------------------------------------------------------------------
# 4. Build XenUI as a Shared Library (DLL/SO)
# --------------------------------------------------------------------------------
add_library(XenUI SHARED
    # List all source files that make up the library.
	${XENUI_SOURCE_DIR}/Anchor.cpp
	${XENUI_SOURCE_DIR}/Button.cpp
	${XENUI_SOURCE_DIR}/CheckBox.cpp
	${XENUI_SOURCE_DIR}/Dropdown.cpp
	${XENUI_SOURCE_DIR}/Image.cpp
	${XENUI_SOURCE_DIR}/InputBox.cpp
	${XENUI_SOURCE_DIR}/Label.cpp
	${XENUI_SOURCE_DIR}/RadioButton.cpp
	${XENUI_SOURCE_DIR}/ScrollView.cpp
	${XENUI_SOURCE_DIR}/Shape.cpp
	${XENUI_SOURCE_DIR}/Slider.cpp
	${XENUI_SOURCE_DIR}/Switch.cpp
	${XENUI_SOURCE_DIR}/TextRenderer.cpp
	${XENUI_SOURCE_DIR}/WindowUtil.cpp
)

set_target_properties(XenUI PROPERTIES
    # Set library versioning info for proper shared library management on Linux/macOS.
	VERSION 	${PROJECT_VERSION}
	SOVERSION 	1
	PUBLIC_HEADER "${XENUI_PUBLIC_HEADERS}"
    # Required for Windows DLLs to ensure symbols are exported correctly.
	WINDOWS_EXPORT_ALL_SYMBOLS ON
)

# Define include directories using PUBLIC/PRIVATE/INTERFACE modern CMake usage.
target_include_directories(XenUI
    PUBLIC
        # The path consumers will use when XenUI is installed.
        $<INSTALL_INTERFACE:include>
        # The path XenUI uses during its own build phase (e.g., when building examples).
        $<BUILD_INTERFACE:${XENUI_PUBLIC_INCLUDE_DIR}>
    PRIVATE
        # Internal header paths used only by XenUI source files.
        ${XENUI_SOURCE_DIR}
        ${SDL3_INCLUDE_DIR}
        ${SDL3_TTF_INCLUDE_DIR}
        ${SDL3_IMAGE_INCLUDE_DIR}
        ${FREETYPE_INCLUDE_DIRS}
        ${OpenGL_INCLUDE_DIR}
        # GLM include is now handled implicitly by linking to glm::glm
)

# --------------------------------------------------------------------------------
# 5. Define Fallback Font Logic
#
#    Sets a compile-time definition for the default font path using its expected
#    installation location.
# --------------------------------------------------------------------------------
set(FALLBACK_FONT_SOURCE 	"${CMAKE_SOURCE_DIR}/third_party/dejavu-fonts-ttf-2.37/ttf/DejaVuSans.ttf")
include(GNUInstallDirs) # Provides standard GNU installation variables (e.g., DATADIR).
set(XENUI_FONT_INSTALL_DIR 	"${CMAKE_INSTALL_DATADIR}/fonts/XenUI")
set(FALLBACK_FONT_INSTALLED_PATH "${XENUI_FONT_INSTALL_DIR}/DejaVuSans.ttf")

# Pass the final installed font path as a preprocessor definition to the C++ code.
target_compile_definitions(XenUI PRIVATE
	"XENUI_FALLBACK_FONT_PATH=\"${FALLBACK_FONT_INSTALLED_PATH}\""
)

# --------------------------------------------------------------------------------
# 6. Link Libraries
#
#    Link XenUI against all required system and external dependencies. Use PRIVATE
#    since consumers of XenUI don't necessarily need to link these themselves.
# --------------------------------------------------------------------------------
target_link_libraries(XenUI PRIVATE
    # Modern targets from find_package
	OpenGL::GL
	Freetype::Freetype
	glm::glm # This links the interface target, pulling in the headers automatically.
    # Manual library variables from find_library
	${SDL3_LIBRARY}
	${SDL3_TTF_LIBRARY}
	${SDL3_IMAGE_LIBRARY}
)

# --------------------------------------------------------------------------------
# 7. Post-build Commands for Windows (DLL Copying)
#
#    On Windows, executables need runtime DLLs next to them. This copies the
#    built XenUI DLL and its SDL dependencies into a central 'lib' folder
#    in the build directory.
# --------------------------------------------------------------------------------
set(OUTPUT_LIB_DIR "${CMAKE_BINARY_DIR}/lib")

# 7.1 Copy the built XenUI library.
add_custom_command(TARGET XenUI POST_BUILD
    # Ensure the destination directory exists.
	COMMAND ${CMAKE_COMMAND} -E make_directory "${OUTPUT_LIB_DIR}"
    # Copy the resulting library file.
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:XenUI> "${OUTPUT_LIB_DIR}"
	COMMENT "Copying XenUI library to ${OUTPUT_LIB_DIR}"
)

# 7.2 Copy the SDL runtime DLLs only if they were successfully found.
if(SDL3_DLL AND SDL3_TTF_DLL AND SDL3_IMAGE_DLL)
	add_custom_command(TARGET XenUI POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different "${SDL3_DLL}" "${OUTPUT_LIB_DIR}"
		COMMAND ${CMAKE_COMMAND} -E copy_if_different "${SDL3_TTF_DLL}" "${OUTPUT_LIB_DIR}"
		COMMAND ${CMAKE_COMMAND} -E copy_if_different "${SDL3_IMAGE_DLL}" "${OUTPUT_LIB_DIR}"
		COMMENT "Copying SDL runtime DLLs to ${OUTPUT_LIB_DIR}"
	)
endif()


# --------------------------------------------------------------------------------
# 8. Installation Rules and Package Configuration
#
#    Defines how the library, headers, and the CMake configuration files should
#    be installed for use by other projects.
# --------------------------------------------------------------------------------

# 8.1 Install the XenUI target components (DLL/SO, import library, headers).
install(TARGETS XenUI
	EXPORT 			XenUITargets
	RUNTIME 		DESTINATION ${CMAKE_INSTALL_BINDIR}
	LIBRARY 		DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE 		DESTINATION ${CMAKE_INSTALL_LIBDIR}
	PUBLIC_HEADER 	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/XenUI
)

# 8.2 Install the remaining public headers (ensuring directory structure).
install(DIRECTORY ${XENUI_PUBLIC_INCLUDE_DIR}/XenUI/
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/XenUI
)

# 8.3 Install the bundled fallback font to its expected runtime location.
install(FILES "${FALLBACK_FONT_SOURCE}"
	DESTINATION "${XENUI_FONT_INSTALL_DIR}"
)

# --- Package configuration for find_package(XenUI) ---
include(CMakePackageConfigHelpers)

# Write the version file for the package (e.g., XenUIConfigVersion.cmake).
write_basic_package_version_file(
	"${CMAKE_CURRENT_BINARY_DIR}/XenUIConfigVersion.cmake"
	VERSION 	${PROJECT_VERSION}
	COMPATIBILITY AnyNewerVersion
)

# Configure the main package config file from its template.
configure_package_config_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake/XenUIConfig.cmake.in"
	"${CMAKE_CURRENT_BINARY_DIR}/XenUIConfig.cmake"
	INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/XenUI
	PATH_VARS 		 CMAKE_INSTALL_INCLUDEDIR CMAKE_INSTALL_LIBDIR XENUI_FONT_INSTALL_DIR
)

# Install the generated configuration files.
install(FILES
	"${CMAKE_CURRENT_BINARY_DIR}/XenUIConfig.cmake"
	"${CMAKE_CURRENT_BINARY_DIR}/XenUIConfigVersion.cmake"
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/XenUI
)

# Export and install the targets file, allowing consumers to link directly via 'XenUI::XenUI'.
install(EXPORT XenUITargets
	FILE 		XenUITargets.cmake
	NAMESPACE XenUI::
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/XenUI
)

# Final status messages for user confirmation.
message(STATUS "Configured XenUI Framework Library ${PROJECT_VERSION} for Windows")
message(STATUS " 	Install Prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS " 	Install Library Dir: ${CMAKE_INSTALL_LIBDIR}")
message(STATUS " 	Install Include Dir: ${CMAKE_INSTALL_INCLUDEDIR}")
message(STATUS " 	Install Font Dir: ${XENUI_FONT_INSTALL_DIR}")
