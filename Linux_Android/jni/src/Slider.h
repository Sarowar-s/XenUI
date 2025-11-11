// SPDX-License-Identifier: Apache-2.0
/*
 *  Copyright (C) 2025 MD S M Sarowar Hossain
 *
 */
//
// Defines the retained-mode Slider class (IControl) and the immediate-mode
// Slider API function, along with shared styling and state structures.
//

#ifndef SLIDER_H
#define SLIDER_H

#include <SDL3/SDL.h>
#include <string>
#include <functional>
#include <unordered_map>
#include <iostream>
#include <algorithm>

#include "TextRenderer.h" // Required for rendering value text
#include "Position.h"     // Required for XenUI::PositionParams
#include "Orientation.h"  // Required for XenUI::Orientation
#include "UIElement.h"    // Required for IControl base class
#include <cmath>          // Required for general math functions (e.g., std::clamp)

/**
 * @brief Defines the visual style properties for both retained and immediate mode Sliders.
 */
struct SliderStyle {
    SDL_Color trackColor = {60, 60, 60, 255};      ///< Color of the slider track.
    SDL_Color thumbColor = {150, 150, 150, 255};   ///< Color of the slider thumb (default state).
    SDL_Color thumbHoverColor = {180, 180, 180, 255}; ///< Color of the slider thumb when hovered.
    SDL_Color valueTextColor = {255, 255, 255, 255}; ///< Color of the text displaying the current value.
    int trackThickness = 24;                       ///< Thickness of the track in pixels.
    int thumbSize = 40;                           ///< Size (width/height) of the square thumb in pixels.
    int padding = 8;                              ///< Padding around the track ends within the slider bounds.
    bool drawValueText = true;                    ///< Flag to enable/disable drawing the current value as text.
    int valueTextFontSize = 30;                   ///< Font size for the value text.
};

const int DEFAULT_SLIDER_FONT_SIZE = 24;

// -----------------------------------------------------------------------------
// --- Retained Mode Slider Implementation Starts Here (Header) ---
// -----------------------------------------------------------------------------

/**
 * @brief A retained-mode control for selecting a floating-point value within a range.
 *
 * This control handles position calculation, drawing, and user interaction (drag, click jump).
 */
class Slider : public IControl {
public:
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
    Slider(const std::string& id,
           XenUI::Orientation orientation,
           const XenUI::PositionParams& posParams,
           float length, // primary dimension
           float initialValue,
           float minValue,
           float maxValue,
           SliderStyle style = {},
           std::function<void(float)> onValueChanged = nullptr);

    /**
     * @brief Backwards-compatibility wrapper for drawing without an explicit offset.
     *
     * Calls the full draw signature with a zero offset.
     *
     * @param renderer The SDL_Renderer context.
     */
    void draw(SDL_Renderer* renderer) { SDL_FPoint z{0,0}; draw(renderer, z); }

    // --- IControl API Overrides ---

    /**
     * @brief Handles input events, including hover, drag start, drag end, and value jump.
     *
     * Expects event coordinates to be already translated to the control's content-space.
     *
     * @param event The SDL_Event to process.
     * @return true if the event was handled and consumed (e.g., drag started/moved/ended), false otherwise.
     */
    bool handleEvent(const SDL_Event& event) override; // expects content-space coords

    /**
     * @brief Renders the slider track, thumb, and optional value text.
     *
     * @param renderer The SDL_Renderer context.
     * @param viewOffset The screen-space offset inherited from parent containers (e.g., ScrollView).
     */
    void draw(SDL_Renderer* renderer, const SDL_FPoint& viewOffset) override;

    /**
     * @brief Recalculates the slider's final position and resolved size based on parent layout.
     *
     * Computes and stores the control's final content-space coordinates (m_posX/m_posY).
     *
     * @param parentWidth The available width in the parent container.
     * @param parentHeight The available height in the parent container.
     */
    void recalculateLayout(int parentWidth, int parentHeight) override;

    /**
     * @brief Compatibility wrapper for layout calculation using current window size.
     */
    void recalculateLayout(){
        // Note: WindowUtil.h must provide GetWindowSize() for this to compile
        SDL_Point s = XenUI::GetWindowSize();
        recalculateLayout(s.x, s.y);
    }

    /**
     * @brief Returns the absolute content-space bounding box of the entire slider control.
     *
     * @return The SDL_FRect defining the control's bounds.
     */
    SDL_FRect getBounds() const override;

    // --- Programmatic Control ---
    /**
     * @brief Programmatically sets the slider's current value.
     *
     * Clamps the value and invokes the callback if the value changes.
     *
     * @param newValue The new value to set.
     */
    void setValue(float newValue);

    /**
     * @brief Retrieves the current value of the slider.
     * @return The current floating-point value.
     */
    float getValue() const { return m_currentValue; }

