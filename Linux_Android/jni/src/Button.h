#ifndef BUTTON_H
#define BUTTON_H
// SPDX-License-Identifier: Apache-2.0
/*
 *  Copyright (C) 2025 MD S M Sarowar Hossain
 *
 * 
 * This header file defines the structure for a visually styled Button component
 * within the XenUI framework. It includes definitions for both the
 * Retained Mode (class-based) Button and the Immediate Mode (function-based)
 * Button interfaces, along with the ButtonStyle structure for appearance configuration.
 */

#include <SDL3/SDL.h>
#include <string>
#include <functional>
#include "TextRenderer.h" // Dependency for text measurement and rendering
#include "Anchor.h"       // Defines the Anchor enumeration
#include "Position.h"     // Defines the PositionParams structure and related logic
#include "UIElement.h"    // Defines the base interface (IControl) for retained UI components
#include <iostream>
#include <unordered_map>  // Required for internal state tracking in Immediate Mode

/**
 * @brief Defines the visual properties and padding of a Button component.
 *
 * This structure allows for comprehensive customization of the button's appearance,
 * including its state colors and layout padding.
 */
struct ButtonStyle {
    /** @brief Background color in the normal state. */
    SDL_Color bgColor = {100, 100, 100, 255};
    /** @brief Color of the text rendered on the button. */
    SDL_Color textColor = {255, 255, 255, 255};
    /** @brief Flag to enable/disable background drawing. */
    bool drawBackground = true;
    /** @brief Flag to enable/disable border drawing. */
    bool drawBorder = true;
    /** @brief Horizontal padding (pixels) added around the text. */
    int paddingX = 10;
    /** @brief Vertical padding (pixels) added around the text. */
    int paddingY = 5;
    /** @brief Background color when the mouse is hovering over the button. */
    SDL_Color hoverColor = {120, 120, 120, 255};
    /** @brief Background color when the button is actively being pressed. */
    SDL_Color pressedColor = {80, 80, 80, 255};
};

/** @brief Default font size used for buttons if no specific size is provided. */
const int DEFAULT_BUTTON_FONT_SIZE = 16;

/*
 * =================================================================
 * |              RETAINED MODE BUTTON (class Button)              |
 * =================================================================
 */

/**
 * @brief Represents a stateful, Retained Mode UI Button.
 *
 * This class inherits from IControl and maintains its own state (position, size,
 * hover/press status) across multiple frames. It implements the IControl
 * interface for drawing, event handling, and layout management.
 */
class Button : public IControl {
public:
    /**
     * @brief Constructor for the Retained Mode Button.
     *
     * Initializes the button with all necessary layout and visual parameters.
     * Dimensions are calculated in the constructor based on text size and padding.
     *
     * @param text The string to display on the button.
     * @param posParams The PositionParams structure defining the anchor and offsets relative to the parent.
     * @param style The visual style (colors, padding, border) of the button.
     * @param onClick The functional callback to execute when the button is clicked.
     * @param fontSize The size of the text to be rendered. Defaults to DEFAULT_BUTTON_FONT_SIZE.
     */
    Button(const std::string& text,
           const XenUI::PositionParams& posParams,
           ButtonStyle style,
           std::function<void()> onClick,
           int fontSize = DEFAULT_BUTTON_FONT_SIZE);
    
    /**
     * @brief Handles an SDL input event (IControl interface).
     *
     * Updates the button's internal state (hover, pressed) based on mouse events
     * and triggers the onClick callback upon a successful click sequence.
     *
     * @param event The SDL_Event structure containing input details.
     * @return bool True if the event was consumed by the button, false otherwise.
     */
    bool handleEvent(const SDL_Event& event) override;

    /**
     * @brief Renders the button to the screen (IControl interface).
     *
     * Draws the background, border, and text, adjusting position by the view offset.
     *
     * @param renderer The SDL_Renderer instance used for drawing.
     * @param viewOffset The scroll/view offset of the parent container, applied to the position.
     */
    void draw(SDL_Renderer* renderer, const SDL_FPoint& viewOffset) override;

