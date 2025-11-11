// SPDX-License-Identifier: Apache-2.0
/*
 *  Copyright (C) 2025 MD S M Sarowar Hossain
 *
 * 
 * 
 * 
 * 
 */
// Implements the retained-mode ScrollView container, which provides a viewport
// for scrollable content, managing child controls, layout, and scroll mechanics.
//

#include "ScrollView.h"
#include "WindowUtil.h" // Provides XenUI::GetWindowSize
#include <algorithm>    // For std::max and std::clamp
#include <stdexcept>    // For std::invalid_argument
#include <iostream>     // For debugging, consider removing in final build

// --- Retained Mode ScrollView Implementation Starts Here ---

/**
 * @brief Constructs a retained-mode ScrollView instance.
 *
 * Validates that explicit dimensions are provided and performs initial layout calculation.
 *
 * @param posParams The positional constraints (must include explicit width and height).
 * @param style The visual style parameters for the scrollbar.
 * @throws std::invalid_argument if width or height in posParams is non-positive.
 */
ScrollView::ScrollView(const XenUI::PositionParams& posParams, const ScrollViewStyle& style)
    : m_posParams(posParams), m_style(style) {
    // ScrollView requires explicit size to define the viewport area.
    if (m_posParams.width <= 0 || m_posParams.height <= 0) {
        throw std::invalid_argument("ScrollView requires an explicit width and height in PositionParams.");
    }
    // Perform initial layout relative to the current window size (assuming top-level placement)
    SDL_Point winSize = XenUI::GetWindowSize();
    recalculateLayout(winSize.x, winSize.y); // Assume top-level; adjust if nested
}

/**
 * @brief Handles an SDL event with default zero parent offset and no explicit window pointer.
 *
 * This is a convenience wrapper for IControl compatibility when the ScrollView is top-level.
 *
 * @param e The SDL_Event to process.
 * @return true if the event was consumed, false otherwise.
 */
bool ScrollView::handleEvent(const SDL_Event& e) {
    return handleEvent(e, nullptr, SDL_FPoint{0.0f, 0.0f});
}

/**
 * @brief Adds a child control to the ScrollView's content area.
 *
 * Triggers a layout recalculation for the new child relative to the content area
 * and updates the total content height.
 *
 * @param control A unique_ptr owning the IControl instance to add.
 */
void ScrollView::addControl(std::unique_ptr<IControl> control) {
    if (!control) return;
    m_controls.push_back(std::move(control));
    // Recalculate child layout relative to content area dimensions
    // NOTE: Child layout is recalculated using the view width (m_viewRect.w) and the
    // current bounds height (m_bounds.h) for the initial position resolution.
    m_controls.back()->recalculateLayout(static_cast<int>(m_viewRect.w), static_cast<int>(m_bounds.h));
    updateContentHeight();
    // Clamp scroll position after content height update
    float maxScrollY = std::max(0.0f, m_contentHeight - m_viewRect.h);
    m_scrollY = std::clamp(m_scrollY, 0.0f, maxScrollY);
}

/**
 * @brief Calculates the total required height of the scrollable content.
 *
 * The content height is determined by the maximum bottom edge (y + h) of all child controls.
 */
void ScrollView::updateContentHeight() {
    m_contentHeight = 0.0f;
    for (const auto& control : m_controls) {
        SDL_FRect childBounds = control->getBounds();
        m_contentHeight = std::max(m_contentHeight, childBounds.y + childBounds.h);
    }
}


/**
 * @brief Recalculates the ScrollView's own size/position and the layout of its children.
 *
 * This uses a two-pass approach:
 * 1. Measure children using viewport height to determine final content height.
 * 2. Position children using the calculated total content height for anchor resolution.
 *
 * @param parentWidth The width of the ScrollView's parent container.
 * @param parentHeight The height of the ScrollView's parent container.
 */
void ScrollView::recalculateLayout(int parentWidth, int parentHeight) {
    // 0) Resolve ScrollView's own bounds (viewport area on screen)
    int resolvedW = m_posParams.width > 0 ? m_posParams.width : parentWidth;
    int resolvedH = m_posParams.height > 0 ? m_posParams.height : parentHeight;

    SDL_Point finalPos = XenUI::CalculateFinalPosition(m_posParams, resolvedW, resolvedH, parentWidth, parentHeight);
    m_bounds = { static_cast<float>(finalPos.x), static_cast<float>(finalPos.y),
                 static_cast<float>(resolvedW), static_cast<float>(resolvedH) };

    // Define the view area (where children are rendered), reserving space for the scrollbar
    float viewW = std::max(0.0f, m_bounds.w - static_cast<float>(m_style.scrollbarWidth));
    float viewH = std::max(0.0f, m_bounds.h);
    m_viewRect = { m_bounds.x, m_bounds.y, viewW, viewH };

    // ---------- Two-pass Layout Calculation ----------

    // PASS 1: Measurement pass to determine the total content height.
    // Children are laid out using the available viewport dimensions (m_viewRect.w/h)
    int pass1ParentW = static_cast<int>(m_viewRect.w);
    int pass1ParentH = static_cast<int>(m_viewRect.h);

    for (const auto& control : m_controls) {
        // Child controls are positioned relative to the viewport area
        control->recalculateLayout(pass1ParentW, pass1ParentH);
    }

    // Compute the total content height (m_contentHeight) from the measured children's bounds.
    float maxBottom = 0.0f;
    for (const auto& control : m_controls) {
        SDL_FRect b = control->getBounds();
        maxBottom = std::max(maxBottom, b.y + b.h);
    }

    // Assign the measured maximum extent as the content height
    m_contentHeight = maxBottom;

    // Ensure contentHeight is at least the view height, preventing division by zero/negative scroll range
    if (m_contentHeight < m_viewRect.h) {
        m_contentHeight = m_viewRect.h;
    }

    // PASS 2: Final layout pass using the total content height.
    // This allows children using bottom/center anchors (relative to the full content) to position correctly.
    int pass2ParentW = static_cast<int>(m_viewRect.w);
    // Use the viewport height as parentHeight, as positions are relative to the viewport's top/bottom
    int pass2ParentH = static_cast<int>(m_viewRect.h);
    for (const auto& control : m_controls) {
        control->recalculateLayout(pass2ParentW, pass2ParentH);
    }

    // Re-verify content height in case children's sizes or positions changed in the second pass.
    maxBottom = 0.0f;
    for (const auto& control : m_controls) {
        SDL_FRect b = control->getBounds();
        maxBottom = std::max(maxBottom, b.y + b.h);
    }
    m_contentHeight = maxBottom;
    if (m_contentHeight < m_viewRect.h) {
        m_contentHeight = m_viewRect.h;
    }

    // Clamp scrollY to the new valid content range
    float maxScrollY = std::max(0.0f, m_contentHeight - m_viewRect.h);
    m_scrollY = std::clamp(m_scrollY, 0.0f, maxScrollY);
}


