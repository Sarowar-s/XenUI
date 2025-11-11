// SPDX-License-Identifier: Apache-2.0
/*
 *  Copyright (C) 2025 MD S M Sarowar Hossain
 */
//
// Defines the retained mode RadioButton and RadioButtonGroup classes,
// as well as the immediate mode XenUI::RadioGroupImmediate function.
//
#ifndef RADIOBUTTON_H
#define RADIOBUTTON_H

#include <SDL3/SDL.h>
#include <vector>
#include <string>
#include <functional>
#include <memory>
#include "Position.h"     // Defines PositionParams and CalculateFinalPosition
#include "TextRenderer.h" // For text rendering and measurement
#include "UIElement.h"    // Base interface for retained-mode controls (IControl)
#include "WindowUtil.h"   // For GetWindowSize

#ifndef DEFAULT_RADIO_FONT_SIZE
#define DEFAULT_RADIO_FONT_SIZE 20 ///< Default point size for radio button labels.
#endif

/**
 * @brief Defines the visual style properties for a RadioButton.
 */
struct RadioButtonStyle {
    SDL_Color textColor = {255,255,255,255};      ///< Color of the button text (legacy/unused if labelColor is set).
    SDL_Color circleColor = {200,200,200,255};    ///< Color of the outer circle outline.
    SDL_Color selectedColor = {10,200,100,255};   ///< Color of the inner fill when selected.
    SDL_Color labelColor = {255,255,255,255};     ///< Color of the label text.
    int circleRadius = 10;                        ///< Radius of the outer circle in pixels.
    int circlePadding = 8;                        ///< Horizontal space between the circle and the label.
    int innerCirclePadding = 4;                   ///< Padding between the outer circle edge and the inner selected fill.
};

class RadioButtonGroup; // Forward declaration of the container class

// -----------------------------------------------------------------------------
// --- Retained Mode Implementation Starts Here ---
// -----------------------------------------------------------------------------

/**
 * @brief A retained-mode control representing a single selectable option in a RadioButtonGroup.
 *
 * This control handles its own rendering and hit testing, reporting selection changes
 * back to its parent RadioButtonGroup.
 */
class RadioButton : public IControl {
public:
    /**
     * @brief Constructs a retained-mode RadioButton.
     *
     * @param group The parent RadioButtonGroup instance (reference).
     * @param label The text to display next to the button.
     * @param value The integer value associated with this button's selection.
     * @param posParams The positional constraints for layout calculation.
     * @param style The visual style parameters (defaults to a sensible style).
     * @param fontSize The font size for the label text.
     */
    RadioButton(RadioButtonGroup& group,
                std::string label,
                int value,
                XenUI::PositionParams posParams,
                RadioButtonStyle style = {},
                int fontSize = DEFAULT_RADIO_FONT_SIZE);

    // --- IControl Interface Overrides ---

    /**
     * @brief Handles mouse input to detect hovering and selection clicks.
     * @param e The SDL_Event to process (assumed to have content-space coordinates).
     * @return true if the button's state changed (hover or selection), false otherwise.
     */
    bool handleEvent(const SDL_Event& e) override;

    /**
     * @brief Renders the radio button circle, selection state, and label.
     * @param renderer The SDL_Renderer context.
     * @param viewOffset The content-to-screen translation offset (e.g., scroll position).
     */
    void draw(SDL_Renderer* renderer, const SDL_FPoint& viewOffset) override;

    /**
     * @brief Recalculates the button's total size and content-space position.
     *
     * This method ensures the bounds encompass both the circle and the label.
     *
     * @param parentWidth The width of the parent container or content area.
     * @param parentHeight The height of the parent container or content area.
     */
    void recalculateLayout(int parentWidth, int parentHeight) override;

    /**
     * @brief Returns the bounding box of the control in content-space.
     * @return The SDL_FRect defining the button's total area.
     */
    SDL_FRect getBounds() const override { return m_bounds; }

    // --- Compatibility ---

    /**
     * @brief Legacy draw method assuming zero view offset.
     * @param renderer The SDL_Renderer context.
     */
    void draw(SDL_Renderer* renderer) { SDL_FPoint zero{0,0}; draw(renderer, zero); }

private:
    RadioButtonGroup& m_group;                  ///< Reference to the parent group controlling selection.
    std::string m_label;                        ///< The text displayed next to the button.
    int m_value;                                ///< The unique integer value this button represents.
    XenUI::PositionParams m_posParams;          ///< Parameters for layout resolution.
    RadioButtonStyle m_style;                   ///< Visual style.
    int m_fontSize;                             ///< Font size of the label.

    SDL_FRect m_bounds{0,0,0,0};                ///< The final calculated bounding box in content-space.
    SDL_FPoint m_circleCenter{0,0};             ///< The center point of the circular element in content-space.
    float m_textWidth = 0.0f;                   ///< Calculated width of the label text.
    float m_textHeight = 0.0f;                  ///< Calculated height of the label text.
    bool m_isHovered = false;                   ///< Current hover state for visual feedback.
};

/**
 * @brief A retained-mode container that manages a collection of RadioButton controls.
 *
 * This class ensures only one RadioButton is selected at a time by managing a pointer
 * to an external integer value and providing event propagation and drawing control.
 */
