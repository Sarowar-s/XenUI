# Install script for directory: C:/Users/Sarowar/Documents/Windows/test/Windows mingw

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/dev/install/XenUI")
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

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "C:/msys64/mingw64/bin/objdump.exe")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY OPTIONAL FILES "C:/Users/Sarowar/Documents/Windows/test/Windows mingw/build/libXenUI.dll.a")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE SHARED_LIBRARY FILES "C:/Users/Sarowar/Documents/Windows/test/Windows mingw/build/libXenUI.dll")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/libXenUI.dll" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/libXenUI.dll")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "C:/msys64/mingw64/bin/strip.exe" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/libXenUI.dll")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/XenUI" TYPE FILE FILES
    "C:/Users/Sarowar/Documents/Windows/test/Windows mingw/include/XenUI/Anchor.h"
    "C:/Users/Sarowar/Documents/Windows/test/Windows mingw/include/XenUI/Button.h"
    "C:/Users/Sarowar/Documents/Windows/test/Windows mingw/include/XenUI/CheckBox.h"
    "C:/Users/Sarowar/Documents/Windows/test/Windows mingw/include/XenUI/Dropdown.h"
    "C:/Users/Sarowar/Documents/Windows/test/Windows mingw/include/XenUI/Image.h"
    "C:/Users/Sarowar/Documents/Windows/test/Windows mingw/include/XenUI/InputBox.h"
    "C:/Users/Sarowar/Documents/Windows/test/Windows mingw/include/XenUI/Label.h"
    "C:/Users/Sarowar/Documents/Windows/test/Windows mingw/include/XenUI/Orientation.h"
    "C:/Users/Sarowar/Documents/Windows/test/Windows mingw/include/XenUI/Position.h"
    "C:/Users/Sarowar/Documents/Windows/test/Windows mingw/include/XenUI/RadioButton.h"
    "C:/Users/Sarowar/Documents/Windows/test/Windows mingw/include/XenUI/ScrollView.h"
    "C:/Users/Sarowar/Documents/Windows/test/Windows mingw/include/XenUI/Shape.h"
    "C:/Users/Sarowar/Documents/Windows/test/Windows mingw/include/XenUI/Slider.h"
    "C:/Users/Sarowar/Documents/Windows/test/Windows mingw/include/XenUI/Switch.h"
    "C:/Users/Sarowar/Documents/Windows/test/Windows mingw/include/XenUI/TextRenderer.h"
    "C:/Users/Sarowar/Documents/Windows/test/Windows mingw/include/XenUI/UIElement.h"
    "C:/Users/Sarowar/Documents/Windows/test/Windows mingw/include/XenUI/WindowUtil.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/XenUI" TYPE DIRECTORY FILES "C:/Users/Sarowar/Documents/Windows/test/Windows mingw/include/XenUI/")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/fonts/XenUI" TYPE FILE FILES "C:/Users/Sarowar/Documents/Windows/test/Windows mingw/third_party/dejavu-fonts-ttf-2.37/ttf/DejaVuSans.ttf")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/XenUI" TYPE FILE FILES
    "C:/Users/Sarowar/Documents/Windows/test/Windows mingw/build/XenUIConfig.cmake"
    "C:/Users/Sarowar/Documents/Windows/test/Windows mingw/build/XenUIConfigVersion.cmake"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/XenUI/XenUITargets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/XenUI/XenUITargets.cmake"
         "C:/Users/Sarowar/Documents/Windows/test/Windows mingw/build/CMakeFiles/Export/df3cd6c2ed9909535155242cf4620f37/XenUITargets.cmake")
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
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/XenUI" TYPE FILE FILES "C:/Users/Sarowar/Documents/Windows/test/Windows mingw/build/CMakeFiles/Export/df3cd6c2ed9909535155242cf4620f37/XenUITargets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^()$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/XenUI" TYPE FILE FILES "C:/Users/Sarowar/Documents/Windows/test/Windows mingw/build/CMakeFiles/Export/df3cd6c2ed9909535155242cf4620f37/XenUITargets-noconfig.cmake")
  endif()
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "C:/Users/Sarowar/Documents/Windows/test/Windows mingw/build/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
if(CMAKE_INSTALL_COMPONENT)
  if(CMAKE_INSTALL_COMPONENT MATCHES "^[a-zA-Z0-9_.+-]+$")
    set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
  else()
    string(MD5 CMAKE_INST_COMP_HASH "${CMAKE_INSTALL_COMPONENT}")
    set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INST_COMP_HASH}.txt")
    unset(CMAKE_INST_COMP_HASH)
  endif()
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "C:/Users/Sarowar/Documents/Windows/test/Windows mingw/build/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
