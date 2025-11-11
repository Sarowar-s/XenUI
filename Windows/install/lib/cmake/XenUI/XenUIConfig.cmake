# --------------------------------------------------------------------------------
# XenUIConfig.cmake.in (Corrected Version)
#
# This template generates the XenUIConfig.cmake file. It ensures that any
# project using XenUI will also find all the necessary dependencies like SDL3,
# Freetype, and OpenGL.
# --------------------------------------------------------------------------------


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

# Include the CMake helper module that provides the find_dependency() command.
include(CMakeFindDependencyMacro)

# Find all the libraries that XenUI itself depends on. This makes them
# available to any project that uses XenUI. The versions here should
# match the ones used in the main CMakeLists.txt.
find_dependency(OpenGL REQUIRED)
find_dependency(Freetype REQUIRED)

# NOTE: We don't need to find SDL3, SDL3_ttf, or SDL3_image here because
# the main CMakeLists.txt finds them manually and links them directly.
# The necessary library paths are already encoded into the XenUI::XenUI target.
# GLM is also handled automatically as it's a header-only INTERFACE target.

# Check if the main library target has been defined, and if not,
# include the file that defines it.
if(NOT TARGET XenUI::XenUI)
    include("${CMAKE_CURRENT_LIST_DIR}/XenUITargets.cmake")
endif()