/**
 * @brief Processes SDL events, handling scrolling, scrollbar interaction, and forwarding input to children.
 *
 * Events must be translated from screen-space to content-space before being forwarded.
 *
 * @param evt The SDL_Event to process.
 * @param window The target SDL_Window (optional, derived from event/focus if null).
 * @param parentViewOffset The offset of this ScrollView relative to the screen.
 * @return true if the event was handled and consumed, false otherwise.
 */
bool ScrollView::handleEvent(const SDL_Event& evt, SDL_Window* window, const SDL_FPoint& parentViewOffset) {
    // Determine the relevant SDL_Window for coordinate and focus context
    SDL_Window* useWindow = window;
    // Fallback logic to determine the window context if not explicitly provided
    if (!useWindow) {
        if (evt.type == SDL_EVENT_MOUSE_BUTTON_DOWN || evt.type == SDL_EVENT_MOUSE_BUTTON_UP) {
            useWindow = SDL_GetWindowFromID(evt.button.windowID);
        } else {
            useWindow = SDL_GetKeyboardFocus();
            if (!useWindow) useWindow = SDL_GetMouseFocus();
        }
    }

    // --- Robust Mouse Position Resolution (Screen-Space) ---
    SDL_FPoint mousePos = {0.0f, 0.0f};
    bool haveMouseCoords = false;

    // Use event-provided window-relative coordinates first
    switch (evt.type) {
    case SDL_EVENT_MOUSE_MOTION:
        mousePos.x = evt.motion.x;
        mousePos.y = evt.motion.y;
        haveMouseCoords = true;
        break;

    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_BUTTON_UP:
        mousePos.x = evt.button.x;
        mousePos.y = evt.button.y;
        haveMouseCoords = true;
        break;

    case SDL_EVENT_MOUSE_WHEEL:
        // Use current mouse state for wheel event location
        SDL_GetMouseState(&mousePos.x, &mousePos.y);
        haveMouseCoords = true;
        break;

    default:
        break;
    }

    if (!haveMouseCoords) {
        // Fallback: query global cursor and convert to window-relative logical coords.
        float gx = 0.0f, gy = 0.0f;
        SDL_GetGlobalMouseState(&gx, &gy);

        if (useWindow) {
            // Complex conversion logic to handle window borders and HiDPI scaling
            int winX = 0, winY = 0;
            if (SDL_GetWindowPosition(useWindow, &winX, &winY)) {
                int top=0, left=0, bottom=0, right=0;
                SDL_GetWindowBordersSize(useWindow, &top, &left, &bottom, &right);

                float clientOriginXpx = (float)winX + (float)left;
                float clientOriginYpx = (float)winY + (float)top;
                float clientXpx = gx - clientOriginXpx;
                float clientYpx = gy - clientOriginYpx;

                int winW = 0, winH = 0;
                int winWpx = 0, winHpx = 0;
                SDL_GetWindowSize(useWindow, &winW, &winH);
                SDL_GetWindowSizeInPixels(useWindow, &winWpx, &winHpx);

                float scaleX = 1.0f, scaleY = 1.0f;
                if (winW > 0 && winWpx > 0) scaleX = (float)winWpx / (float)winW;
                if (winH > 0 && winHpx > 0) scaleY = (float)winHpx / (float)winH;

                mousePos.x = clientXpx / scaleX;
                mousePos.y = clientYpx / scaleY;
                haveMouseCoords = true;
            }
        }

        // Last fallback: use SDL_GetMouseState (window-relative to focused window)
        if (!haveMouseCoords) {
            float mx=0.0f, my=0.0f;
            SDL_GetMouseState(&mx, &my);
            mousePos.x = mx;
            mousePos.y = my;
            haveMouseCoords = true;
        }
    }
    // --- End Mouse Position Resolution ---

    // Calculate the ScrollView's view rectangle in absolute screen coordinates
    const SDL_FRect absViewRect = { m_viewRect.x + parentViewOffset.x, m_viewRect.y + parentViewOffset.y, m_viewRect.w, m_viewRect.h };

    // --- Scrollbar Interaction Handling (Mouse) ---

    // Check/update hovered state for scrollbar thumb on motion
    if (evt.type == SDL_EVENT_MOUSE_MOTION) {
        const SDL_FRect thumbRel = getScrollbarThumbRect({0.0f, 0.0f});
        // Scrollbar track is immediately to the right of m_viewRect
        const SDL_FRect absThumb = { absViewRect.x + absViewRect.w, absViewRect.y + thumbRel.y, thumbRel.w, thumbRel.h };
        m_isScrollbarHovered = SDL_PointInRectFloat(&mousePos, &absThumb);
    }

    // Handle ongoing scrollbar drag
    if (m_isScrollbarGrabbed) {
        if (evt.type == SDL_EVENT_MOUSE_MOTION) {
            float trackTop = absViewRect.y;
            float trackHeight = absViewRect.h;
            const SDL_FRect thumbRel = getScrollbarThumbRect({0.0f, 0.0f});
            float thumbHeight = thumbRel.h;
            float scrollableTrack = trackHeight - thumbHeight;

            if (scrollableTrack > 0.0f) {
                // Calculate new scroll position based on mouse position and grab offset
                float newThumbYAbs = mousePos.y - m_scrollbarGrabOffsetY;
                // Calculate percentage of track scrolled
                float pct = std::clamp((newThumbYAbs - trackTop) / scrollableTrack, 0.0f, 1.0f);
                float maxScroll = std::max(0.0f, m_contentHeight - m_viewRect.h);
                // Apply percentage to the total scroll range
                m_scrollY = pct * maxScroll;
            }
            return true;
        }

        if (evt.type == SDL_EVENT_MOUSE_BUTTON_UP && evt.button.button == SDL_BUTTON_LEFT) {
            m_isScrollbarGrabbed = false;
            return true;
        }
    }

    // Start new scrollbar drag
    if (evt.type == SDL_EVENT_MOUSE_BUTTON_DOWN && evt.button.button == SDL_BUTTON_LEFT) {
        const SDL_FRect thumbRel = getScrollbarThumbRect({0.0f, 0.0f});
        const SDL_FRect absThumb = { absViewRect.x + absViewRect.w, absViewRect.y + thumbRel.y, thumbRel.w, thumbRel.h };

        if (SDL_PointInRectFloat(&mousePos, &absThumb)) {
            m_isScrollbarGrabbed = true;
            // Record offset from mouse to thumb top for smooth dragging
            m_scrollbarGrabOffsetY = mousePos.y - absThumb.y;
            return true;
        }
    }

    // --- Input Forwarding and Focus Management ---

    // Check if the mouse is inside the ScrollView's content area (viewport)
    const bool isMouseInsideView = SDL_PointInRectFloat(&mousePos, &absViewRect);

    // Unfocus logic: if clicked outside the view, unfocus the active child
    if (evt.type == SDL_EVENT_MOUSE_BUTTON_DOWN && !isMouseInsideView) {
        if (m_focusedChild) {
            m_focusedChild->unfocus(useWindow);
            m_focusedChild = nullptr;
        }
        return false; // Not handled by this ScrollView
    }

    // If mouse event occurred outside the view (and wasn't handled by unfocus logic above), ignore
    if (!isMouseInsideView && evt.type != SDL_EVENT_MOUSE_WHEEL && evt.type != SDL_EVENT_FINGER_DOWN) {
        return false;
    }

    // Mouse wheel scrolling
    if (evt.type == SDL_EVENT_MOUSE_WHEEL) {
        m_scrollY -= evt.wheel.y * 25.0f; // Standard scroll speed multiplier
        m_scrollY = std::clamp(m_scrollY, 0.0f, std::max(0.0f, m_contentHeight - m_viewRect.h));
        return true;
    }

    // Translate screen-space mouse coordinates to ScrollView's content-space
    const SDL_FPoint contentMousePos = { mousePos.x - absViewRect.x, mousePos.y - absViewRect.y + m_scrollY };

    // The screen-space offset required to draw children correctly
    const SDL_FPoint childViewOffset = {
        absViewRect.x,
        absViewRect.y - m_scrollY // Offset to apply to content-space position
    };

    // 1. Determine which child is the target for this mouse event
    IControl* mouseTargetChild = nullptr;
    const bool isMouseEvent = (evt.type == SDL_EVENT_MOUSE_MOTION || evt.type == SDL_EVENT_MOUSE_BUTTON_DOWN || evt.type == SDL_EVENT_MOUSE_BUTTON_UP);

    if (isMouseEvent) {
        // Iterate backwards to prioritize controls rendered on top (higher Z-order)
        for (int i = static_cast<int>(m_controls.size()) - 1; i >= 0; --i) {
            auto &ctrl = m_controls[static_cast<size_t>(i)];
            if (!ctrl) continue;
            // isInside checks against contentMousePos (translated mouse coords) and child's bounds (content-space)
            bool inside = ctrl->isInside(contentMousePos);
            if (inside) {
                mouseTargetChild = ctrl.get();
                break;
            }
        }
    }

    // 2. Handle focus change on MOUSE_BUTTON_DOWN
    if (evt.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        // The target is the child clicked on
        if (mouseTargetChild != m_focusedChild) {
            if (m_focusedChild) {
                m_focusedChild->unfocus(useWindow);
            }
            m_focusedChild = mouseTargetChild;
            if (m_focusedChild) {
                // Set context (window, view offset) and call focus logic
                m_focusedChild->setWindow(useWindow);
                m_focusedChild->setViewOffset(childViewOffset);
                m_focusedChild->focus(useWindow);
            }
        }
    }

    // 3. Forward events to the correct target

    const bool isKeyboardEvent = (evt.type == SDL_EVENT_KEY_DOWN || evt.type == SDL_EVENT_KEY_UP ||
                                  evt.type == SDL_EVENT_TEXT_INPUT || evt.type == SDL_EVENT_TEXT_EDITING);

    // Keyboard/Text events go to the focused child (if any)
    if (isKeyboardEvent && m_focusedChild) {
        // Pass the childViewOffset so the child can correctly position its IME box
        if (m_focusedChild->handleEvent(evt, useWindow, childViewOffset)) {
            return true;
        }
    }
    // Mouse events go to the child currently under the cursor (if any)
    else if (isMouseEvent && mouseTargetChild) {
        // Translate the mouse event coordinates into the child's content space
        SDL_Event translatedEvt = evt;
        if (evt.type == SDL_EVENT_MOUSE_MOTION) {
            translatedEvt.motion.x = static_cast<Sint32>(contentMousePos.x);
            translatedEvt.motion.y = static_cast<Sint32>(contentMousePos.y);
        } else if (evt.type == SDL_EVENT_MOUSE_BUTTON_DOWN || evt.type == SDL_EVENT_MOUSE_BUTTON_UP) {
            translatedEvt.button.x = static_cast<Sint32>(contentMousePos.x);
            translatedEvt.button.y = static_cast<Sint32>(contentMousePos.y);
        }

        // Forward the translated event
        if (mouseTargetChild->handleEvent(translatedEvt, useWindow, childViewOffset)) {
            return true;
        }
    }


    // --- Touch/Finger Event Handling (SDL3) ---

    // Simple finger-to-scroll implementation
    if (evt.type == SDL_EVENT_FINGER_DOWN || evt.type == SDL_EVENT_FINGER_MOTION || evt.type == SDL_EVENT_FINGER_UP) {
        SDL_Window* w = useWindow ? useWindow : SDL_GetKeyboardFocus();
        int winW = 0, winH = 0;
        if (w) SDL_GetWindowSize(w, &winW, &winH);

        // Get normalized finger coords (0..1)
        float fx = evt.tfinger.x;
        float fy = evt.tfinger.y;

        // Convert normalized coords to window logical coordinates
        float touchX = fx * (winW > 0 ? (float)winW : 1.0f);
        float touchY = fy * (winH > 0 ? (float)winH : 1.0f);

        // If the initial touch is outside the ScrollView bounds, ignore it.
        const bool isTouchInsideView = SDL_PointInRectFloat(&mousePos, &absViewRect);

        // Update mousePos for hit-testing based on touch event location
        mousePos.x = touchX;
        mousePos.y = touchY;
        haveMouseCoords = true; // Not strictly needed here, but keeps state consistent

        if (evt.type == SDL_EVENT_FINGER_DOWN && isTouchInsideView) {
            m_touchActive = true;
            m_lastTouchY = touchY;
            m_activeTouchId = evt.tfinger.touchID; // Track touch for multi-touch scenarios
            // Prevent further processing of the down event for children (ScrollViews consume the down event for potential drag)
            return true;
        } else if (evt.type == SDL_EVENT_FINGER_MOTION && m_touchActive && evt.tfinger.touchID == m_activeTouchId) {
            // Drag to scroll: vertical difference moves the content
            float dy = m_lastTouchY - touchY;
            m_scrollY += dy;
            m_scrollY = std::clamp(m_scrollY, 0.0f, std::max(0.0f, m_contentHeight - m_viewRect.h));
            m_lastTouchY = touchY;
            return true; // Event handled: scroll motion
        } else if (evt.type == SDL_EVENT_FINGER_UP && evt.tfinger.touchID == m_activeTouchId) {
            m_touchActive = false;
            m_activeTouchId = 0;
            // Touch up event should fall through to be processed as a mouse up/click on a child if no dragging occurred
        }
    }

    return false; // Event was not handled by the ScrollView or any of its children
}