    /**
     * @brief Retrieves the unique identifier of the slider.
     * @return The ID string.
     */
    const std::string& getId() const { return m_id; }

private:
    std::string m_id;                   ///< Unique identifier (for state, though not strictly needed in retained mode unless serialized).
    XenUI::Orientation m_orientation;   ///< Horizontal or Vertical.
    XenUI::PositionParams m_posParams;  ///< Layout constraints.
    float m_minValue;                   ///< Minimum possible value.
    float m_maxValue;                   ///< Maximum possible value.
    float m_currentValue;               ///< The current selected value.
    SliderStyle m_style;                ///< Visual style.
    std::function<void(float)> m_onValueChanged; ///< Callback function.

    int m_posX, m_posY;                 ///< Content-space top-left position.
    int m_width, m_height;              ///< Resolved size of the slider's bounding box.

    bool m_isDragging;                  ///< True if the thumb is currently being dragged.
    bool m_isHovered;                   ///< True if the thumb is currently being hovered.
    float m_dragOffset;                 ///< Offset from the center of the thumb where the drag started (to maintain grab point).

    /**
     * @brief Internal helper to calculate the thumb's content-space bounding box based on m_currentValue.
     * @return The SDL_FRect for the thumb.
     */
    SDL_FRect getThumbRectContent() const;

    /**
     * @brief Internal helper to check if a point is within the thumb's content-space bounds.
     * @param x Content-space X coordinate.
     * @param y Content-space Y coordinate.
     * @return true if the point is in the thumb.
     */
    bool isPointInThumbContent(float x, float y) const;

    /**
     * @brief Internal helper to calculate and update m_currentValue based on a content-space mouse position.
     * @param mouseX Content-space X coordinate.
     * @param mouseY Content-space Y coordinate.
     */
    void updateValueFromMouseContent(float mouseX, float mouseY);
};

// -----------------------------------------------------------------------------
// --- Retained Mode Slider Implementation Ends Here (Header) ---
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --- Immediate Mode Slider API Starts Here ---
// -----------------------------------------------------------------------------
namespace XenUI {
    /**
     * @brief Internal namespace for immediate-mode state management.
     */
    namespace Detail {
        /**
         * @brief Structure holding the necessary persistent state for an immediate-mode slider.
         */
        struct SliderState {
            float currentValue = 0.0f;  ///< The slider's current value (mirrors the external pointer).
            bool isDragging = false;    ///< Dragging flag.
            bool isHovered = false;     ///< Hover flag.
            float dragOffset = 0.0f;    ///< Offset used to maintain the grab point during drag.
        };
        /**
         * @brief Global map to store the state of all immediate-mode sliders, keyed by their unique ID.
         */
        static std::unordered_map<std::string, SliderState> sliderStates;
    }

    /**
     * @brief Immediate Mode: Renders and manages a slider control using window size as parent fallback.
     *
     * @param id Unique identifier (for state persistence).
     * @param orientation Slider orientation.
     * @param posParams Positional constraints.
     * @param length Track length.
     * @param value Pointer to the value variable (read/write).
     * @param minValue Minimum value.
     * @param maxValue Maximum value.
     * @param style Visual style.
     * @param viewOffset Screen-space offset from parent (applied to drawing).
     * @param event The current SDL_Event pointer (optional: not directly used by this implementation, which polls mouse state).
     * @return true if the slider's value was adjusted this call/frame, false otherwise.
     */
     bool Slider(const std::string& id,
                XenUI::Orientation orientation,
                const XenUI::PositionParams& posParams,
                float length,
                float* value,
                float minValue,
                float maxValue,
                SliderStyle style = {},
                const SDL_FPoint& viewOffset = {0.0f, 0.0f},
                const SDL_Event* event = nullptr);

    /**
     * @brief Immediate Mode: Parent-aware implementation of the slider control.
     *
     * This overload is used when the immediate-mode slider is placed within a container (like a ScrollView)
     * where parent dimensions are known and required for correct anchor resolution.
     *
     * @param id Unique identifier.
     * @param orientation Slider orientation.
     * @param posParams Positional constraints.
     * @param length Track length.
     * @param value Pointer to the value variable (read/write).
     * @param minValue Minimum value.
     * @param maxValue Maximum value.
     * @param parentWidth The width of the parent container for layout resolution.
     * @param parentHeight The height of the parent container for layout resolution.
     * @param style Visual style.
     * @param viewOffset Screen-space offset from parent (applied to drawing).
     * @param event The current SDL_Event pointer (optional).
     * @return true if the slider's value was adjusted this call/frame, false otherwise.
     */
    bool Slider(const std::string& id,
                XenUI::Orientation orientation,
                const XenUI::PositionParams& posParams,
                float length,
                float* value,
                float minValue,
                float maxValue,
                int parentWidth,
                int parentHeight,
                SliderStyle style = {},
                const SDL_FPoint& viewOffset = {0.0f, 0.0f},
                const SDL_Event* event = nullptr);
} // namespace XenUI
// -----------------------------------------------------------------------------
// --- Immediate Mode Slider API Ends Here ---
// -----------------------------------------------------------------------------

#endif // SLIDER_H