// SPDX-License-Identifier: Apache-2.0
/*
 *  Copyright (C) 2025 MD S M Sarowar Hossain
 *
 * 
 */

package com.example.myapp;

import org.libsdl.app.SDLActivity; 

/**
 * MainActivity is a thin wrapper over SDLActivity.
 * Its sole purpose is to serve as the entry point for the Android system
 * and to load the native SDL library. All application logic resides in C++.
 */
public class MainActivity extends SDLActivity {

    
    static {
        System.loadLibrary("main"); // This should match TARGET_NAME in CMakeLists.txt
    }

    // You can override methods here if you need specific Android lifecycle events
    // but for a basic SDL app, SDLActivity handles most of it.
}