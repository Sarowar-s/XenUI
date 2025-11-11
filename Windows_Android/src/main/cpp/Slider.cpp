// SPDX-License-Identifier: Apache-2.0
/*
 *  Copyright (C) 2025 MD S M Sarowar Hossain
 *
 * 
 */
// Implements both the retained-mode (class Slider) and immediate-mode (function Slider)
// versions of a user interface slider control.
//

#include "Slider.h"
#include "WindowUtil.h" // Provides XenUI::GetWindowSize, XenUI::CalculateFinalPosition
#include "TextRenderer.h" // For rendering value text and initialization check

using namespace XenUI;

// ---------------- Retained Mode Slider Implementation Starts Here ----------------

/**
 * @brief Constructs a retained-mode Slider control.
 *
 * @param id Unique identifier for the slider.
 * @param orientation The orientation of the slider (Horizontal or Vertical).
 * @param posParams The positional constraints for the slider's bounding box.
 * @param length The total length of the slider track (width for horizontal, height for vertical).
 * @param initialValue The starting value, clamped between minValue and maxValue.
 * @param minValue The minimum possible value.
 * @param maxValue The maximum possible value.
 * @param style The visual style parameters.
 * @param onValueChanged Optional callback function invoked when the value changes.
 */
Slider::Slider(const std::string& id,
               XenUI::Orientation orientation,
               const XenUI::PositionParams& posParams,
               float length,
               float initialValue,
               float minValue,
               float maxValue,
               SliderStyle style,
               std::function<void(float)> onValueChanged)
    : m_id(id),
      m_orientation(orientation),
      m_posParams(posParams),
      m_minValue(minValue),
      m_maxValue(maxValue),
      m_currentValue(initialValue),
      m_style(style),
      m_onValueChanged(std::move(onValueChanged)),
      m_isDragging(false), m_isHovered(false), m_dragOffset(0.0f),
      m_posX(0), m_posY(0), m_width(0), m_height(0)
{
    // Clamp the initial value to the defined range
    m_currentValue = std::clamp(m_currentValue, m_minValue, m_maxValue);

    // Calculate the total bounding box size based on orientation and requested length
    if (m_orientation == XenUI::Orientation::HORIZONTAL) {
        m_width = static_cast<int>(length);
        // Height is determined by thumb size plus padding above and below
        m_height = m_style.thumbSize + 2 * m_style.padding;
    } else {
        // Width is determined by thumb size plus padding left and right
        m_width = m_style.thumbSize + 2 * m_style.padding;
        m_height = static_cast<int>(length);
    }

    // Perform initial layout calculation (uses window size as default parent)
    recalculateLayout();
}

/**
 * @brief Calculates the content-space bounding box of the slider thumb.
 *
 * @return The SDL_FRect defining the thumb's size and content-space position.
 */
SDL_FRect Slider::getThumbRectContent() const {
    float thumbW = static_cast<float>(m_style.thumbSize);
    float thumbH = static_cast<float>(m_style.thumbSize);
    // Normalize current value to a 0.0 to 1.0 range
    float normalized = 0.0f;
    if (m_maxValue != m_minValue)
        normalized = (m_currentValue - m_minValue) / (m_maxValue - m_minValue);
    normalized = std::clamp(normalized, 0.0f, 1.0f);

    if (m_orientation == XenUI::Orientation::HORIZONTAL) {
        float trackLength = static_cast<float>(m_width - 2 * m_style.padding);
        // Calculate X position: start of track + normalized displacement - half thumb width (to center)
        float tx = static_cast<float>(m_posX) + m_style.padding + (trackLength * normalized) - (thumbW / 2.0f);
        // Calculate Y position: centered vertically within the slider height
        float ty = static_cast<float>(m_posY) + (m_height - thumbH) / 2.0f;
        return SDL_FRect{tx, ty, thumbW, thumbH};
    } else {
        float trackLength = static_cast<float>(m_height - 2 * m_style.padding);
        // Vertical track is inverted: 1.0 - normalized is used so value increases moving up/left (SDL origin is top-left)
        // Calculate Y position: start of track + inverted normalized displacement - half thumb height
        float ty = static_cast<float>(m_posY) + m_style.padding + (trackLength * (1.0f - normalized)) - (thumbH / 2.0f);
        // Calculate X position: centered horizontally
        float tx = static_cast<float>(m_posX) + (m_width - thumbW) / 2.0f;
        return SDL_FRect{tx, ty, thumbW, thumbH};
    }
}

