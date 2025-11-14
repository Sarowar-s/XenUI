.. _Mac_build_guide:

======================================
Guide to build application on mac os
======================================

    ==================================
    |:Author: MD S M Sarowar Hossain |
    ==================================



Introduction
============

Welcome, gentle reader, to Sarowar's Remarkably Unreliable Guide to build mac application.
This document describes how you can install things needed to build application and also how 
you can apply your codes to build your desired application. This documentation is for moderate
level to experienced C/C++ programmers.

Before you read this, please understand that I never thought I have to write documentation,
but here I am writing because I feel like I would have needed these information to understand properly
how a thing works. I hope this will help you to have some idea about how it works.


Necessary things to install
===========================

At first download XenonUI release version for mac os. Go to `XenonUI for Mac <>`_ and download the 
released version. After that install glm, freetype, and sdl3(sdl3, sdl3_image, sdl3_ttf).
You can install them from HomeBrew.
``brew install glm
brew install freetype
brew install sdl3
brew install sdl3_ttf
brew install sdl3_image``



Build Mac app
==================

Now that you have sdl3, sdl3_ttf, sdl3_image, glm, freetype installed, you can build the mac app now.
To build it, go to the build folder ``{ProjectRoot}Mac/build`` and open a new terminal. Then run
    ``cmake -DCMAKE_BUILD_TYPE=Release ..
    cmake --build . -- -j$(nproc)
``
It will make the CmakeLists file remaining in the project root.
The ``Info.plist`` file insdie the Mac folder holds necessary information about the app



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

It's highly recommended to check the demo files given in ``Mac/Demos/Immediate mode app demo``, 
``Mac/Demos/Retained mode app demo``, ``Mac/Demos/Immediate and Retained mode mix demo app`` inside
the project root.

**All the ui files are given in src folder** Use them as needed.

The header files are inside ``Mac/XenUI/include/XenUI`` folder. Check them to have better understanding of the apis. 












