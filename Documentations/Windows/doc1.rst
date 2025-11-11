.. _Windows_build_guide:

========================================
Guide to build application on windows os
========================================

    ==================================
    |:Author: MD S M Sarowar Hossain |
    ==================================



Introduction
============

Welcome, gentle reader, to Sarowar's Remarkably Unreliable Guide to build windows application.
This document describes how you can install things needed to build application and also how 
you can apply your codes to build your desired application. This documentation is for moderate
level to experienced C/C++ programmers.

Before you read this, please understand that I never thought I have to write documentation,
but here I am writing because I feel like I would have needed these information to understand properly
how a thing works. I hope this will help you to have some idea about how it works.


Necessary things to install
===========================

    **`All builds and experiments were done in msys2 mingw terminal, not the native cmd`**
    
At first download XenonUI release version for windows os. Go to `XenonUI for Windows <>`_ and download the 
released version. Then install the msys2 terminal. After that install glm, freetype in the msys2 terminal.
`You need to install necessary things of msys2 terminal in /c/mingw64`
Now you are ready to build the windows executable. Follow the next step-


Build the windows executable (.exe)
===================================
Once everything is downloaded you can build the windows executable `xenonui_test.exe`. To do so
go to the ``Windows/examples/Test_files/build`` in your msys2 mingw terminal and run

    ``export_path="../../../install"
    cmake .. -G Ninja -DCMAKE_PREFIX_PATH="${export_path}""
    cmake --build .``

Here the `export_path` will look for `libXenUI.dll` and other necessary things needed to build the `.exe`.

Once it builds successfully, a `xenonui_test.exe` executable file will be generated in the build folder.
You can run it to debug or check your application.

Setup the .exe installer
========================
Once the .exe file generates successfully, you are ready to wrap it into an installer. To do this you can use 
inno setup. You can download it and install it in your computer if not available already. 

After installing it, it should be somewhere like ``/c/Program\ Files\ \(x86\)/Inno\ Setup\ 6/ISCC.exe``.
To make the .exe installer go to ``Windows/examples/Test_files`` and run,
    ``/c/Program\ Files\ \(x86\)/Inno\ Setup\ 6/ISCC.exe "setup.iss"``

It will take the setup.iss file for necessary inforamtion and wrap your app in an installer. 
Once its done, your application is ready to be installed in a windows machine.


Build ``libXenUI.dll`` (Needed only if you change XenUI provided cpp/h files)
=============================================================================
If you change any file and want to recompile XenUI you can follow this.
----------------------------------------------------------------------------
At first make sure your created file is mentioned in the ``CmakeLists.txt`` file. Then go to ``Windows/build`` 
and open a terminal in that folder. Then you need to set a path for the installation location. That means after 
building the ``libXenUI.dll `` it will be located there with necessary things also. So, mentioning the path first is needed. 
Apply,
    ``export_path="../install"
    cmake .. -G Ninja -DCMAKE_PREFIX_PATH="C:/mingw64" -DCMAKE_INSTALL_PREFIX="${export_path}"
    cmake --build .
    cmake --install . ``




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

It's highly recommended to check the demo files given in ``Windows/examples/demos/Immediate mode app demo``, 
``Windows/examples/demos/Retained mode app demo``, ``Windows/examples/demos/Immediate and Retained mode mix demo app`` besides 
``Test_files`` folder.

**All the ui files are given in src folder** Use them as needed.

The header files are inside ``Windows/include/XenUI`` folder. Check them to have better understanding of the apis. 

.. Warning::
    Do not directly change the header files and try to compile them. The libXenUI.dll lib won't know about that change and 
    it will throw error. If you must change some value or things inside a header file, recompile the libXenUI from scratch
    then build you project again. 


.. TL;DR::

    .. Note::

        I dont know how much this documentation will help, I did my setup a long time ago with so much grinding and struggle, 
        and so I barely remember where I did what. After writing, I read this documentation myself, and found the solution is too complex
        for newbies. And maybe experienced wizards will find this explanation troublesome. And I wish I could do better. Plese be gentle and 
        read it with a calm mind.  