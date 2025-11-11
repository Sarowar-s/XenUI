// SPDX-License-Identifier: Apache-2.0
/*
 *  Copyright (C) 2025 MD S M Sarowar Hossain
 *
 * 
 */
// Defines utility functions for setting and retrieving the main application window context.
// These functions are essential for global layout calculations and context provision.
//

#pragma once
#include <SDL3/SDL.h>

namespace XenUI {
    /**
     * @brief Stores the global reference to the main application SDL_Window.
     *
     * This reference is used by internal components, such as layout managers,
     * to access the current window dimensions for proportional sizing or anchoring.
     * It must be called once during the application's initialization phase.
     *
     * @param window The pointer to the application's main SDL_Window instance.
     */
    void SetWindow(SDL_Window* window);

    /**
     * @brief Retrieves the current pixel size of the main application window.
     *
     * This function queries the size from the internally stored SDL_Window pointer
     * and provides a fallback value if the window has not been set.
     *
     * @return An SDL_Point structure where the 'x' field holds the window's width
     * and the 'y' field holds the window's height.
     */
    SDL_Point GetWindowSize();
}