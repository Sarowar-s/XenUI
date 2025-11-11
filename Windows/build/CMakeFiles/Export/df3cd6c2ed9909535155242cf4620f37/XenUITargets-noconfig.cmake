#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "XenUI::XenUI" for configuration ""
set_property(TARGET XenUI::XenUI APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(XenUI::XenUI PROPERTIES
  IMPORTED_IMPLIB_NOCONFIG "${_IMPORT_PREFIX}/lib/libXenUI.dll.a"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/bin/libXenUI.dll"
  )

list(APPEND _cmake_import_check_targets XenUI::XenUI )
list(APPEND _cmake_import_check_files_for_XenUI::XenUI "${_IMPORT_PREFIX}/lib/libXenUI.dll.a" "${_IMPORT_PREFIX}/bin/libXenUI.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
