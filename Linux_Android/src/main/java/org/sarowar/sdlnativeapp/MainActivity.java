// SPDX-License-Identifier: Apache-2.0
/*
 *  Copyright (C) 2025 MD S M Sarowar Hossain
 *
 * 
 */
package org.sarowar.sdlnativeapp;

import org.libsdl.app.SDLActivity;

/**
 * A minimal Java shim that loads the native C++ code
 * and hands off control to the SDL framework.
 */
public class MainActivity extends SDLActivity {
    // This static block is executed when the class is loaded.
    // It loads the compiled C++ library, named "libmain.so".
    static {
        System.loadLibrary("main");
    }
}