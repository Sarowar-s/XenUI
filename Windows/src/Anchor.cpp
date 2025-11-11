// SPDX-License-Identifier: Apache-2.0
/*
 *  Copyright (C) 2025 MD S M Sarowar Hossain
 *
 * 
 * 
 * 
 * 
 *
 * This file implements the core logic for spatial anchoring of UI elements.
 * It provides a function to calculate an element's absolute position (in pixels)
 * relative to its parent container, based on a defined Anchor point and offsets.
 *
 * This system abstracts layout management, ensuring elements remain correctly
 * positioned even when the parent container (like a window) is resized.
 */


#include "Anchor.h"           // Defines the Anchor enumeration and related types
#include "WindowUtil.h"       // Provides XenUI::GetWindowSize() if needed, but no longer used here
#include <SDL3/SDL.h>         // Required for SDL_Point structure definition
#include <iostream>           // For potential error logging (currently commented out)

namespace XenUI {

    /**
     * @brief Calculates the absolute pixel position of a UI element based on its anchor point.
     *
     * This function translates a logical anchor (e.g., TOP_RIGHT, CENTER) and pixel offsets
     * into a final (x, y) coordinate pair relative to the parent container's top-left corner.
     *
     * @param anchor The Anchor enumeration value specifying the element's alignment point.
     * @param offsetX The horizontal pixel offset from the calculated anchor point.
     * @param offsetY The vertical pixel offset from the calculated anchor point.
     * @param width The width of the UI element being positioned.
     * @param height The height of the UI element being positioned.
     * @param parentWidth The width of the parent container or window.
     * @param parentHeight The height of the parent container or window.
     * @return SDL_Point A structure containing the final absolute (x, y) pixel coordinates.
     */
    SDL_Point ResolveAnchorPosition(Anchor anchor, int offsetX, int offsetY, int width, int height, int parentWidth, int parentHeight) {
        /*
         * Handle potential error case where parent size might be zero or negative.
         * This can happen during early initialization phases or if the window is minimized.
         * Returning the raw offset ensures the element's position is not dependent
         * on a temporary invalid parent dimension.
         */
        if (parentWidth <= 0 || parentHeight <= 0) {
             // Returning raw offset { offsetX, offsetY } as a safe fallback.
             return { offsetX, offsetY };
        }

        SDL_Point position = {0, 0}; // Initialize position

        /*
         * Use a switch statement to handle all possible anchor combinations.
         *
         * The general formula for a position component (P) is:
         * P = Base_Position + Alignment_Correction + Offset
         *
         * - Base_Position is 0 for TOP/LEFT, (parent_size - element_size) for BOTTOM/RIGHT,
         * and (parent_size / 2) - (element_size / 2) for CENTER.
         * - Alignment_Correction is zero when the anchor already targets the element's
         * top/left corner (e.g., in TOP_LEFT). It is used to align the element's
         * center with the parent's center line.
         */
        switch (anchor) {
            case Anchor::TOP_LEFT:
                // Base position is (0, 0)
                position.x = 0 + offsetX;
                position.y = 0 + offsetY;
                break;

            case Anchor::TOP_RIGHT:
                // Base X position is parentWidth - width to align the right edge
                // Base Y position is 0
                position.x = parentWidth - width + offsetX;
                position.y = 0 + offsetY;
                break;

            case Anchor::BOTTOM_LEFT:
                // Base X position is 0
                // Base Y position is parentHeight - height to align the bottom edge
                position.x = 0 + offsetX;
                position.y = parentHeight - height + offsetY;
                break;

            case Anchor::BOTTOM_RIGHT:
                // Base X position is parentWidth - width
                // Base Y position is parentHeight - height
                position.x = parentWidth - width + offsetX;
                position.y = parentHeight - height + offsetY;
                break;

            case Anchor::CENTER:
                // Base X position is (parentWidth / 2) - (width / 2)
                // Base Y position is (parentHeight / 2) - (height / 2)
                position.x = (parentWidth / 2) - (width / 2) + offsetX;
                position.y = (parentHeight / 2) - (height / 2) + offsetY;
                break;

            case Anchor::TOP_CENTER:
                // X center alignment, Y top alignment (0)
                position.x = (parentWidth / 2) - (width / 2) + offsetX;
                position.y = 0 + offsetY;
                break;

            case Anchor::BOTTOM_CENTER:
                // X center alignment, Y bottom alignment (parentHeight - height)
                position.x = (parentWidth / 2) - (width / 2) + offsetX;
                position.y = parentHeight - height + offsetY;
                break;

            case Anchor::CENTER_LEFT:
                // X left alignment (0), Y center alignment
                position.x = 0 + offsetX;
                position.y = (parentHeight / 2) - (height / 2) + offsetY;
                break;

            case Anchor::CENTER_RIGHT:
                // X right alignment (parentWidth - width), Y center alignment
                position.x = parentWidth - width + offsetX;
                position.y = (parentHeight / 2) - (height / 2) + offsetY;
                break;

            default:
                /*
                 * Unhandled anchor: This should ideally not occur if all enum values are mapped.
                 * Defaulting to TOP_LEFT behavior (position = {0, 0}) + offsets.
                 */
                position.x = offsetX;
                position.y = offsetY;
                break;
        }

        return position;
    }

} // namespace XenUI