/**
 * @brief Checks if a content-space point falls within the slider thumb.
 *
 * @param x Content-space X coordinate.
 * @param y Content-space Y coordinate.
 * @return true if the point is within the thumb, false otherwise.
 */
bool Slider::isPointInThumbContent(float x, float y) const {
    SDL_FRect r = getThumbRectContent();
    return (x >= r.x && x <= (r.x + r.w) && y >= r.y && y <= (r.y + r.h));
}

/**
 * @brief Updates the slider's value based on a content-space mouse position during drag.
 *
 * @param mouseX Content-space X coordinate of the pointer.
 * @param mouseY Content-space Y coordinate of the pointer.
 */
void Slider::updateValueFromMouseContent(float mouseX, float mouseY) {
    float newValue = m_currentValue;
    if (m_orientation == XenUI::Orientation::HORIZONTAL) {
        // Track calculation: start, end, and total length
        float trackStart = static_cast<float>(m_posX + m_style.padding);
        float trackEnd = static_cast<float>(m_posX + m_width - m_style.padding);
        float trackLen = trackEnd - trackStart;
        // Position on the track adjusted for the drag offset (maintains grab point)
        float posInTrack = mouseX - trackStart - m_dragOffset;
        // Convert position in track to value using linear interpolation
        newValue = m_minValue + (m_maxValue - m_minValue) * (posInTrack / trackLen);
    } else {
        float trackStart = static_cast<float>(m_posY + m_style.padding);
        float trackEnd = static_cast<float>(m_posY + m_height - m_style.padding);
        float trackLen = trackEnd - trackStart;
        float posInTrack = mouseY - trackStart - m_dragOffset;
        // For vertical, we use (1.0f - percentage) as the SDL Y-axis is inverted relative to typical UI vertical values
        newValue = m_minValue + (m_maxValue - m_minValue) * (1.0f - (posInTrack / trackLen));
    }
    // Final clamping and event firing if value has actually changed
    newValue = std::clamp(newValue, m_minValue, m_maxValue);
    if (newValue != m_currentValue) {
        m_currentValue = newValue;
        if (m_onValueChanged) m_onValueChanged(m_currentValue);
    }
}

/**
 * @brief Renders the slider track, thumb, and optional value text.
 *
 * @param renderer The SDL_Renderer context.
 * @param viewOffset The screen-space offset inherited from parent containers.
 */
void Slider::draw(SDL_Renderer* renderer, const SDL_FPoint& viewOffset) {
    // Check renderer and ensure text rendering system is ready if text is needed
    if (!renderer || !TextRenderer::getInstance().isInitialized()) return;

    // Calculate the track rectangle in screen-space
    SDL_FRect trackRect;
    if (m_orientation == XenUI::Orientation::HORIZONTAL) {
        trackRect = {m_posX + viewOffset.x + m_style.padding,
                     m_posY + viewOffset.y + (m_height - m_style.trackThickness) / 2.0f, // Center vertically
                     static_cast<float>(m_width - 2 * m_style.padding),
                     static_cast<float>(m_style.trackThickness)};
    } else {
        trackRect = {m_posX + viewOffset.x + (m_width - m_style.trackThickness) / 2.0f, // Center horizontally
                     m_posY + viewOffset.y + m_style.padding,
                     static_cast<float>(m_style.trackThickness),
                     static_cast<float>(m_height - 2 * m_style.padding)};
    }

    // Draw the track
    SDL_SetRenderDrawColor(renderer, m_style.trackColor.r, m_style.trackColor.g, m_style.trackColor.b, m_style.trackColor.a);
    SDL_RenderFillRect(renderer, &trackRect);

    // Get the thumb rectangle in content-space and convert to screen-space for drawing
    SDL_FRect thumb = getThumbRectContent();
    SDL_FRect thumbScreen = { thumb.x + viewOffset.x, thumb.y + viewOffset.y, thumb.w, thumb.h };

    // Determine thumb color based on hover state
    SDL_Color curThumbColor = m_isHovered ? m_style.thumbHoverColor : m_style.thumbColor;
    SDL_SetRenderDrawColor(renderer, curThumbColor.r, curThumbColor.g, curThumbColor.b, curThumbColor.a);
    SDL_RenderFillRect(renderer, &thumbScreen);

    // Draw value text if enabled
    if (m_style.drawValueText) {
        std::string valueText = std::to_string(static_cast<int>(m_currentValue)); // Display as integer
        int tw = 0, th = 0;
        TextRenderer::getInstance().measureText(valueText, m_style.valueTextFontSize, tw, th);
        int tx, ty;

        // Position text relative to the thumb/slider depending on orientation
        if (m_orientation == XenUI::Orientation::HORIZONTAL) {
            // Below the slider
            tx = static_cast<int>(thumbScreen.x + (thumbScreen.w - tw) / 2.0f); // Center on thumb
            ty = static_cast<int>(m_posY + m_height - th - 2 + viewOffset.y);
        } else {
            // To the right of the slider
            tx = static_cast<int>(m_posX + m_width + 2 + viewOffset.x);
            ty = static_cast<int>(thumbScreen.y + (thumbScreen.h - th) / 2.0f); // Center on thumb
        }
        TextRenderer::getInstance().getInstance().renderText(valueText, tx, ty, m_style.valueTextColor, m_style.valueTextFontSize);
    }
}