class RadioButtonGroup : public IControl {
public:
    /**
     * @brief Constructs a retained-mode RadioButtonGroup.
     *
     * @param selectedValue Pointer to an integer variable that stores the currently selected value.
     * @param onSelectionChange Optional callback function invoked when selection changes.
     */
    RadioButtonGroup(int* selectedValue, std::function<void(int)> onSelectionChange = nullptr);

    /**
     * @brief Creates and adds a new retained-mode RadioButton to the group.
     *
     * @param label The text for the new button.
     * @param value The value associated with the new button.
     * @param posParams Positional constraints for the new button.
     * @param style Visual style for the new button.
     * @param fontSize Font size for the new button's label.
     */
    void addButton(const std::string& label,
                   int value,
                   XenUI::PositionParams posParams,
                   RadioButtonStyle style = {},
                   int fontSize = DEFAULT_RADIO_FONT_SIZE);

    // --- IControl Interface Overrides (Group Container) ---

    /**
     * @brief Forwards the event to all child RadioButtons for handling.
     * @param e The SDL_Event to process.
     * @return true if any child button handled the event, false otherwise.
     */
    bool handleEvent(const SDL_Event& e) override;

    /**
     * @brief Draws all contained RadioButtons.
     * @param renderer The SDL_Renderer context.
     * @param viewOffset The content-to-screen translation offset.
     */
    void draw(SDL_Renderer* renderer, const SDL_FPoint& viewOffset) override;

    /**
     * @brief Recalculates the layout for all child buttons and updates the overall group bounds.
     * @param parentWidth The width of the parent container or content area.
     * @param parentHeight The height of the parent container or content area.
     */
    void recalculateLayout(int parentWidth, int parentHeight) override;

    /**
     * @brief Gets the minimum bounding box that encompasses all buttons in the group.
     * @return The SDL_FRect representing the group's combined bounds in content-space.
     */
    SDL_FRect getBounds() const override;

    // --- Internal Selection Management ---

    /**
     * @brief Updates the selected value and triggers the selection change callback.
     *
     * Called by a child RadioButton when it is clicked.
     *
     * @param value The value of the newly selected button.
     */
    void notifySelection(int value);

    /**
     * @brief Checks if a specific value corresponds to the currently selected radio button.
     * @param value The value to check.
     * @return true if the value is currently selected, false otherwise.
     */
    bool isSelected(int value) const;

    // --- Compatibility ---

    /**
     * @brief Legacy draw method assuming zero view offset.
     * @param renderer The SDL_Renderer context.
     */
    void draw(SDL_Renderer* renderer) { draw(renderer, {0,0}); }

    /**
     * @brief Legacy event handler wrapper.
     * @param e The SDL_Event to process.
     * @return true if the event was handled.
     */
    bool handleEventLegacy(const SDL_Event& e) { return handleEvent(e); }

private:
    std::vector<std::unique_ptr<RadioButton>> m_buttons; ///< Collection of managed RadioButton children.
    int* m_selectedValue;                                ///< Pointer to the external variable storing the selection.
    std::function<void(int)> m_onSelectionChangeCallback;///< Callback for selection changes.
    SDL_FRect m_groupBounds{0,0,0,0};                     ///< The minimum bounding box of all contained buttons.
};

// -----------------------------------------------------------------------------
// --- Retained Mode Implementation Ends Here ---
// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------
// --- Immediate Mode Implementation Starts Here ---
// -----------------------------------------------------------------------------

namespace XenUI {
    /**
     * @brief Renders an immediate-mode radio button group (vertical list) and handles input.
     *
     * This function performs layout calculation, hit-testing, input processing, and drawing
     * for a set of radio options within a single frame's call.
     *
     * @param id A unique identifier string (for internal IM state tracking, if implemented).
     * @param options A vector of strings for the button labels.
     * @param selectedIndex Pointer to an integer holding the current 0-based index selection.
     * @param pos The PositionParams for the entire group.
     * @param style The visual style parameters (defaults to a sensible style).
     * @param fontSize Font size for the labels.
     * @param spacing Minimum vertical spacing between the items.
     * @param viewOffset The content-to-screen translation offset (e.g., scroll position, defaults to {0,0}).
     * @param parentWidth The width of the content area (or -1 to use window size).
     * @param parentHeight The height of the content area (or -1 to use window size).
     * @param event Optional pointer to the current frame's SDL_Event for click detection.
     * @return true if the selection index was changed by user interaction this frame, false otherwise.
     */
    bool RadioGroupImmediate(
        const char* id,
        const std::vector<std::string>& options,
        int* selectedIndex,
        const XenUI::PositionParams& pos,
        const RadioButtonStyle& style = {},
        int fontSize = DEFAULT_RADIO_FONT_SIZE,
        int spacing = 35,
        const SDL_FPoint& viewOffset = {0.0f, 0.0f},
        int parentWidth = -1,
        int parentHeight = -1,
        const SDL_Event* event = nullptr
    );
}
// -----------------------------------------------------------------------------
// --- Immediate Mode Implementation Ends Here ---
// -----------------------------------------------------------------------------


#endif // RADIOBUTTON_H