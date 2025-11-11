// SPDX-License-Identifier: Apache-2.0
/*
 *  Copyright (C) 2025 MD S M Sarowar Hossain
 *
 * 
 */
// Implements utility functions for accessing and managing the main application window context.
//

#include "WindowUtil.h"

// Global static pointer to the main SDL_Window.
// This allows UI elements to retrieve window-specific properties (like size)
// without explicitly passing the window pointer everywhere.
static SDL_Window* g_window = nullptr;

namespace XenUI {

    /**
     * @brief Stores the reference to the main application SDL_Window.
     *
     * This pointer is used internally by XenonUI components (e.g., layout calculation)
     * that need window dimensions or context. This must be called after the window
     * is created during application setup.
     *
     * @param window The pointer to the application's main SDL_Window instance.
     */
    void SetWindow(SDL_Window* window) {
        g_window = window;
    }

    /**
     * @brief Retrieves the current pixel size of the main application window.
     *
     * This function wraps SDL_GetWindowSize and caches the result for potential
     * performance benefits, although SDL_GetWindowSize is generally fast. It also
     * provides a fallback default size if the window pointer has not been set.
     *
     * @return An SDL_Point structure where x is the width and y is the height of the window.
     */
    SDL_Point GetWindowSize() {
        // Check if the window pointer is valid
        if (g_window) {
            SDL_Point newSize;
            // Retrieve the current window dimensions
            SDL_GetWindowSize(g_window, &newSize.x, &newSize.y);

            // Static variable to store the last known size. Used for caching/comparison.
            static SDL_Point g_lastWindowSize = {800, 600}; // Default starting size

            // Check if the window size has actually changed since the last call.
            // This logic is redundant if called every frame but ensures the variable
            // is only updated when a change occurs.
            if (newSize.x != g_lastWindowSize.x || newSize.y != g_lastWindowSize.y) {
                g_lastWindowSize = newSize;
            }
            return g_lastWindowSize;
        }

        // Return a reasonable default size if the window pointer is null.
        return {800, 600}; // Fallback if window is not set
    }

} // namespace XenUI