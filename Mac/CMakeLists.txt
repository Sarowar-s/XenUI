#   SPDX-License-Identifier: Apache-2.0
#
#   Copyright (C) 2025 MD S M Sarowar Hossain
#
#



# Minimum CMake version required
cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

# Define the project name and language
project(MyFrameworkApp LANGUAGES CXX)

# Set the C++ standard to C++20 as required by XenUI
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add the executable target

add_executable(${PROJECT_NAME} MACOSX_BUNDLE main.cpp)

# --- macOS Specific Linking: CoreFoundation ---
# Link against CoreFoundation framework for macOS-specific APIs (like bundle resource access)
if (APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework CoreFoundation")
endif()

# --- Macro for Finding and Bundling System-Wide Libraries ---
# This macro encapsulates the logic for finding a system-wide library
# and then setting up its bundling into the .app bundle.
#
# Usage:
# FIND_AND_BUNDLE_SYSTEM_LIB(
#   <LIB_NAME>             # e.g., SDL3, SDL3_ttf, SDL3_image, Freetype
#   <PACKAGE_NAME>         # e.g., SDL3, SDL3_ttf, SDL3_image, Freetype (for find_package)
# )
macro(FIND_AND_BUNDLE_SYSTEM_LIB LIB_NAME PACKAGE_NAME)
    # Define variables for this specific library
    set(${LIB_NAME}_FOUND FALSE)
    set(${LIB_NAME}_LOCATION "")
    set(${LIB_NAME}_LINK_TARGET "")
    set(${LIB_NAME}_LIB_FILENAME "") # To store the actual filename found (e.g., libSDL3.0.dylib)

    message(STATUS "Attempting to find ${LIB_NAME} via find_package...")


    find_package(${PACKAGE_NAME} REQUIRED)

    if (${PACKAGE_NAME}_FOUND)
        message(STATUS "  Found ${LIB_NAME} via find_package.")
        set(${LIB_NAME}_LINK_TARGET ${PACKAGE_NAME}::${PACKAGE_NAME})
        get_target_property(${LIB_NAME}_LOCATION ${PACKAGE_NAME}::${PACKAGE_NAME} LOCATION)
        if (${LIB_NAME}_LOCATION)
            get_filename_component(${LIB_NAME}_LIB_FILENAME "${${LIB_NAME}_LOCATION}" NAME)
            set(${LIB_NAME}_FOUND TRUE)
        else()
            message(FATAL_ERROR "  ${LIB_NAME} found via find_package, but LOCATION property is empty. Cannot bundle. Ensure it's a valid library.")
        endif()
    else()
        # This block should ideally not be reached due to 'REQUIRED' in find_package,
        # but serves as a redundant check.
        message(FATAL_ERROR "Could not find ${LIB_NAME} via find_package. Please ensure it's installed system-wide (e.g., via Homebrew).")
    endif()


    # --- Bundling Logic (macOS specific) ---
    if (APPLE AND ${LIB_NAME}_FOUND)
        message(STATUS "  Bundling ${LIB_NAME} (${${LIB_NAME}_LIB_FILENAME}) into .app bundle.")

        set(BUNDLE_FRAMEWORKS_DIR "$<TARGET_BUNDLE_DIR:${PROJECT_NAME}>/Contents/Frameworks")
        set(BUNDLED_LIB_PATH "${BUNDLE_FRAMEWORKS_DIR}/${${LIB_NAME}_LIB_FILENAME}")

        # Copy the dynamic library into the bundle
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "${BUNDLE_FRAMEWORKS_DIR}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${${LIB_NAME}_LOCATION}"
                    "${BUNDLE_FRAMEWORKS_DIR}/"
            COMMENT "Copying ${LIB_NAME} dynamic library: ${${LIB_NAME}_LIB_FILENAME}"
        )

       
        # 1. Tell the executable to look for this library within its own bundle
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND install_name_tool -change
                    "${${LIB_NAME}_LOCATION}" # Original path linked by executable
                    "@rpath/${${LIB_NAME}_LIB_FILENAME}" # Relative path within the bundle
                    "$<TARGET_BUNDLE_DIR:${PROJECT_NAME}>/Contents/MacOS/${PROJECT_NAME}"
            COMMENT "Fixing install_name for ${LIB_NAME} in executable"
        )
        # 2. Set the identification name of the bundled library itself
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND install_name_tool -id "@rpath/${${LIB_NAME}_LIB_FILENAME}" "${BUNDLED_LIB_PATH}"
            COMMENT "Fixing install_name for bundled ${LIB_NAME} library"
        )

       
    endif()
endmacro()

# --- Find and Bundle Core Libraries (System-Wide Only) ---

# Find and Bundle SDL3 and its extensions
FIND_AND_BUNDLE_SYSTEM_LIB(SDL3 SDL3)
FIND_AND_BUNDLE_SYSTEM_LIB(SDL3_ttf SDL3_ttf)
FIND_AND_BUNDLE_SYSTEM_LIB(SDL3_image SDL3_image)

# Find and Bundle FreeType
FIND_AND_BUNDLE_SYSTEM_LIB(Freetype Freetype)

