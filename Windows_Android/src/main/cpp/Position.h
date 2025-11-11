// SPDX-License-Identifier: Apache-2.0
/*
 *  Copyright (C) 2025 MD S M Sarowar Hossain
 *
 * 
 */
// Defines structures and functions for resolving the final content-space position
// of a UI element based on either absolute coordinates or anchor points.
//
#ifndef XENUI_POSITION_H
#define XENUI_POSITION_H

#include "Anchor.h"     // Defines the Anchor enumeration
#include "WindowUtil.h" // Provides XenUI::GetWindowSize dependency
#include<algorithm>     // For std::max
#include <SDL3/SDL.h>

namespace XenUI {

    // Forward declaration for the function that computes the final position based on an Anchor.
    // (Implementation assumed to be in WindowUtil.cpp or similar).
    SDL_Point ResolveAnchorPosition(Anchor anchor, int offsetX, int offsetY, int width, int height, int parentWidth, int parentHeight);

    /**
     * @brief Defines how an element's position is calculated.
     */
    enum class PositionMode {
        /** Position is defined by direct (x, y) coordinates relative to the parent's origin. */
        ABSOLUTE,
        /** Position is determined relative to an anchor point within the parent. */
        ANCHORED
    };

    /**
     * @brief Parameters required to fully define a UI element's size and position within its parent.
     */
    struct PositionParams {
        PositionMode mode = PositionMode::ANCHORED; ///< The method used to determine position. Defaults to anchored.

        // --- Sizing ---
        /** Explicit width for the element in content-space. A value of 0 often means "auto-size based on content". */
        int width = 0;
        /** Explicit height for the element in content-space. */
        int height = 0;

        // --- Absolute Mode Fields ---
        int x = 0; ///< X coordinate in absolute mode.
        int y = 0; ///< Y coordinate in absolute mode.

        // --- Anchored Mode Fields ---
        Anchor anchor = Anchor::TOP_LEFT; ///< The reference point within the parent for anchored mode. Defaults to top-left.
        int relOffsetX = 0; ///< Horizontal offset from the resolved anchor point.
        int relOffsetY = 0; ///< Vertical offset from the resolved anchor point.

        // --- Convenience Static "Constructors" ---

        /**
         * @brief Creates parameters for ABSOLUTE positioning.
         *
         * @param absX The absolute X coordinate.
         * @param absY The absolute Y coordinate.
         * @param w The desired width (0 for auto-size/content-determined).
         * @param h The desired height.
         * @return A PositionParams structure configured for absolute mode.
         */
        static PositionParams Absolute(int absX, int absY, int w = 0, int h = 0) {
            PositionParams p;
            p.mode = PositionMode::ABSOLUTE;
            p.x = absX;
            p.y = absY;
            p.width = w;
            p.height = h;
            return p;
        }

        /**
         * @brief Creates parameters for ANCHORED positioning.
         *
         * @param anch The anchor point within the parent.
         * @param rX The horizontal offset from the anchor point.
         * @param rY The vertical offset from the anchor point.
         * @param w The desired width (0 for auto-size/content-determined).
         * @param h The desired height.
         * @return A PositionParams structure configured for anchored mode.
         */
        static PositionParams Anchored(Anchor anch, int rX = 0, int rY = 0, int w = 0, int h = 0) {
            PositionParams p;
            p.mode = PositionMode::ANCHORED;
            p.anchor = anch;
            p.relOffsetX = rX;
            p.relOffsetY = rY;
            p.width = w;
            p.height = h;
            return p;
        }
    };

    // forward-declare overload that takes parent dimensions
    inline SDL_Point CalculateFinalPosition(const PositionParams& params,
                                            int elementWidth,
                                            int elementHeight,
                                            int parentWidth,
                                            int parentHeight);

    // --- Central Calculation Functions ---

    /**
     * @brief Calculates the final content-space position using window size as the parent dimensions.
     *
     * This is a convenience overload suitable for top-level elements or when parent size is unknown.
     *
     * @param params The PositionParams defining the mode, anchor, and offsets.
     * @param elementWidth The calculated width of the element's content.
     * @param elementHeight The calculated height of the element's content.
     * @return The final top-left (x, y) coordinates in content-space.
     */
    inline SDL_Point CalculateFinalPosition(const PositionParams& params, int elementWidth, int elementHeight) {
        SDL_Point windowSize = XenUI::GetWindowSize();
        return CalculateFinalPosition(params, elementWidth, elementHeight, windowSize.x, windowSize.y);
    }

    /**
     * @brief Calculates the final content-space position using explicit parent dimensions.
     *
     * This is the main implementation used when positioning elements inside containers like ScrollView.
     * Handles fallbacks for invalid parent dimensions by using window size as a last resort.
     *
     * @param params The PositionParams defining the mode, anchor, and offsets.
     * @param elementWidth The calculated width of the element's content.
     * @param elementHeight The calculated height of the element's content.
     * @param parentWidth The width of the element's parent content area.
     * @param parentHeight The height of the element's parent content area.
     * @return The final top-left (x, y) coordinates in content-space.
     */
    inline SDL_Point CalculateFinalPosition(const PositionParams& params, int elementWidth, int elementHeight, int parentWidth, int parentHeight) {
        if (params.mode == PositionMode::ABSOLUTE) {
            // For absolute mode, the final position is simply the stored x/y values
            return { params.x, params.y };
        } else { // ANCHORED
            // Handle invalid parent dimensions by attempting a fallback to window size
            if (parentWidth <= 0 || parentHeight <= 0) {
                SDL_Point win = XenUI::GetWindowSize();
                if (win.x > 0 && win.y > 0) {
                    // Use window size if parent dimensions are missing or invalid
                    parentWidth  = parentWidth  > 0 ? parentWidth  : win.x;
                    parentHeight = parentHeight > 0 ? parentHeight : win.y;
                } else {
                    // Last resort to prevent division by zero in anchor resolution logic
                    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                                "ResolveAnchorPosition: parent size invalid (%d x %d), falling back to (1,1).",
                                parentWidth, parentHeight);
                    parentWidth  = std::max(1, parentWidth);
                    parentHeight = std::max(1, parentHeight);
                }
            }

            // Delegate anchor calculation to the dedicated function
            return ResolveAnchorPosition(params.anchor, params.relOffsetX, params.relOffsetY, elementWidth, elementHeight, parentWidth, parentHeight);
        }
    }

} // namespace XenUI

#endif // XENUI_POSITION_H