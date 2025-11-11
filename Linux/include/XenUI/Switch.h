// SPDX-License-Identifier: Apache-2.0
/*
 *  Copyright (C) 2025 MD S M Sarowar Hossain
 */
//
// Defines the shared structures, the retained-mode Switch class (IControl),
// and the immediate-mode Switch API function (SwitchImmediate).
//

#ifndef XENUI_SWITCH_H
#define XENUI_SWITCH_H

#include <string>
#include <functional>
#include <SDL3/SDL.h>
#include <unordered_map>

#include "UIElement.h"     // Defines IControl base class
#include "Position.h"      // Defines XenUI::PositionParams for layout
#include "WindowUtil.h"    // Provides GetWindowSize, CalculateFinalPosition (used in wrappers)
#include "TextRenderer.h"  // For rendering labels

namespace XenUI {

/**
 * @brief Defines the visual style and dimensions for the toggle Switch control.
 */
struct SwitchStyle {
    SDL_Color trackColorOff    = {180,180,180,255}; ///< Color of the track when the switch is OFF.
    SDL_Color trackColorOn     = {100,200,100,255}; ///< Color of the track when the switch is ON.
    SDL_Color thumbColorOff    = {255,255,255,255}; ///< Color of the thumb when the switch is OFF.
    SDL_Color thumbColorOn     = {255,255,255,255}; ///< Color of the thumb when the switch is ON.
    SDL_Color hoverTrackColor  = {200,200,200,255}; ///< Track color when the switch is hovered.
    SDL_Color hoverThumbColor  = {240,240,240,255}; ///< Thumb color when the switch is hovered.
    float     trackHeight      = 30.0f;           ///< The height of the switch track (determines thumb size).
    float     trackWidth       = 60.0f;            ///< The width of the switch track (determines bounding box size).
    float     thumbPadding     = 3.0f;             ///< Padding space between the thumb and the track edge.

    // --- Text fields ---
    std::string labelOff = "Off";               ///< Label text shown when the switch is OFF (usually on the left side).
    std::string labelOn  = "On";                ///< Label text shown when the switch is ON (usually on the right side).

    int labelFontSize = 14;                     ///< Font size for the ON/OFF labels.
    SDL_Color labelColor = { 20, 20, 20, 255 }; ///< Color for the labels (default dark text).
};


// ---------------- Retained-mode Switch Implementation Starts Here (Header) ----------------

/**
 * @brief A retained-mode toggle switch control.
 *
 * Implements the IControl interface for placement within container hierarchies (e.g., ScrollView).
 */
class Switch : public IControl {
public:
    /**
     * @brief Constructs a retained-mode Switch control.
     *
     * @param posParams The positional constraints (anchors, offsets) for layout.
     * @param style The visual style parameters.
     * @param onToggle Optional callback function invoked when the switch state changes.
     * @param initialState The initial ON/OFF state of the switch.
     */
    Switch(const XenUI::PositionParams& posParams,
           const SwitchStyle& style = {},
           std::function<void(bool)> onToggle = nullptr,
           bool initialState = false);

    // --- IControl API Overrides ---

    /**
     * @brief Handles input events (mouse clicks and movement).
     *
     * Expects event coordinates to be already translated to the control's content-space.
     *
     * @param event The SDL_Event to process.
     * @return true if the event was handled and consumed, false otherwise.
     */
    bool handleEvent(const SDL_Event& event) override; // expects content-space coords

    /**
     * @brief Renders the switch, including the track, thumb, and labels.
     *
     * @param renderer The SDL_Renderer context.
     * @param viewOffset The screen-space offset inherited from parent containers.
     */
    void draw(SDL_Renderer* renderer, const SDL_FPoint& viewOffset) override;

    /**
     * @brief Recalculates the switch's final position based on parent layout.
     *
     * Computes and stores the control's final content-space coordinates (m_posX/m_posY).
     *
     * @param parentWidth The available width in the parent container.
     * @param parentHeight The available height in the parent container.
     */
    void recalculateLayout(int parentWidth, int parentHeight) override; // computes m_posX/m_posY and final w/h in content-space

