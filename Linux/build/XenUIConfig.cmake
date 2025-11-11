make/XenUIConfig.cmake.in

# cmake/XenUIConfig.cmake.in


####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was XenUIConfig.cmake.in                            ########

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)

macro(set_and_check _var _file)
  set(${_var} "${_file}")
  if(NOT EXISTS "${_file}")
    message(FATAL_ERROR "File or directory ${_file} referenced by variable ${_var} does not exist !")
  endif()
endmacro()

macro(check_required_components _NAME)
  foreach(comp ${${_NAME}_FIND_COMPONENTS})
    if(NOT ${_NAME}_${comp}_FOUND)
      if(${_NAME}_FIND_REQUIRED_${comp})
        set(${_NAME}_FOUND FALSE)
      endif()
    endif()
  endforeach()
endmacro()

####################################################################################

# Find dependencies that XenUI depends on transitively
# (Users might need these include dirs/libs too)
find_dependency(SDL2 REQUIRED)
find_dependency(SDL2_ttf REQUIRED)
# Add find_dependency for Freetype, glm, OpenGL if their headers/libs are needed by users
# find_dependency(Freetype REQUIRED)

# Check if targets file exists (created by install(EXPORT ...))
if(NOT TARGET XenUI::XenUI)
    include("${CMAKE_CURRENT_LIST_DIR}/XenUITargets.cmake")
endif()

# Define helper variable (optional)
set(XENUI_INCLUDE_DIRS "${PACKAGE_PREFIX_DIR}/include")
set(XENUI_LIBRARY_DIRS "${PACKAGE_PREFIX_DIR}/lib")
set(XENUI_LIBRARIES XenUI::XenUI) # The imported target