/**
 * @brief Handles input events (mouse movement, clicks) for the slider.
 *
 * Expects event coordinates to be in content-space if the slider is nested.
 *
 * @param event The SDL_Event to process.
 * @return true if the slider's state or value changed as a result of the event, false otherwise.
 */
bool Slider::handleEvent(const SDL_Event& event) {
    bool changed = false;

    // Read event coordinates, converting to float for calculation
    float mx=0.0f, my=0.0f;
    if (event.type == SDL_EVENT_MOUSE_MOTION) {
        mx = (float)event.motion.x; my = (float)event.motion.y;
    } else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN || event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
        mx = (float)event.button.x; my = (float)event.button.y;
    } else {
        // Only interested in mouse events for this control
        return false;
    }

    // Hover state update
    bool wasHovered = m_isHovered;
    m_isHovered = isPointInThumbContent(mx, my);
    if (wasHovered != m_isHovered) changed = true;

    // Mouse Button Down (LMB)
    if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button == SDL_BUTTON_LEFT) {
        if (m_isHovered) {
            // Start drag on the thumb
            m_isDragging = true;
            SDL_FRect thumb = getThumbRectContent();
            // Calculate drag offset to maintain grab point relative to the thumb center
            if (m_orientation == XenUI::Orientation::HORIZONTAL)
                m_dragOffset = mx - (thumb.x + thumb.w / 2.0f);
            else
                m_dragOffset = my - (thumb.y + thumb.h / 2.0f);
            changed = true;
        } else {
            // Check click on the slider's overall bounds (jump to click position)
            SDL_FRect bounds = { (float)m_posX, (float)m_posY, (float)m_width, (float)m_height };
            if (mx >= bounds.x && mx <= bounds.x + bounds.w && my >= bounds.y && my <= bounds.y + bounds.h) {
                // Start dragging and update value immediately to jump
                m_isDragging = true;
                m_dragOffset = 0.0f; // Reset offset for the initial jump click
                updateValueFromMouseContent(mx, my);
                changed = true;
            }
        }
    }
    // Mouse Button Up (LMB)
    else if (event.type == SDL_EVENT_MOUSE_BUTTON_UP && event.button.button == SDL_BUTTON_LEFT) {
        if (m_isDragging) {
            m_isDragging = false;
            changed = true;
        }
    }
    // Mouse Motion
    else if (event.type == SDL_EVENT_MOUSE_MOTION) {
        if (m_isDragging) {
            // Continue drag, updating value
            updateValueFromMouseContent(mx, my);
            changed = true;
        }
    }

    return changed;
}

/**
 * @brief Recalculates the slider's screen position based on parent layout.
 *
 * @param parentWidth The available width in the parent container.
 * @param parentHeight The available height in the parent container.
 */