/**
 * @brief Renders the ScrollView, its background, border, child controls, and scrollbar.
 *
 * This function sets up a clipping rectangle based on the view area before drawing
 * the children to ensure they do not overflow the viewport.
 *
 * @param renderer The SDL_Renderer context.
 * @param parentViewOffset The screen-space offset inherited from parent containers.
 */
void ScrollView::draw(SDL_Renderer* renderer, const SDL_FPoint& parentViewOffset) {
    if (!renderer) return;

    // Calculate final screen-space coordinates for the outer bounds and the inner view rectangle
    SDL_FRect finalBounds = { m_bounds.x + parentViewOffset.x, m_bounds.y + parentViewOffset.y, m_bounds.w, m_bounds.h };
    SDL_FRect finalViewRect = { m_viewRect.x + parentViewOffset.x, m_viewRect.y + parentViewOffset.y, m_viewRect.w, m_viewRect.h };

    // Draw background if style dictates it
    if (m_style.drawBackground) {
        SDL_SetRenderDrawColor(renderer, m_style.bgColor.r, m_style.bgColor.g, m_style.bgColor.b, m_style.bgColor.a);
        SDL_RenderFillRect(renderer, &finalBounds);
    }

    // --- Clipping Setup ---
    SDL_Rect oldClipRect;
    bool hadClip = SDL_RenderClipEnabled(renderer);
    // If clipping was already enabled, save the existing clip rectangle
    if (hadClip) {
        SDL_GetRenderClipRect(renderer, &oldClipRect);
    }

    // Set the new clip rectangle to the inner view area (excluding the scrollbar track)
    SDL_Rect clipRect = { static_cast<int>(finalViewRect.x), static_cast<int>(finalViewRect.y), static_cast<int>(finalViewRect.w), static_cast<int>(finalViewRect.h) };
    SDL_SetRenderClipRect(renderer, &clipRect);

    // Calculate the screen-space offset for children:
    // This is the viewRect top-left (screen) minus the current scroll position (m_scrollY)
    SDL_FPoint childOffset = { finalViewRect.x, finalViewRect.y - m_scrollY };
    // Draw all child controls with the calculated offset
    for (auto& control : m_controls) {
        control->draw(renderer, childOffset);
    }

    // --- Clipping Restoration ---
    // Restore the previous clip rectangle or disable clipping
    if (hadClip) {
        SDL_SetRenderClipRect(renderer, &oldClipRect);
    } else {
        SDL_SetRenderClipRect(renderer, nullptr);
    }

    // Draw border around the outer bounds if enabled (over the scrollbar track)
    if (m_style.drawBorder) {
        SDL_SetRenderDrawColor(renderer, m_style.borderColor.r, m_style.borderColor.g, m_style.borderColor.b, m_style.borderColor.a);
        SDL_RenderRect(renderer, &finalBounds);
    }

    // Render the scrollbar on the side
    drawScrollbar(renderer, parentViewOffset);
}

