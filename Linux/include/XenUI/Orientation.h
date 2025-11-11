// SPDX-License-Identifier: Apache-2.0
/*
 *  Copyright (C) 2025 MD S M Sarowar Hossain
 */
//
// Defines the enumeration for common UI element orientations.
//
#ifndef XENUI_ORIENTATION_H
#define XENUI_ORIENTATION_H

namespace XenUI {
    /**
     * @brief Defines the principal axis of arrangement or movement for a UI element.
     *
     * This enum is used by components such as sliders, scrollbars, and layout managers
     * to determine their primary direction.
     */
    enum class Orientation {
        /** Elements are arranged or move along the horizontal axis (left-right). */
        HORIZONTAL,
        /** Elements are arranged or move along the vertical axis (up-down). */
        VERTICAL
    };
} // namespace XenUI

#endif // XENUI_ORIENTATION_H