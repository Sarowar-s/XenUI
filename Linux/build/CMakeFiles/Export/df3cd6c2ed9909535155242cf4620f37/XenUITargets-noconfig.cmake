#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "XenUI::XenUI" for configuration ""
set_property(TARGET XenUI::XenUI APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(XenUI::XenUI PROPERTIES
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libXenUI.so.1.0.0"
  IMPORTED_SONAME_NOCONFIG "libXenUI.so.1"
  )

list(APPEND _cmake_import_check_targets XenUI::XenUI )
list(APPEND _cmake_import_check_files_for_XenUI::XenUI "${_IMPORT_PREFIX}/lib/libXenUI.so.1.0.0" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