    /**
     * @brief Recalculates the button's layout using explicit parent dimensions (IControl interface).
     *
     * Re-measures text, recalculates dimensions (width/height), and resolves the
     * final relative position (m_posX/m_posY) based on anchor parameters.
     *
     * @param parentWidth The width of the parent container's content area.
     * @param parentHeight The height of the parent container's content area.
     */
    void recalculateLayout(int parentWidth, int parentHeight) override;

    /**
     * @brief Recalculates the layout using the current global window size as the parent dimensions.
     *
     * Convenience function for top-level controls not placed within a scrolling container.
     */
    void recalculateLayout(){
        SDL_Point s = XenUI::GetWindowSize();
        recalculateLayout(s.x, s.y);
    }

    /**
     * @brief Returns the button's current bounds relative to its parent content area (IControl interface).
     *
     * @return SDL_FRect A rectangle defining the relative position and size of the button.
     */
    SDL_FRect getBounds() const override { return { (float)m_posX, (float)m_posY, (float)m_width, (float)m_height }; }

private:
    std::string m_text;
    /** @brief The positioning intent (Anchor and offsets). */
    XenUI::PositionParams m_posParams;
    
    /** @brief The calculated X position, relative to the parent's content area. */
    int m_posX;
    /** @brief The calculated Y position, relative to the parent's content area. */
    int m_posY;
    /** @brief The calculated final width of the button. */
    int m_width;
    /** @brief The calculated final height of the button. */
    int m_height;

    /** @brief The configured visual style. */
    ButtonStyle m_style;
    /** @brief The function executed on click. */
    std::function<void()> m_onClick;

    /** @brief The size of the text font. */
    int m_fontSize;
    /** @brief Internal state flag: True if the left mouse button is currently held down over the button. */
    bool m_isPressed = false;
    /** @brief Internal state flag: True if the press sequence started while the mouse was inside the button. */
    bool m_wasInside = false;
    /** @brief Internal state flag: True if the mouse is currently hovering over the button. */
    bool m_isHovered = false;
};

/*
 * =================================================================
 * |             IMMEDIATE MODE BUTTON (XenUI::Button)             |
 * =================================================================
 */

namespace XenUI {
    namespace Detail {
        /**
         * @brief Internal structure for managing the state of an Immediate Mode Button instance.
         *
         * State must be persisted between frames using a unique ID.
         */
        struct ButtonState {
            /** @brief True if the button is currently pressed. */
            bool isPressed = false;
            /** @brief True if the press started inside the button bounds. */
            bool wasInside = false;
            /** @brief True if the mouse is currently hovering over the button. */
            bool isHovered = false;
        };
        /** @brief Global static map to store state for all active Immediate Mode Buttons, keyed by unique ID. */
        static std::unordered_map<std::string, ButtonState> buttonStates;
    }

    /**
     * @brief Renders and handles an Immediate Mode Button for a single frame.
     *
     * This function calculates layout, draws the component, processes input,
     * and updates its internal state (stored in a static map).
     *
     * @param id A unique string identifier for state tracking across frames.
     * @param text The string to display on the button.
     * @param posParams The position parameters (Anchor and offset).
     * @param renderer The SDL_Renderer instance required for drawing.
     * @param viewOffset The scroll/view offset to apply to the position (e.g., from BeginScrollView). Defaults to {0.0f, 0.0f}.
     * @param style The visual style of the button. Defaults to an empty ButtonStyle (using defaults).
     * @param fontSize The size of the text. Defaults to DEFAULT_BUTTON_FONT_SIZE.
     * @param triggerOnPress If true, returns true on mouse button down; if false, returns true on mouse button release (standard click).
     * @param parentWidth The width of the parent container's content area. Uses window size if <= 0.
     * @param parentHeight The height of the parent container's content area. Uses window size if <= 0.
     * @return bool True if the button was activated (clicked) this frame, false otherwise.
     */
    bool Button(const std::string& id,
                const std::string& text,
                const PositionParams& posParams,
                SDL_Renderer* renderer,

                const SDL_FPoint& viewOffset = {0.0f, 0.0f},
                ButtonStyle style = {},
                int fontSize = DEFAULT_BUTTON_FONT_SIZE,
                bool triggerOnPress = false,
                int parentWidth = -1,
                int parentHeight = -1
            );
} // namespace XenUI


#endif // BUTTON_H