/**
 * @brief Calculates the relative position and size of the scrollbar thumb.
 *
 * The returned rectangle's x coordinate is relative to the viewRect's origin,
 * positioned at the right edge of the viewRect (m_viewRect.w).
 *
 * @param viewOffset Unused parameter for this retained mode implementation (kept for potential IControl consistency).
 * @return The SDL_FRect defining the thumb's size and position relative to the ScrollView's view rectangle.
 */
SDL_FRect ScrollView::getScrollbarThumbRect(const SDL_FPoint& /*viewOffset*/) const {
    // Check if scrolling is necessary
    if (m_contentHeight <= m_viewRect.h) {
        return {0.0f, 0.0f, 0.0f, 0.0f}; // No scrollbar needed
    }

    float trackHeight = m_viewRect.h;
    // Thumb height scales with the proportion of content visible, clamped to a minimum size
    float thumbHeight = std::max(20.0f, (trackHeight / m_contentHeight) * trackHeight);
    // Height of the track that the thumb can move within
    float scrollableTrackHeight = trackHeight - thumbHeight;
    // Total vertical content that is currently off-screen
    float scrollableContentHeight = m_contentHeight - m_viewRect.h;

    float thumbY = 0.0f;
    if (scrollableContentHeight > 0.0f) {
        // Calculate thumb Y position based on current scroll amount as a percentage of the total scrollable range
        thumbY = (m_scrollY / scrollableContentHeight) * scrollableTrackHeight;
    }

    // X position is at the right edge of the view area, covering the reserved scrollbar width
    return { m_viewRect.w, thumbY, static_cast<float>(m_style.scrollbarWidth), thumbHeight };
}

