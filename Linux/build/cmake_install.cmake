# Install script for directory: /home/sarowar/Documents/Linux Experiment/Applications

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libXenUI.so.1.0.0"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libXenUI.so.1"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      file(RPATH_CHECK
           FILE "${file}"
           RPATH "")
    endif()
  endforeach()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES
    "/home/sarowar/Documents/Linux Experiment/Applications/build/libXenUI.so.1.0.0"
    "/home/sarowar/Documents/Linux Experiment/Applications/build/libXenUI.so.1"
    )
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libXenUI.so.1.0.0"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libXenUI.so.1"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      file(RPATH_CHANGE
           FILE "${file}"
           OLD_RPATH "/home/sarowar/Documents/Linux Experiment/Applications/tools/SDL3-3.2.16/build:/home/sarowar/Documents/Linux Experiment/Applications/tools/SDL3_ttf-3.2.2/build:/home/sarowar/Documents/Linux Experiment/Applications/tools/SDL3_image-3.2.4/build:"
           NEW_RPATH "")
      if(CMAKE_INSTALL_DO_STRIP)
        execute_process(COMMAND "/usr/bin/strip" "${file}")
      endif()
    endif()
  endforeach()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/home/sarowar/Documents/Linux Experiment/Applications/build/libXenUI.so")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/XenUI" TYPE FILE FILES
    "/home/sarowar/Documents/Linux Experiment/Applications/include/XenUI/Anchor.h"
    "/home/sarowar/Documents/Linux Experiment/Applications/include/XenUI/Button.h"
    "/home/sarowar/Documents/Linux Experiment/Applications/include/XenUI/CheckBox.h"
    "/home/sarowar/Documents/Linux Experiment/Applications/include/XenUI/Dropdown.h"
    "/home/sarowar/Documents/Linux Experiment/Applications/include/XenUI/Image.h"
    "/home/sarowar/Documents/Linux Experiment/Applications/include/XenUI/InputBox.h"
    "/home/sarowar/Documents/Linux Experiment/Applications/include/XenUI/Label.h"
    "/home/sarowar/Documents/Linux Experiment/Applications/include/XenUI/Orientation.h"
    "/home/sarowar/Documents/Linux Experiment/Applications/include/XenUI/Position.h"
    "/home/sarowar/Documents/Linux Experiment/Applications/include/XenUI/RadioButton.h"
    "/home/sarowar/Documents/Linux Experiment/Applications/include/XenUI/ScrollView.h"
    "/home/sarowar/Documents/Linux Experiment/Applications/include/XenUI/Shape.h"
    "/home/sarowar/Documents/Linux Experiment/Applications/include/XenUI/Slider.h"
    "/home/sarowar/Documents/Linux Experiment/Applications/include/XenUI/Switch.h"
    "/home/sarowar/Documents/Linux Experiment/Applications/include/XenUI/TextRenderer.h"
    "/home/sarowar/Documents/Linux Experiment/Applications/include/XenUI/UIElement.h"
    "/home/sarowar/Documents/Linux Experiment/Applications/include/XenUI/WindowUtil.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/XenUI" TYPE DIRECTORY FILES "/home/sarowar/Documents/Linux Experiment/Applications/include/XenUI/")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/fonts/XenUI" TYPE FILE FILES "/home/sarowar/Documents/Linux Experiment/Applications/third_party/dejavu-fonts-ttf-2.37/ttf/DejaVuSans.ttf")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/XenUI" TYPE FILE FILES
    "/home/sarowar/Documents/Linux Experiment/Applications/build/XenUIConfig.cmake"
    "/home/sarowar/Documents/Linux Experiment/Applications/build/XenUIConfigVersion.cmake"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/XenUI/XenUITargets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/XenUI/XenUITargets.cmake"
         "/home/sarowar/Documents/Linux Experiment/Applications/build/CMakeFiles/Export/df3cd6c2ed9909535155242cf4620f37/XenUITargets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/XenUI/XenUITargets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/XenUI/XenUITargets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/XenUI" TYPE FILE FILES "/home/sarowar/Documents/Linux Experiment/Applications/build/CMakeFiles/Export/df3cd6c2ed9909535155242cf4620f37/XenUITargets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^()$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/XenUI" TYPE FILE FILES "/home/sarowar/Documents/Linux Experiment/Applications/build/CMakeFiles/Export/df3cd6c2ed9909535155242cf4620f37/XenUITargets-noconfig.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/home/sarowar/Documents/Linux Experiment/Applications/build/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