    /**
     * @brief Compatibility wrapper for layout calculation using current window size.
     */
    void recalculateLayout(){
        // Note: WindowUtil.h must provide GetWindowSize() for this to compile
        SDL_Point s = XenUI::GetWindowSize();
        recalculateLayout(s.x, s.y);
    }

    /**
     * @brief Returns the absolute content-space bounding box of the entire switch control.
     *
     * @return The SDL_FRect defining the control's bounds.
     */
    SDL_FRect getBounds() const override;

    /**
     * @brief Backwards-compatibility wrapper for drawing without an explicit offset.
     * @param renderer The SDL_Renderer context.
     */
    void draw(SDL_Renderer* renderer) { draw(renderer, {0.0f,0.0f}); }

    // --- Programmatic Control ---
    /**
     * @brief Retrieves the current ON/OFF state.
     * @return true if ON, false if OFF.
     */
    bool isOn() const { return m_isOn; }

    /**
     * @brief Programmatically sets the ON/OFF state.
     * @param on The desired state.
     */
    void setOn(bool on);

private:
    // Position & style configuration
    XenUI::PositionParams m_posParams;          ///< Layout constraints.
    SwitchStyle m_style;                        ///< Visual style.
    std::function<void(bool)> m_onToggle;       ///< State change callback.

    // Runtime state
    bool m_isOn;                                ///< The current ON/OFF state.
    bool m_hovered;                             ///< True if the cursor is over the switch bounds.
    bool m_isPressed;                           ///< True if the left mouse button is down while over the switch.
    bool m_wasInside;                           ///< True if the press started inside the switch bounds.

    // Resolved geometry (content-space)
    float m_posX, m_posY;                       ///< Content-space top-left position.
    float m_width, m_height;                    ///< Resolved size (fixed by trackWidth/trackHeight).

    /**
     * @brief Internal helper: Calculates the X coordinate of the thumb's center in content-space.
     * @return The thumb's center X coordinate.
     */
    float getThumbCenterX_Content() const;

    /**
     * @brief Internal helper: Calculates the bounding box of the thumb in content-space.
     * @return The SDL_FRect for the thumb.
     */
    SDL_FRect getThumbRect_Content() const;
};

// ---------------- Retained-mode Switch Implementation Ends Here (Header) ----------------


// ---------------- Immediate-mode Switch Implementation Starts Here (Header) ----------------

namespace Detail {
    /**
     * @brief Structure holding the necessary persistent state for an immediate-mode switch.
     *
     * Used internally to manage hover and press tracking across frames.
     */
    struct ImmediateSwitchState {
        bool hovered = false;       ///< Hover state for drawing.
        bool isPressed = false;     ///< Press state for input tracking.
        bool wasInside = false;     ///< Start-of-press location tracking.
    };
    /**
     * @brief External map to store the transient state of all immediate-mode switches, keyed by their unique ID.
     */
    extern std::unordered_map<std::string, ImmediateSwitchState> immediateSwitchStates;
}

/**
 * @brief Immediate Mode: Renders and manages a toggle switch control.
 *
 * This function handles rendering, input, and updates the external state variable pointed to by `pValue`.
 *
 * @param id Unique identifier (must be consistent and unique per switch instance).
 * @param posParams Positional constraints (content-space coordinates).
 * @param style Visual style parameters.
 * @param pValue Pointer to the external boolean state variable (required for read/write).
 * @param triggerOnPress If true, toggle occurs on mouse down; otherwise, on mouse button release.
 * @param parentWidth The width of the parent container (for anchor/position resolution).
 * @param parentHeight The height of the parent container (for anchor/position resolution).
 * @param viewOffset Screen-space offset from parent (applied to drawing, typically from a ScrollView).
 * @param event The current SDL_Event pointer (optional, not strictly used as mouse state is polled).
 * @return true if the external value (*pValue) was toggled this frame, false otherwise.
 */
bool SwitchImmediate(const std::string& id,
                     const XenUI::PositionParams& posParams,
                     const XenUI::SwitchStyle& style = {},
                     bool* pValue = nullptr,
                     bool triggerOnPress = false,
                     int parentWidth = -1,
                     int parentHeight = -1,
                     const SDL_FPoint& viewOffset = {0.0f, 0.0f},
                     const SDL_Event* event = nullptr);


} // namespace XenUI

#endif // XENUI_SWITCH_H