/**
 * @brief Renders the vertical scrollbar track and thumb.
 *
 * Only draws if the content height exceeds the view height.
 *
 * @param renderer The SDL_Renderer context.
 * @param parentViewOffset The screen-space offset of the ScrollView.
 */
void ScrollView::drawScrollbar(SDL_Renderer* renderer, const SDL_FPoint& parentViewOffset) {
    // Only draw if scrolling is required
    if (m_contentHeight <= m_viewRect.h) return;

    // Calculate the ScrollView's outer bounds in screen-space
    SDL_FRect finalBounds = { m_bounds.x + parentViewOffset.x, m_bounds.y + parentViewOffset.y, m_bounds.w, m_bounds.h };

    // Draw the scrollbar track (occupies the space reserved at the right edge)
    SDL_FRect scrollbarTrackRect = { finalBounds.x + finalBounds.w - static_cast<float>(m_style.scrollbarWidth), finalBounds.y, static_cast<float>(m_style.scrollbarWidth), finalBounds.h };
    SDL_SetRenderDrawColor(renderer, m_style.scrollbarBgColor.r, m_style.scrollbarBgColor.g, m_style.scrollbarBgColor.b, m_style.scrollbarBgColor.a);
    SDL_RenderFillRect(renderer, &scrollbarTrackRect);

    // Get the thumb's relative position and translate it to screen-space
    SDL_FRect thumbRelativeRect = getScrollbarThumbRect({0.0f, 0.0f});
    SDL_FRect scrollbarThumbRect = { finalBounds.x + finalBounds.w - static_cast<float>(m_style.scrollbarWidth), finalBounds.y + thumbRelativeRect.y, thumbRelativeRect.w, thumbRelativeRect.h };

    // Determine the thumb color based on interaction state
    SDL_Color thumbColor = m_style.scrollbarThumbColor;
    if (m_isScrollbarGrabbed) {
        thumbColor = m_style.scrollbarThumbGrabbedColor;
    } else if (m_isScrollbarHovered) {
        thumbColor = m_style.scrollbarThumbHoverColor;
    }

    // Draw the thumb
    SDL_SetRenderDrawColor(renderer, thumbColor.r, thumbColor.g, thumbColor.b, thumbColor.a);
    SDL_RenderFillRect(renderer, &scrollbarThumbRect);
}


// --- Retained Mode ScrollView Implementation Ends Here ---

// -----------------------------------------------------------------------------
// --- Immediate Mode ScrollView Implementation Starts Here ---
// -----------------------------------------------------------------------------

#include <unordered_map>
#include <vector>

/**
 * @brief Structure to hold and restore the renderer's clip state.
 */
struct ClipState {
    bool hadClip;       ///< True if clipping was enabled before this ScrollView.
    SDL_Rect oldClip;   ///< The previous clip rectangle.
    std::string id;     ///< The ID of the ScrollView that pushed this clip state.
};
static std::vector<ClipState> g_clipStack; ///< Global stack for managing nested clip rectangles.

