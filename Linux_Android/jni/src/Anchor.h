#ifndef ANCHOR_H
#define ANCHOR_H

// SPDX-License-Identifier: Apache-2.0
/*
 *  Copyright (C) 2025 MD S M Sarowar Hossain
 *
 * 
 *
 * 
 *
 *
 * This header defines the Anchor enumeration used for spatial layout
 * within the XenUI framework. It also declares the core function
 * for translating these logical anchors and offsets into absolute
 * pixel coordinates.
 */

#include <SDL3/SDL.h>

namespace XenUI {
    
/**
 * @brief Defines the available anchoring points for a UI element relative to its parent container.
 *
 * Anchors are used to automatically calculate an element's position when the parent
 * container (e.g., a Window or another component) is resized. The element's
 * logical anchor point (e.g., its top-right corner, or its center) will be fixed
 * relative to the parent's corresponding anchor point.
 */
enum class Anchor {
    /** Element's top-left corner is aligned with the parent's top-left corner. */
    TOP_LEFT,
    /** Element's top-right corner is aligned with the parent's top-right corner. */
    TOP_RIGHT,
    /** Element's bottom-left corner is aligned with the parent's bottom-left corner. */
    BOTTOM_LEFT,
    /** Element's bottom-right corner is aligned with the parent's bottom-right corner. */
    BOTTOM_RIGHT,
    /** Element's center is aligned with the parent's center. */
    CENTER,
    /** Element's top-center point is aligned with the parent's top-center point. */
    TOP_CENTER,
    /** Element's bottom-center point is aligned with the parent's bottom-center point. */
    BOTTOM_CENTER,
    /** Element's center-left point is aligned with the parent's center-left point. */
    CENTER_LEFT,
    /** Element's center-right point is aligned with the parent's center-right point. */
    CENTER_RIGHT
};

/**
 * @brief Converts a logical Anchor and pixel offsets into final absolute coordinates.
 *
 * This function calculates the absolute (x, y) pixel position of a child element's
 * top-left corner relative to its parent's top-left corner (0, 0), taking into account
 * the element's size and the anchor constraints.
 *
 * @param anchor The logical anchor point to use for positioning.
 * @param offsetX The pixel offset to apply horizontally after anchoring is resolved.
 * @param offsetY The pixel offset to apply vertically after anchoring is resolved.
 * @param width The width of the element being positioned.
 * @param height The height of the element being positioned.
 * @param parentWidth The width of the parent container.
 * @param parentHeight The height of the parent container.
 * @return SDL_Point A structure containing the final absolute (x, y) coordinates
 * of the element's top-left corner.
 */
SDL_Point ResolveAnchorPosition(Anchor anchor, int offsetX, int offsetY, int width, int height, int parentWidth, int parentHeight);
}
#endif