# Find OpenGL 
find_package(OpenGL REQUIRED)

# Find GLM 
find_package(glm REQUIRED)

# --- XenUI Framework Library ---
# Define XenUI project directories relative to the top-level project source directory
set(XENUI_PUBLIC_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/XenUI/include")
set(XENUI_SOURCE_DIR "${PROJECT_SOURCE_DIR}/src")

# Lists of XenUI source files
set(XENUI_SOURCES
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

# the version for the XenUI library

set(XENUI_LIBRARY_VERSION "1.0.0")

# Build XenUI as a shared library (.dylib)
add_library(XenUI SHARED ${XENUI_SOURCES})

# Set properties for XenUI library
set_target_properties(XenUI PROPERTIES
    VERSION ${XENUI_LIBRARY_VERSION}
    SOVERSION 1
)

# Include directories for XenUI
target_include_directories(XenUI
    PUBLIC
        $<INSTALL_INTERFACE:include> # For when XenUI is installed
        $<BUILD_INTERFACE:${XENUI_PUBLIC_INCLUDE_DIR}> # For when XenUI is built within this project
    PRIVATE
        ${XENUI_SOURCE_DIR}
        # Include directories for SDL3 and its extensions
        ${SDL3_INCLUDE_DIR}
        ${SDL3_TTF_INCLUDE_DIR}
        ${SDL3_IMAGE_INCLUDE_DIR}
        # Include directories for FreeType, OpenGL, and GLM
        ${FREETYPE_INCLUDE_DIRS}
        ${OPENGL_INCLUDE_DIRS} # find_package(OpenGL) sets OPENGL_INCLUDE_DIRS
        ${glm_INCLUDE_DIR}     # find_package(glm) sets glm_INCLUDE_DIR
)

# Define fallback font path for XenUI relative to the top-level project source directory
set(FALLBACK_FONT_SOURCE "${PROJECT_SOURCE_DIR}/third_party/dejavu-fonts-ttf-2.37/ttf/DejaVuSans.ttf")
set(XENUI_FONT_INSTALL_DIR "fonts/XenUI") # Relative path within bundle's Resources

target_compile_definitions(XenUI PRIVATE
    "XENUI_FALLBACK_FONT_PATH=\"@rpath/../Resources/${XENUI_FONT_INSTALL_DIR}/DejaVuSans.ttf\""
)


# Link libraries for XenUI
target_link_libraries(XenUI PRIVATE
    SDL3::SDL3
    SDL3_ttf::SDL3_ttf
    SDL3_image::SDL3_image
    Freetype::Freetype
    OpenGL::GL 
    glm::glm   
)


target_link_libraries(${PROJECT_NAME} PRIVATE
    XenUI
    SDL3::SDL3
    SDL3_ttf::SDL3_ttf
    SDL3_image::SDL3_image
)

# --- Add rpath to executable ONCE after all libraries are processed ---

if (APPLE)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND install_name_tool -add_rpath
                "@executable_path/../Frameworks" # Add rpath to executable
                "$<TARGET_BUNDLE_DIR:${PROJECT_NAME}>/Contents/MacOS/${PROJECT_NAME}"
        COMMENT "Adding rpath for Frameworks directory to executable (once)"
    )
endif()

# --- Info.plist Copying ---
# Copies the Info.plist from the project root into the .app bundle's Contents/ directory.

configure_file(
    "${PROJECT_SOURCE_DIR}/Info.plist"
    "$<TARGET_BUNDLE_DIR:${PROJECT_NAME}>/Contents/Info.plist"
    COPYONLY
)

# --- Resource Copying ---
# This section ensures that resources (like fonts, images, etc.)
# are copied into the .app bundle's Contents/Resources directory.


# Copy general resources
file(GLOB_RECURSE APP_RESOURCES "${PROJECT_SOURCE_DIR}/res/*")
foreach(RES_FILE ${APP_RESOURCES})
    # Filter out .DS_Store files to avoid copying macOS metadata
    if (NOT "${RES_FILE}" MATCHES "/\\.DS_Store$")
        file(RELATIVE_PATH REL_PATH "${PROJECT_SOURCE_DIR}/res" "${RES_FILE}") 
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
                    "$<TARGET_BUNDLE_DIR:${PROJECT_NAME}>/Contents/Resources/${REL_PATH}/.."
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${RES_FILE}"
                    "$<TARGET_BUNDLE_DIR:${PROJECT_NAME}>/Contents/Resources/${REL_PATH}"
            COMMENT "Copying resource: ${REL_PATH}"
        )
    endif()
endforeach()

# Copy XenUI fallback font
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
            "$<TARGET_BUNDLE_DIR:${PROJECT_NAME}>/Contents/Resources/${XENUI_FONT_INSTALL_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${FALLBACK_FONT_SOURCE}"
            "$<TARGET_BUNDLE_DIR:${PROJECT_NAME}>/Contents/Resources/${XENUI_FONT_INSTALL_DIR}/DejaVuSans.ttf"
    COMMENT "Copying XenUI fallback font: DejaVuSans.ttf"
)