/**
 * @brief Structure to hold the runtime state for an immediate-mode ScrollView.
 */
struct ScrollState {
    SDL_FPoint pos{0.0f, 0.0f};         ///< Current scroll position (x=0, y=scroll amount).
    SDL_FRect view;                     ///< The final screen-space rectangle of the viewport.
    SDL_FPoint content;                 ///< The total content dimensions.
    bool thumb_dragging = false;        ///< True if the scrollbar thumb is actively being dragged.
    bool content_dragging = false;      ///< True if the content area is actively being dragged (e.g., via touch).
    float grabOffsetY = 0.0f;           ///< Offset from the thumb's top edge to the grab point (for thumb dragging).
    float last_y = 0.0f;                ///< Last y-position of the pointer for calculating drag delta.
    ScrollViewStyle style_;             ///< Copy of the style used to draw the scrollbar.
    uint64_t activeFingerId = 0;        ///< The ID of the finger currently dragging (0 for mouse or inactive).
};
static std::unordered_map<std::string, ScrollState> g_states; ///< Global map storing state for all active immediate ScrollViews by ID.

/**
 * @brief Helper function to resolve PositionParams into a final screen-space view rectangle.
 * @param p The positional parameters.
 * @param w The explicit width of the viewport.
 * @param h The explicit height of the viewport.
 * @param parentW The width of the parent container (uses window size if <= 0).
 * @param parentH The height of the parent container (uses window size if <= 0).
 * @return The calculated SDL_FRect in screen coordinates.
 */
static SDL_FRect ResolveViewRectFromParams(const XenUI::PositionParams& p, int w, int h, int parentW = -1, int parentH = -1) {
    // Fallback to window size if parent dimensions are not explicitly provided
    if (parentW <= 0 || parentH <= 0) {
        SDL_Point win = XenUI::GetWindowSize();
        parentW = win.x;
        parentH = win.y;
    }
    // Calculate final position based on anchoring/offset rules
    SDL_Point pos = XenUI::CalculateFinalPosition(p, w, h, parentW, parentH);
    return { (float)pos.x, (float)pos.y, (float)w, (float)h };
}