void Slider::recalculateLayout(int parentWidth, int parentHeight) {
    // Resolve position using PositionParams
    SDL_Point p = XenUI::CalculateFinalPosition(m_posParams, m_width, m_height, parentWidth, parentHeight);
    m_posX = p.x; m_posY = p.y;
}

/**
 * @brief Returns the absolute content-space bounding box of the entire slider control.
 *
 * @return The SDL_FRect defining the bounds.
 */
SDL_FRect Slider::getBounds() const {
    return SDL_FRect{ (float)m_posX, (float)m_posY, (float)m_width, (float)m_height };
}

/**
 * @brief Programmatically sets the slider's current value.
 *
 * Clamps the new value and invokes the callback if the value changes.
 *
 * @param newValue The new value to set.
 */
void Slider::setValue(float newValue) {
    newValue = std::clamp(newValue, m_minValue, m_maxValue);
    if (newValue != m_currentValue) {
        m_currentValue = newValue;
        if (m_onValueChanged) m_onValueChanged(m_currentValue);
    }
}

// ---------------- Retained Mode Slider Implementation Ends Here ----------------


namespace XenUI {
// ---------------- Immediate Mode Slider Implementation Starts Here ----------------

/**
 * @brief Immediate Mode: Renders and manages a slider control using window size as parent fallback.
 *
 * This function is a convenience wrapper that resolves layout relative to the window size.
 *
 * @param id Unique identifier (for state persistence).
 * @param orientation Slider orientation.
 * @param posParams Positional constraints.
 * @param length Track length.
 * @param value Pointer to the value variable (read/write).
 * @param minValue Minimum value.
 * @param maxValue Maximum value.
 * @param style Visual style.
 * @param viewOffset Screen-space offset from parent (e.g., ScrollView offset).
 * @param event The current SDL_Event pointer (optional, but needed for input).
 * @return true if the slider's value was changed this frame, false otherwise.
 */
bool Slider(const std::string& id,
            XenUI::Orientation orientation,
            const PositionParams& posParams,
            float length,
            float* value,
            float minValue,
            float maxValue,
            SliderStyle style,
            const SDL_FPoint& viewOffset,
            const SDL_Event* event)
{
    // Get window size and forward to the parent-aware implementation
    SDL_Point ws = XenUI::GetWindowSize();
    return Slider(id, orientation, posParams, length, value, minValue, maxValue,
                  ws.x, ws.y, style, viewOffset, event);
}

/**
 * @brief Immediate Mode: Parent-aware implementation of the slider control.
 *
 * Renders the slider, handles input using the shared state map, and updates the external
 * value pointer.
 *
 * @param id Unique identifier (for state persistence).
 * @param orientation Slider orientation.
 * @param posParams Positional constraints.
 * @param length Track length.
 * @param value Pointer to the value variable (read/write).
 * @param minValue Minimum value.
 * @param maxValue Maximum value.
 * @param parentWidth The width of the parent container for layout resolution.
 * @param parentHeight The height of the parent container for layout resolution.
 * @param style Visual style.
 * @param viewOffset Screen-space offset from parent (e.g., ScrollView offset).
 * @param event The current SDL_Event pointer (optional, but input logic uses direct mouse state).
 * @return true if the slider's value was changed this frame, false otherwise.
 */
bool Slider(const std::string& id,
            XenUI::Orientation orientation,
            const PositionParams& posParams,
            float length,
            float* value,
            float minValue,
            float maxValue,
            int parentWidth,
            int parentHeight,
            SliderStyle style,
            const SDL_FPoint& viewOffset,
            const SDL_Event* /*event*/)
{
    // Essential checks
    if (!value || !TextRenderer::getInstance().getInstance().isInitialized()) return false;

    // Retrieve or create persistent state for this ID
    Detail::SliderState& st = Detail::sliderStates[id];
    bool changed = false;

    // Sync external value to internal state at the start of the frame
    if (st.currentValue != *value) st.currentValue = *value;

    // Compute slider total dimensions (content-space)
    int sliderW, sliderH;
    if (orientation == XenUI::Orientation::HORIZONTAL) {
        sliderW = static_cast<int>(length);
        sliderH = style.thumbSize + 2 * style.padding;
    } else {
        sliderW = style.thumbSize + 2 * style.padding;
        sliderH = static_cast<int>(length);
    }

    // Resolve final position (content-space) using parent dimensions
    SDL_Point finalPos = CalculateFinalPosition(posParams, sliderW, sliderH, parentWidth, parentHeight);
    float finalX = static_cast<float>(finalPos.x); // content-space X
    float finalY = static_cast<float>(finalPos.y); // content-space Y

    // Get mouse state (window coords) and convert to content-space
    float mx_win = 0, my_win = 0;
    Uint32 mstate = SDL_GetMouseState(&mx_win, &my_win);
    float mouseX = mx_win - viewOffset.x; // CONTENT-space X
    float mouseY = my_win - viewOffset.y; // CONTENT-space Y
    bool mouseLeftDown = (mstate & SDL_BUTTON_MASK(SDL_BUTTON_LEFT)) != 0;

    // --- Thumb Geometry Calculation (Content-Space) ---
    float thumbW = static_cast<float>(style.thumbSize);
    float thumbH = static_cast<float>(style.thumbSize);
    float normalized = 0.0f;
    if (maxValue != minValue) normalized = (st.currentValue - minValue) / (maxValue - minValue);
    normalized = std::clamp(normalized, 0.0f, 1.0f);

    float trackLen;
    SDL_FRect thumbRect; // content-space thumb bounding box
    thumbRect.w = thumbW; thumbRect.h = thumbH;

    if (orientation == XenUI::Orientation::HORIZONTAL) {
        trackLen = static_cast<float>(sliderW - 2 * style.padding);
        thumbRect.x = finalX + style.padding + (trackLen * normalized) - (thumbW / 2.0f);
        thumbRect.y = finalY + (sliderH - thumbH) / 2.0f;
    } else {
        trackLen = static_cast<float>(sliderH - 2 * style.padding);
        // Vertical is inverted: use 1.0 - normalized
        thumbRect.y = finalY + style.padding + (trackLen * (1.0f - normalized)) - (thumbH / 2.0f);
        thumbRect.x = finalX + (sliderW - thumbW) / 2.0f;
    }

    // --- Input Processing (Always uses content-space coordinates) ---

    // Hover check
    st.isHovered = (mouseX >= thumbRect.x && mouseX <= (thumbRect.x + thumbRect.w) &&
                    mouseY >= thumbRect.y && mouseY <= (thumbRect.y + thumbRect.h));

    // Mouse Down (Drag Start or Jump)
    if (mouseLeftDown) {
        if (!st.isDragging && st.isHovered) {
            // Start drag on the thumb
            st.isDragging = true;
            // Calculate drag offset relative to the thumb center
            if (orientation == XenUI::Orientation::HORIZONTAL)
                st.dragOffset = mouseX - (thumbRect.x + thumbRect.w / 2.0f);
            else
                st.dragOffset = mouseY - (thumbRect.y + thumbRect.h / 2.0f);
        } else if (!st.isDragging) {
            // Check click on the slider's overall bounds (jump to click position)
            SDL_FRect sliderBounds = { finalX, finalY, static_cast<float>(sliderW), static_cast<float>(sliderH) };
            if (mouseX >= sliderBounds.x && mouseX <= (sliderBounds.x + sliderBounds.w) &&
                mouseY >= sliderBounds.y && mouseY <= (sliderBounds.y + sliderBounds.h)) {
                // Perform jump, then enter drag state
                st.isDragging = true;
                st.dragOffset = 0.0f; // Reset offset for the initial jump click
                float mousePosInTrack;
                float newVal;

                if (orientation == XenUI::Orientation::HORIZONTAL) {
                    mousePosInTrack = mouseX - (finalX + style.padding);
                    newVal = minValue + (maxValue - minValue) * (mousePosInTrack / trackLen);
                } else {
                    mousePosInTrack = mouseY - (finalY + style.padding);
                    newVal = minValue + (maxValue - minValue) * (1.0f - (mousePosInTrack / trackLen));
                }
                newVal = std::clamp(newVal, minValue, maxValue);
                if (newVal != st.currentValue) { st.currentValue = newVal; *value = st.currentValue; changed = true; }
            }
        }
    }
    // Mouse Up (Drag End)
    else {
        if (st.isDragging) {
            st.isDragging = false;
            st.dragOffset = 0.0f;
        }
    }

    // Drag Motion (Update Value)
    if (st.isDragging) {
        float mousePosInTrack;
        float newVal = st.currentValue;
        if (orientation == XenUI::Orientation::HORIZONTAL) {
            float trackStart = finalX + style.padding;
            mousePosInTrack = mouseX - trackStart - st.dragOffset;
            newVal = minValue + (maxValue - minValue) * (mousePosInTrack / trackLen);
        } else {
            float trackStart = finalY + style.padding;
            mousePosInTrack = mouseY - trackStart - st.dragOffset;
            newVal = minValue + (maxValue - minValue) * (1.0f - (mousePosInTrack / trackLen));
        }
        newVal = std::clamp(newVal, minValue, maxValue);
        if (newVal != st.currentValue) { st.currentValue = newVal; *value = st.currentValue; changed = true; }
    }

    // ---------- Drawing (Uses screen-space coordinates) ----------
    SDL_Renderer* renderer = TextRenderer::getInstance().getRenderer();
    if (!renderer) return changed; // Safety check for renderer access

    // Track drawing (screen-space)
    SDL_FRect trackRect;
    if (orientation == XenUI::Orientation::HORIZONTAL) {
        trackRect = { finalX + style.padding + viewOffset.x,
                      finalY + (sliderH - style.trackThickness) / 2.0f + viewOffset.y,
                      static_cast<float>(sliderW - 2 * style.padding),
                      static_cast<float>(style.trackThickness) };
    } else {
        trackRect = { finalX + (sliderW - style.trackThickness) / 2.0f + viewOffset.x,
                      finalY + style.padding + viewOffset.y,
                      static_cast<float>(style.trackThickness),
                      static_cast<float>(sliderH - 2 * style.padding) };
    }
    SDL_SetRenderDrawColor(renderer, style.trackColor.r, style.trackColor.g, style.trackColor.b, style.trackColor.a);
    SDL_RenderFillRect(renderer, &trackRect);

    // Thumb drawing: Recalculate thumb position based on the (potentially newly updated) st.currentValue
    normalized = std::clamp((st.currentValue - minValue) / (maxValue - minValue), 0.0f, 1.0f);
    if (orientation == XenUI::Orientation::HORIZONTAL) {
        thumbRect.x = finalX + style.padding + (trackLen * normalized) - (thumbW / 2.0f);
        thumbRect.y = finalY + (sliderH - thumbH) / 2.0f;
    } else {
        thumbRect.y = finalY + style.padding + (trackLen * (1.0f - normalized)) - (thumbH / 2.0f);
        thumbRect.x = finalX + (sliderW - thumbW) / 2.0f;
    }

    // Convert thumb rect to screen-space
    SDL_FRect thumbScreen = { thumbRect.x + viewOffset.x, thumbRect.y + viewOffset.y, thumbRect.w, thumbRect.h };
    SDL_Color curThumbColor = st.isHovered ? style.thumbHoverColor : style.thumbColor;
    SDL_SetRenderDrawColor(renderer, curThumbColor.r, curThumbColor.g, curThumbColor.b, curThumbColor.a);
    SDL_RenderFillRect(renderer, &thumbScreen);

    // Draw value text
    if (style.drawValueText) {
        std::string valueText = std::to_string(static_cast<int>(st.currentValue));
        int tw=0, th=0;
        TextRenderer::getInstance().measureText(valueText, style.valueTextFontSize, tw, th);
        int tx, ty;

        // Position text relative to the thumb/slider
        if (orientation == XenUI::Orientation::HORIZONTAL) {
            // Below the slider
            tx = (int)std::round(thumbScreen.x + (thumbScreen.w - tw) / 2.0f);
            ty = (int)std::round(finalY + sliderH - th - 2 + viewOffset.y);
        } else {
            // To the right of the slider
            tx = (int)std::round(finalX + sliderW + 2 + viewOffset.x);
            ty = (int)std::round(thumbScreen.y + (thumbScreen.h - th) / 2.0f);
        }
        TextRenderer::getInstance().getInstance().renderText(valueText, tx, ty, style.valueTextColor, style.valueTextFontSize);
    }

    return changed;
}

} // namespace XenUI
// ---------------- Immediate Mode Slider Implementation Ends Here ---------------