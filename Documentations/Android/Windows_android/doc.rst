.. _Windows_android_build_guide:

======================================================
Guide to build application from windows to android apk
======================================================

    ==================================
    |:Author: MD S M Sarowar Hossain |
    ==================================



Introduction
============

Welcome, gentle reader, to Sarowar's Remarkably Unreliable Guide to build android application.
This document describes how you can install things needed to build application when in windows os
and also how you can apply your codes to build your desired application. This documentation is for moderate
level to experienced C/C++ programmers.

Before you read this, please understand that I never thought I have to write documentation,
but here I am writing because I feel like I would have needed these information to understand properly
how a thing works. I hope this will help you to have some idea about how it works.


Necessary things to install
===========================

At first download XenonUI release version for Android os. Go to `XenonUI for Android <>`_ and download the 
``windows_android`` released version. After that install glm, freetype, and sdl3 android(sdl3, sdl3_image, sdl3_ttf).
Download glm from its github repository and download freetype also.

Since sld3, sdl3_image, sdl3_ttf might not available in package managers you may need to build it from its github 
repository. For that complexity, I attached the libs and headers files which I bundled with this project inside 
``{ProjectRoot}Windows_Android/lib`` there you will find version SDL3, SDL3_ttf, SDL3_image for all abi of android.
You can either use them or build your own.


Whatever or from wherever you use sdl, mention it in the cmakefile which is located in the project root folder.

`SDL3_image is not required unless you use it`

Android ndk, sdk, jdk
---------------------

Download Android ndk, sdk and jdk and put them somewhere where you can easily mention them from the ``build.bat`` file later.
Since the versions of these development kits matter, so its viable to mention that, I was using-
``android-ndk-r27c``
``jdk-24.0.2``
``sdk-34.0.0``

`If you download different versions and find trouble compiling and wrapping the apk, try recompiling everything with your current versions.
That includes sdl also. I mean you need to recompile sdl3-android also with your current version of jdk. Also replace the classes.jar with your
updated one in lib/classes.jar`

*You can mention your ndk, sdk and jdk path in the `local.properties` file in the project root folder. *


Place the android ndk somewhere and include the path of it in your ``Environment variable``.


You also need to download 7zip for ziping the application. And mention its path in the ``scripts/build.bat`` file.

You also need to build the sdk at first.



Build android application (.apk) using native cmd
=================================================
You can compile and wrap the application only by running the ``build.bat`` file located in ``Windows_Android/scripts``.
Apply, 
    ``build.bat --abi arm64-v8a --release``

During the build if it asks for inforamtion, just provide them, and if it asks for password, just pass ``android`` everywhere. 
For test cases.

    Modern android devices are 64 bit, which is **arm64-v8a**. But if you have an older android device or Android Go version, with a 
    32 bit architechture, then you need to apply **armeabi-v7a** as the abi name to build the app.

Other available abis are, x86, x86_64

To show available commands, apply ``build.bat --help``



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

It's highly recommended to check the demo files given in ``Linux_Android/Demos/Immediate mode app demo``, 
``Linux_Android/Demos/Retained mode app demo``, ``Linux_Android/Demos/Immediate and Retained mode mix demo app``
to have a better understanding on how to use the uis.

**All the ui files are given in jni/src folder** Use them as needed.

The main.cpp file is in, **jni/src** folder also. This is the file where you design your application.









