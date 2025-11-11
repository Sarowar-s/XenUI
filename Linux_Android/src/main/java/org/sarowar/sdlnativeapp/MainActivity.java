package org.sarowar.sdlnativeapp;

import org.libsdl.app.SDLActivity;

/**
 * A minimal Java shim that loads the native C++ code
 * and hands off control to the SDL framework.
 */
public class MainActivity extends SDLActivity {
    // This static block is executed when the class is loaded.
    // It loads our compiled C++ library, named "libmain.so".
    static {
        System.loadLibrary("main");
    }
}