namespace XenUI {

// --- Immediate Mode Overloads ---

/**
 * @brief Immediate Mode: Starts a scrollable region by resolving PositionParams.
 *
 * This overload simplifies usage by deriving the viewRect from position parameters.
 *
 * @param id Unique identifier for the ScrollView instance (used for state lookup).
 * @param posParams Positional constraints for the viewport.
 * @param viewWidth The explicit width of the viewport area.
 * @param viewHeight The explicit height of the viewport area.
 * @param contentSize The total dimensions of the content inside (used for scroll limits).
 * @param renderer The SDL_Renderer context.
 * @param event The current SDL_Event for input processing.
 * @param style The visual style for the scrollbar and background.
 * @return The content-space offset (screen_x, screen_y - scroll_y) to apply to child elements.
 */
SDL_FPoint BeginScrollView(
    const std::string&            id,
    const XenUI::PositionParams&  posParams,
    int                           viewWidth,
    int                           viewHeight,
    const SDL_FPoint&             contentSize,
    SDL_Renderer* renderer,
    const SDL_Event&              event,
    const ScrollViewStyle&        style)
{
    SDL_FRect viewRect = ResolveViewRectFromParams(posParams, viewWidth, viewHeight);
    return BeginScrollView(id, viewRect, contentSize, renderer, event, style);
}

/**
 * @brief Immediate Mode: Starts a scrollable region, allowing explicit parent dimensions.
 *
 * @param id Unique identifier for the ScrollView instance.
 * @param posParams Positional constraints for the viewport.
 * @param viewWidth The explicit width of the viewport area.
 * @param viewHeight The explicit height of the viewport area.
 * @param parentWidth The width of the parent area for layout calculation.
 * @param parentHeight The height of the parent area for layout calculation.
 * @param contentSize The total dimensions of the content inside.
 * @param renderer The SDL_Renderer context.
 * @param event The current SDL_Event for input processing.
 * @param style The visual style for the scrollbar and background.
 * @return The content-space offset to apply to child elements.
 */
SDL_FPoint BeginScrollView(
    const std::string&            id,
    const XenUI::PositionParams&  posParams,
    int                           viewWidth,
    int                           viewHeight,
    int                           parentWidth,
    int                           parentHeight,
    const SDL_FPoint&             contentSize,
    SDL_Renderer* renderer,
    const SDL_Event&              event,
    const ScrollViewStyle&        style)
{

    SDL_FRect viewRect = ResolveViewRectFromParams(posParams, viewWidth, viewHeight, parentWidth, parentHeight);
    return BeginScrollView(id, viewRect, contentSize, renderer, event, style);
}

/**
 * @brief Checks if the immediate-mode ScrollView is currently being dragged (thumb or content).
 * @param id The ID of the ScrollView to check.
 * @return true if the ScrollView is in a dragging state, false otherwise.
 */
bool IsScrollViewDragging(const std::string& id) {
    auto it = g_states.find(id);
    return (it != g_states.end()) ? (it->second.thumb_dragging || it->second.content_dragging) : false;
}

// --- Primary Immediate Mode Function ---

/**
 * @brief Immediate Mode: Starts a scrollable region, setting up state and clipping.
 *
 * This function processes input (scrolling, dragging) and returns the offset needed
 * to draw child elements inside the scrolling region.
 *
 * @param id Unique identifier for the ScrollView instance.
 * @param viewRect The final screen-space rectangle of the viewport.
 * @param contentSize The total dimensions of the content inside.
 * @param renderer The SDL_Renderer context.
 * @param event The current SDL_Event for input processing.
 * @param style The visual style for the scrollbar and background.
 * @return The content-space offset (screen_x, screen_y - scroll_y) to apply to child elements.
 */
SDL_FPoint BeginScrollView(
    const std::string& id,
    const SDL_FRect& viewRect,
    const SDL_FPoint& contentSize,
    SDL_Renderer* renderer,
    const SDL_Event& event,
    const ScrollViewStyle& style) {

    // Draw background
    if (style.drawBackground) {
        SDL_SetRenderDrawColor(renderer, style.bgColor.r, style.bgColor.g, style.bgColor.b, style.bgColor.a);
        SDL_RenderFillRect(renderer, &viewRect);
    }

    // --- Clipping Setup ---
    // Save old clip state and push to the global stack
    bool had = SDL_RenderClipEnabled(renderer);
    SDL_Rect old;
    if (had) SDL_GetRenderClipRect(renderer, &old);
    g_clipStack.push_back({had, old, id});

    // Determine clip width (subtract scrollbar width if needed)
    bool needsScrollbar = (contentSize.y > viewRect.h);
    float scrollbarW = static_cast<float>(style.scrollbarWidth);
    int clipW = needsScrollbar ? static_cast<int>(viewRect.w - scrollbarW) : static_cast<int>(viewRect.w);

    // Set the new clip rectangle
    SDL_Rect clip = { static_cast<int>(viewRect.x), static_cast<int>(viewRect.y), clipW, static_cast<int>(viewRect.h) };
    SDL_SetRenderClipRect(renderer, &clip);

    // --- State Management ---
    // Lookup or create the state for this ID
    auto& st = g_states[id];
    st.view = viewRect;
    st.content = contentSize;
    st.style_ = style;

    // --- Pointer Position Resolution (Screen-Space) ---
    SDL_FPoint mp{0.0f, 0.0f};
    bool haveMP = false;

    // TOUCH HANDLING: Convert normalized touch coords to logical window coords
    if (event.type == SDL_EVENT_FINGER_DOWN ||
        event.type == SDL_EVENT_FINGER_MOTION ||
        event.type == SDL_EVENT_FINGER_UP) {

        // Debug log for touch events (optional, useful for device testing)
        SDL_Log("BeginScrollView: finger event type=%d fingerId=%llu x=%f y=%f window=%u",
                event.type,
                (unsigned long long)event.tfinger.touchID,
                (double)event.tfinger.x,
                (double)event.tfinger.y,
                (unsigned)event.tfinger.windowID);

        // Attempt to find the correct window for conversion
        SDL_Window* useWindow = nullptr;
        if (event.tfinger.windowID != 0) {
            useWindow = SDL_GetWindowFromID(event.tfinger.windowID);
        }
        if (!useWindow) {
            useWindow = SDL_GetKeyboardFocus();
            if (!useWindow) useWindow = SDL_GetMouseFocus();
        }

        int winW = 0, winH = 0;
        if (useWindow) {
            SDL_GetWindowSize(useWindow, &winW, &winH);
        }

        // Apply scale: normalized * window_size = logical coordinate
        mp.x = (winW > 0) ? (event.tfinger.x * (float)winW) : event.tfinger.x;
        mp.y = (winH > 0) ? (event.tfinger.y * (float)winH) : event.tfinger.y;
        haveMP = true;

        // --- Touch State Logic ---
        uint64_t fid = event.tfinger.touchID;

        if (event.type == SDL_EVENT_FINGER_DOWN) {
            // Record active finger ID for tracking subsequent motion/up events
            st.activeFingerId = fid;
        }
        else if (event.type == SDL_EVENT_FINGER_UP) {
            // Only stop dragging if this is the active finger
            if (st.activeFingerId == fid) {
                st.thumb_dragging = false;
                st.content_dragging = false;
                st.activeFingerId = 0;
            }
        }
    }
    // MOUSE FALLBACK (desktop): Get current mouse state
    else {
        SDL_GetMouseState(&mp.x, &mp.y);
        haveMP = true;
    }

    // --- Input Processing: Scroll Wheel ---
    if (event.type == SDL_EVENT_MOUSE_WHEEL) {
        // Only scroll if the mouse pointer is within the viewport bounds
        if (haveMP && SDL_PointInRectFloat(&mp, &viewRect)) {
            st.pos.y -= event.wheel.y * 25.0f;
        }
    }

    // --- Input Processing: Dragging (Thumb and Content) ---
    // Recalculate thumb parameters for hit-testing
    float trackH = viewRect.h;
    float thumbH = std::max(20.0f, (trackH / contentSize.y) * trackH);
    float scrollable = std::max(0.0f, contentSize.y - trackH);
    float rangeH = trackH - thumbH; // Range the thumb can move
    float thumbY = viewRect.y + (scrollable > 0.0f ? (st.pos.y / scrollable) * rangeH : 0.0f);

    SDL_FRect thumbRect = { viewRect.x + viewRect.w - scrollbarW, thumbY, scrollbarW, thumbH };

    // Check if pointer is over the scrollbar thumb
    bool overThumb = haveMP && SDL_PointInRectFloat(&mp, &thumbRect);

    // Pointer Down (Mouse Button Down or Finger Down)
    if ((event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button == SDL_BUTTON_LEFT) ||
        (event.type == SDL_EVENT_FINGER_DOWN)) {

        if (overThumb) {
            // Start thumb drag
            st.thumb_dragging = true;
            st.content_dragging = false; // Ensure content drag is off
            st.grabOffsetY = mp.y - thumbY;
            if (event.type == SDL_EVENT_FINGER_DOWN) st.activeFingerId = event.tfinger.touchID;
        } else if (haveMP && SDL_PointInRectFloat(&mp, &viewRect)) {
            // Start content drag (swiping/touch) if click/touch is within the view area but not on the thumb
            st.content_dragging = true;
            st.thumb_dragging = false; // Ensure thumb drag is off
            st.last_y = mp.y;
            if (event.type == SDL_EVENT_FINGER_DOWN) st.activeFingerId = event.tfinger.touchID;
        }
    }
    // Pointer Up (Mouse Button Up or Finger Up)
    else if ((event.type == SDL_EVENT_MOUSE_BUTTON_UP && event.button.button == SDL_BUTTON_LEFT) ||
             (event.type == SDL_EVENT_FINGER_UP)) {
        // Stop dragging only if the releasing finger/mouse button matches the active state
        if (event.type == SDL_EVENT_FINGER_UP) {
            if (st.activeFingerId == event.tfinger.touchID) {
                st.thumb_dragging = false;
                st.content_dragging = false;
                st.activeFingerId = 0;
            }
        } else {
            // Mouse up always stops both drag types
            st.thumb_dragging = false;
            st.content_dragging = false;
        }
    }
    // Pointer Motion (Mouse Motion or Finger Motion)
    else if ((event.type == SDL_EVENT_MOUSE_MOTION) ||
             (event.type == SDL_EVENT_FINGER_MOTION)) {

        // Handle thumb drag
        if (st.thumb_dragging) {
            bool process = true;
            // For touch, ensure motion comes from the active finger
            if (event.type == SDL_EVENT_FINGER_MOTION && st.activeFingerId != event.tfinger.touchID) {
                process = false;
            }
            if (process) {
                // Convert thumb position on track to scroll position in content
                float rel = mp.y - st.grabOffsetY - viewRect.y;
                float pct = (rangeH > 0.0f) ? std::clamp(rel / rangeH, 0.0f, 1.0f) : 0.0f;
                st.pos.y = pct * scrollable;
            }
        }

        // Handle content drag (scroll by swiping content directly)
        if (st.content_dragging) {
            bool process = true;
            // For touch, ensure motion comes from the active finger
            if (event.type == SDL_EVENT_FINGER_MOTION && st.activeFingerId != event.tfinger.touchID) {
                process = false;
            }
            if (process) {
                // Calculate vertical delta and apply to scroll position
                float delta = st.last_y - mp.y;
                st.pos.y += delta;
                st.last_y = mp.y; // Update last position for next delta calculation
            }
        }
    }

    // Clamp the final scroll position to the valid range
    st.pos.y = std::clamp(st.pos.y, 0.0f, scrollable);

    // Return the content offset: the viewRect top-left, minus the current scroll amount
    return { viewRect.x, viewRect.y - st.pos.y };
}

/**
 * @brief Immediate Mode: Ends a scrollable region, restoring clip and drawing scrollbar/border.
 * @param renderer The SDL_Renderer context.
 */
void EndScrollView(SDL_Renderer* renderer) {
    if (g_clipStack.empty()) return;

    // Pop the clip state saved by BeginScrollView
    ClipState cs = g_clipStack.back();
    g_clipStack.pop_back();

    // Restore the previous clip rectangle or disable clipping
    if (cs.hadClip) {
        SDL_SetRenderClipRect(renderer, &cs.oldClip);
    } else {
        SDL_SetRenderClipRect(renderer, nullptr);
    }

    // Get the state associated with this ScrollView ID
    auto& st = g_states[cs.id];
    auto& v = st.view; // Viewport rectangle
    auto& S = st.style_; // Style
    auto& c = st.content; // Content size

    // Draw border around the viewport
    if (S.drawBorder) {
        SDL_SetRenderDrawColor(renderer, S.borderColor.r, S.borderColor.g, S.borderColor.b, S.borderColor.a);
        SDL_RenderRect(renderer, &v);
    }

    // Draw scrollbar if content exceeds viewport height
    if (c.y > v.h) {
        // Draw track
        float scrollbarW = static_cast<float>(S.scrollbarWidth);
        SDL_FRect track = { v.x + v.w - scrollbarW, v.y, scrollbarW, v.h };
        SDL_SetRenderDrawColor(renderer, S.scrollbarBgColor.r, S.scrollbarBgColor.g, S.scrollbarBgColor.b, S.scrollbarBgColor.a);
        SDL_RenderFillRect(renderer, &track);

        // Recompute thumb parameters (must be done again for drawing)
        float trackH = v.h;
        float thumbH = std::max(20.0f, (trackH / c.y) * trackH);
        float scrollable = std::max(0.0f, c.y - trackH);
        float rangeH = trackH - thumbH;
        float thumbY = v.y + (scrollable > 0.0f ? (st.pos.y / scrollable) * rangeH : 0.0f);

        SDL_FRect thumb = { v.x + v.w - scrollbarW, thumbY, scrollbarW, thumbH };

        // Choose color based on interaction state
        SDL_FPoint mp;
        SDL_GetMouseState(&mp.x, &mp.y);
        SDL_Color col = S.scrollbarThumbColor;
        if (st.thumb_dragging) {
            col = S.scrollbarThumbGrabbedColor;
        } else if (SDL_PointInRectFloat(&mp, &thumb)) {
            col = S.scrollbarThumbHoverColor;
        }

        // Draw the thumb
        SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, col.a);
        SDL_RenderFillRect(renderer, &thumb);
    }
}
} // namespace XenUI

// -----------------------------------------------------------------------------
// --- Immediate Mode ScrollView Implementation Ends Here ---
// -----------------------------------------------------------------------------