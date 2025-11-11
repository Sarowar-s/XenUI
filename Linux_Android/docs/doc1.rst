.. _Linux_android_build_guide:

====================================================
Guide to build application from linux to android apk
====================================================

    ==================================
    |:Author: MD S M Sarowar Hossain |
    ==================================



Introduction
============

Welcome, gentle reader, to Sarowar's Remarkably Unreliable Guide to build android application.
This document describes how you can install things needed to build application and also how 
you can apply your codes to build your desired application. This documentation is for moderate
level to experienced C/C++ programmers.

Before you read this, please understand that I never thought I have to write documentation,
but here I am writing because I feel like I would have needed these information to understand properly
how a thing works. I hope this will help you to have some idea about how it works.


Necessary things to install
===========================

At first download XenonUI release version for Android os. Go to `XenonUI for Android <>`_ and download the 
``linux_android`` released version. After that install glm, freetype, and sdl3 android(sdl3, sdl3_image, sdl3_ttf).
Install glm-dev using ``sudo apt install libglm-dev`` and install freetype-dev using ``sudo apt install libfreetype-dev``.

Since sld3, sdl3_image, sdl3_ttf might not available in package managers you may need to build it from its github 
repository. For that complexity, I attached the libs and headers files which I bundled with this project inside 
``{ProjectRoot}Linux_Android/jni/SDL3/lib`` there you will find version SDL3-devel-3.2.10 build version for all abi of android.
You can either use them or build your own.


Whatever or from wherever you use sdl, mention it in the cmakefile relative to the jni folder.

`SDL3_image is not required unless you use it`

Android ndk, sdk, jdk
---------------------

Download Android ndk, sdk and jdk and put them somewhere where you can easily mention them from the ``build.sh`` file later.
Since the versions of these development kits matter, so its viable to mention that, I was using-
``android-ndk-r27c``
``jdk-23.0.2``
``sdk-34.0.0``

`If you download different versions and find trouble compiling and wrapping the apk, try recompiling everything with your current versions.
That includes sdl also. I mean you need to recompile sdl3-android also with your current version of jdk. Also replace the classes.jar with your
updated one in jni/SDL3`

Place the android ndk somewhere and include the path of it in your ``.bashrc`` file.
run ``nano \.bashrc`` command and at the very last of the file write, ``export ANDROID_NDK=/path/to/your/ndk``

You also need to build the sdk using jdk. (If you have no knowledge about these, read some documentations about that or watch videos
on how to setup ndk, sdk and jdk, for better understanding).

Build libmain.so
----------------
Once you have everything in place you are ready to build the XenonUI lib libmain.so
Go to the build folder and open a terminal, and run ``cmake -S . -B build \
    -DANDROID_ABI=arm64-v8a \
    -DANDROID_PLATFORM=21 \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK/build/cmake/android.toolchain.cmake"``

And then build it, ``cmake --build build --parallel $(nproc)``

    `Change the abi name and build type as you need`. 


Build android application (.apk)
================================
Once you built the libmain.so, you are ready to build the .apk. To do so, go to the ``jni`` folder and run the bash file, applying
``bash build.sh arm64-v8a`` 
You need to pass the **abi** name also which you used to build the **libmain.so** file a moment ago.
If it ask for necessary info, provide them, and if it ask for password, just use ``android`` everywhere, for test cases.

Once it successfully built, your apk file will be inside the folder ``Linux_Android/output_apk/{abi}.apk``. Here abi is the name
you used to compile and build the app. Now your .apk application is ready to be installed in an android device.

    Modern android devices are 64 bit, which is **arm64-v8a**. But if you have an older android device or Android Go version, with a 
    32 bit architechture, then you need to apply **armeabi-v7a** as the abi name to both compile and build the app.

Other available abis are, x86, x86_64




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









