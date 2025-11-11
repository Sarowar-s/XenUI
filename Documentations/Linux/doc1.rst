.. _Linux_build_guide:

======================================
Guide to build application on linux os
======================================

    ==================================
    |:Author: MD S M Sarowar Hossain |
    ==================================



Introduction
============

Welcome, gentle reader, to Sarowar's Remarkably Unreliable Guide to build linux application.
This document describes how you can install things needed to build application when in linux os 
and also how you can apply your codes to build your desired application. This documentation is for moderate
level to experienced C/C++ programmers.

Before you read this, please understand that I never thought I have to write documentation,
but here I am writing because I feel like I would have needed these information to understand properly
how a thing works. I hope this will help you to have some idea about how it works.


Necessary things to install
===========================

At first download XenonUI release version for linux os. Go to `XenonUI for Linux <>`_ and download the 
released version. After that install glm, freetype, and sdl3(sdl3, sdl3_image, sdl3_ttf).
Install glm-dev using ``sudo apt install libglm-dev`` and install freetype-dev using ``sudo apt isntall libfreetype-dev``.

Since sld3, sdl3_image, sdl3_ttf might not available in package managers you may need to build it from its github 
repository. For that complexity, I attached the libs and headers files which I bundled with this project, inside 
``{ProjectRoot}Linux/tools`` there you will find version SDL3-3.2.16, SDL3_ttf-3.2.2, SDL3_image-3.2.4
You can either use them or build your own. The process is mentioned bellow on how you can build your own
sdl3 libs.


Build sdl3
----------
Follow these steps to build sdl3, sdl3_image, sdl3_ttf from the github repository.

``sudo apt install git cmake ninja-build build-essential
# SDL3
    git clone https://github.com/libsdl-org/SDL.git -b main SDL3
    cd SDL3
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DSDL_STATIC=OFF
    cmake --build build -j$(nproc)
    sudo cmake --install build
    cd ..
# SDL3_image
    git clone https://github.com/libsdl-org/SDL_image.git -b main
    cd SDL_image
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build -j$(nproc)
    sudo cmake --install build
    cd ..
# SDL3_ttf
    git clone https://github.com/libsdl-org/SDL_ttf.git -b main
    cd SDL_ttf
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build -j$(nproc)
    sudo cmake --install build
    cd ..``

Whatever or from wherever you use sdl, mention it in the cmakefile relative to that tools folder in the project root.
In line `50, 51, 52` in the CmakeLists file ``set(SDL3_ROOT         "${CMAKE_SOURCE_DIR}/tools/SDL3-3.2.16")
    set(SDL3_TTF_ROOT     "${CMAKE_SOURCE_DIR}/tools/SDL3_ttf-3.2.2")
    set(SDL3_IMAGE_ROOT   "${CMAKE_SOURCE_DIR}/tools/SDL3_image-3.2.4")``

`SDL3_image is not required unless you use it`

Here change the path based on where your sdl libs are locating. Or use this default one for using XenonUI provided libs.

**If you dont build sdl3 on your own, then you can skip modifying the cmakefile I mentioned above. And you also dont have to
build the XenonUI again. Just skip the next step of building XenonUI**



Build XenonUI libs
==================

.. Note::
    Skip if you are not building libXenUI.so, and this build is not required by default.


Now that you have sdl3, sdl3_ttf, sdl3_image, glm, freetype installed, you can build libXenUI.so libs.
To build it, go to the build folder ``{ProjectRoot}Linux/build`` and open a new terminal. Then run
    ``cmake -DCMAKE_BUILD_TYPE=Release ..
    cmake --build . -- -j$(nproc)
``

It will make the CmakeLists.txt file inside the project root(Linux folder).
And libXenUI.so/.1/.1.0.0 should be there inside the build folder. And ready to be called from anywhere.


Build linux executable
======================

Since XenonUI is there inside build folder, now you are ready to build linux executable binary. Go to
``Linux/Your project/Test_files`` and create a new cpp file or use the given linux.cpp file. This folder is the 
place where you apply your application design logic. Check the demo cpp file(linux.cpp) given there.

Now build the linux binary using the CmakeLists.txt file relative to the linux.cpp file.
Go to the build folder and open a new terminal and run,
    ``cmake -DCMAKE_BUILD_TYPE=Release ..
    cmake --build . -- -j$(nproc)
    ``

A linux executable binary(test) should be there inside that build folder. The "test" name is according to the name 
set inside the CmakeLists file. You can test, debug the application checking this linux executable "test".


Build .deb file
===============

Once you check the cmakefile is building the linux excutable, you are ready to wrap it and necessary libs, assets in a
.deb file. To do so, go to ``Linux/Your project/Test_files`` and run the ``make_deb.sh`` bash file in a terminal. And
it should make a .deb file there inside ``Test_files`` folder. You can choose your app logo by keeping a logo.png file
relative to the make_deb.sh file. 



Available UIs
=============

- Immediate mode
    - Label
    - Button
    - Radio Button
    - Check Box
    - Dropdown menu
    - Image
    - Scroll View
    - Shapes(rectangle, circle)
    - Slider
    - Switch button


- Retained mode
    - Label
    - Button
    - Radio Button
    - Check Box
    - Dropdown menu
    - Image
    - Inputbox
    - Scroll View
    - Shapes(rectangle, circle)
    - Slider
    - Switch button




How to use the UIs
==================

It's highly recommended to check the demo files given in ``Linux/Your project/Immediate mode app demo``, 
``Linux/Your project/Retained mode app demo``, ``Linux/Your project/Immediate and Retained mode mix demo app`` besides 
``Test_files`` folder.

**All the ui files are given in src folder** Use them as needed.

The header files are inside ``Linux/include/XenUI`` folder. Check them to have better understanding of the apis. 

.. Warning::
    Do not directly change the header files and try to compile them. The libXenUI.so lib won't know about that change and 
    it will throw error. If you must change some value or things inside a header file, recompile the libXenUI from scratch
    then build you project again. 










