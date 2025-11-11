// SPDX-License-Identifier: Apache-2.0
/*
 *  Copyright (C) 2025 MD S M Sarowar Hossain
 *
 * This file implements the Button component, supporting both:
 * 1. Retained Mode (class-based): Components are created once, maintain their state,
 * and are explicitly drawn and handled each frame.
 * 2. Immediate Mode (function-based): Components are defined and drawn in a single
 * function call each frame; state management is delegated to a static internal map.
 *
 * It manages visual state (hover, pressed) and handles click events, supporting
 * anchoring relative to a parent container.
 */

#include "Button.h"
#include "WindowUtil.h" // For ResolveAnchorPosition dependency and GetWindowSize()
#include <iostream>
#include <utility>      // For std::move
#include <algorithm>    // For std::clamp if needed



/*
 * =================================================================
 * |                    RETAINED MODE IMPLEMENTATION                 |
 * =================================================================
 */

/**
 * @brief Constructs a new Retained Mode Button object.
 *
 * Initializes the button with text, layout parameters, style, and an optional
 * click handler. It immediately calculates the required dimensions based on the text
 * and padding.
 *
 * @param text The string to display on the button.
 * @param posParams The PositionParams structure defining the anchor and offsets.
 * @param style The visual style (colors, padding, border) of the button.
 * @param onClick The functional callback executed when the button is clicked.
 * @param fontSize The size of the text to be rendered. Uses DEFAULT_BUTTON_FONT_SIZE if <= 0.
 */
Button::Button(const std::string& text,
               const XenUI::PositionParams& posParams,
               ButtonStyle style,
               std::function<void()> onClick,
               int fontSize)
    : m_text(text),
      m_posParams(posParams),
      m_style(style),
      m_onClick(std::move(onClick)),
      m_fontSize(fontSize > 0 ? fontSize : DEFAULT_BUTTON_FONT_SIZE),
      m_isHovered(false), // Initialize hover state
      m_isPressed(false), // Initialize pressed state
      m_wasInside(false)  // Initialize flag for tracking release position
{
    // Sanity check to ensure text rendering is possible before measuring text
    if (!TextRenderer::getInstance().isInitialized()) {
        std::cerr << "Error: TextRenderer not initialized when creating Button." << std::endl;
        // The width and height will remain 0, preventing proper layout
    } else {
        int textW = 0, textH = 0;
        // Measure text dimensions using the configured font size
        TextRenderer::getInstance().measureText(m_text, m_fontSize, textW, textH);

        // Calculate the required button dimensions based on text size and padding
        m_width = textW + 2 * m_style.paddingX;
        m_height = textH + 2 * m_style.paddingY;
    }
    // Initial layout calculation, setting m_posX, m_posY relative to parent's content area
    recalculateLayout();
}

/**
 * @brief Renders the retained button to the screen.
 *
 * Draws the button background, border, and centered text, adjusting the position
 * based on the provided view offset (used for scrolling containers).
 *
 * @param renderer The SDL_Renderer instance used for drawing.
 * @param viewOffset The scroll/view offset of the parent container (e.g., ScrollView).
 */
void Button::draw(SDL_Renderer* renderer, const SDL_FPoint& viewOffset) {
    // Calculate the button's absolute position on screen by adding the view offset
    float finalX = static_cast<float>(m_posX) + viewOffset.x;
    float finalY = static_cast<float>(m_posY) + viewOffset.y;
    SDL_FRect rect = {finalX, finalY, static_cast<float>(m_width), static_cast<float>(m_height)};

    // Determine current color based on the state machine (Pressed > Hovered > Normal)
    SDL_Color currentColor = m_style.bgColor;
    if (m_isPressed) {
        currentColor = m_style.pressedColor;
    } else if (m_isHovered) {
        currentColor = m_style.hoverColor;
    }

    // Draw Background
    if (m_style.drawBackground) {
        SDL_SetRenderDrawColor(renderer, currentColor.r, currentColor.g, currentColor.b, currentColor.a);
        SDL_RenderFillRect(renderer, &rect);
    }
    
    // Draw Border
    if (m_style.drawBorder) {
        // Set border color (currently hardcoded to white for simplicity)
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderRect(renderer, &rect);
    }

    // Draw Centered Text
    if (!m_text.empty() && TextRenderer::getInstance().isInitialized()) {
        int textW, textH;
        TextRenderer::getInstance().getInstance().measureText(m_text, m_fontSize, textW, textH);
        
        // Calculate text position to center it within the button rectangle
        int textX = static_cast<int>(finalX + (m_width - textW) / 2);
        int textY = static_cast<int>(finalY + (m_height - textH) / 2);
        
        TextRenderer::getInstance().renderText(m_text, textX, textY, m_style.textColor, m_fontSize);
    }
}

/**
 * @brief Handles an SDL input event for the retained button.
 *
 * Updates the button's state (hover, pressed) based on mouse events and triggers
 * the m_onClick callback if a click (press and release inside) occurs.
 *
 * @param event The SDL_Event structure containing input details.
 * @return bool True if the event was consumed (handled by this button), false otherwise.
 */
bool Button::handleEvent(const SDL_Event& event) {
    bool changed = false;

    /*
     * Note on Coordinates: The incoming event coordinates (event.button.x/y)
     * are assumed to be already translated by the parent container (e.g., a ScrollView)
     * to be relative to the button's content area (where m_posX/m_posY are defined).
     */
    float mouseX = (float)event.button.x;
    float mouseY = (float)event.button.y;

    // Check if the relative mouse position is over the button's relative bounds
    bool isCurrentlyInside =
        (mouseX >= m_posX && mouseX < (m_posX + m_width) &&
         mouseY >= m_posY && mouseY < (m_posY + m_height));

    // Check and update hover state
    if (m_isHovered != isCurrentlyInside) {
        m_isHovered = isCurrentlyInside;
        changed = true; // Indicate state change requires redraw
    }

    if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        if (event.button.button == SDL_BUTTON_LEFT) {
            if (isCurrentlyInside) {
                m_isPressed = true;
                m_wasInside = true;
                changed = true;
                return true; // Consume event if clicked
            }
        }
    } else if (event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
        if (event.button.button == SDL_BUTTON_LEFT) {
            if (m_isPressed) { // Check if we were tracking a press
                if (m_wasInside && isCurrentlyInside) { // Check for a successful click
                    if (m_onClick) {
                        m_onClick();
                        changed = true; // Click handler might update UI
                    }
                }
                // Reset press state regardless of release position
                m_isPressed = false;
                m_wasInside = false;
                changed = true;
                return true; // Consume event as it pertains to our press state
            }
        }
    }
    // SDL_EVENT_MOUSE_MOTION is implicitly handled by the hover check above.

    return changed;
}

/**
 * @brief Recalculates the button's position and size based on parent dimensions.
 *
 * This method should be called whenever the parent container is resized, ensuring
 * the anchor constraints are reapplied. It updates m_width, m_height, m_posX, and m_posY.
 *
 * @param parentWidth The new width of the parent container.
 * @param parentHeight The new height of the parent container.
 */
void Button::recalculateLayout(int parentWidth, int parentHeight) {
    // 1. Re-measure text and calculate button dimensions
    SDL_Point size = TextRenderer::getInstance().getTextSize(m_text, m_fontSize);
    m_width = size.x + m_style.paddingX * 2;
    m_height = size.y + m_style.paddingY * 2;

    // 2. Compute final relative position using anchor logic
    SDL_Point pos = XenUI::CalculateFinalPosition(m_posParams, m_width, m_height, parentWidth, parentHeight);
    m_posX = pos.x;
    m_posY = pos.y;

    // Debugging printout commented out:
    // std::cout << "Button '" << m_text << "' Recalculated Layout: (" << m_posX << ", " << m_posY << ")" << std::endl;
}

/*
 * =================================================================
 * |                    IMMEDIATE MODE IMPLEMENTATION                |
 * =================================================================
 */

namespace XenUI {
    /**
     * @brief Renders an Immediate Mode Button and handles its input for one frame.
     *
     * This function performs all layout, rendering, and input handling in a single call.
     * Its state (hover, pressed) is tracked externally using the unique 'id' and a static map.
     *
     * @param id A unique string identifier for this button instance (used for state persistence).
     * @param text The string to display on the button.
     * @param posParams The position parameters (Anchor and offset).
     * @param renderer The SDL_Renderer instance for drawing.
     * @param viewOffset The scroll/view offset to apply to the position before drawing.
     * @param style The visual style of the button.
     * @param fontSize The size of the text.
     * @param triggerOnPress If true, the button returns true immediately on mouse down. If false, it returns true on mouse up (standard click).
     * @param parentWidth The width of the parent container.
     * @param parentHeight The height of the parent container.
     * @return bool True if the button was "clicked" (according to triggerOnPress logic) this frame, false otherwise.
     */
    bool Button(const std::string& id, const std::string& text,
                const PositionParams& posParams,
                SDL_Renderer* renderer,

                const SDL_FPoint& viewOffset,
                ButtonStyle style,
                int fontSize,
                bool triggerOnPress,
                int parentWidth,
                int parentHeight)
    {
        if (!renderer || !TextRenderer::getInstance().isInitialized()) {
            return false;
        }

        // 1. Calculate Dimensions
        int textW = 0, textH = 0;
        TextRenderer::getInstance().measureText(text, fontSize, textW, textH);
        int width = textW + 2 * style.paddingX;
        int height = textH + 2 * style.paddingY;

        // 2. Resolve Parent Dimensions (fallback to window size if not explicitly provided)
        int pW = parentWidth;
        int pH = parentHeight;
        if (pW <= 0 || pH <= 0) {
            SDL_Point win = XenUI::GetWindowSize();
            pW = win.x;
            pH = win.y;
        }

        // 3. Calculate Relative Position (using Anchor logic)
        SDL_Point relativePos = CalculateFinalPosition(posParams, width, height, pW, pH);

        // 4. Calculate Absolute Screen Position (by applying viewOffset)
        float finalX = static_cast<float>(relativePos.x) + viewOffset.x;
        float finalY = static_cast<float>(relativePos.y) + viewOffset.y;
        SDL_FRect rect = {finalX, finalY, static_cast<float>(width), static_cast<float>(height)};

        // 5. Get/Retrieve Persistent State
        // State (Detail::ButtonState) is looked up using the unique ID
        Detail::ButtonState& state = Detail::buttonStates[id];

        // 6. Determine Color based on current state
        SDL_Color currentColor = style.bgColor;
        if (state.isPressed) {
            currentColor = style.pressedColor;
        } else if (state.isHovered) {
            currentColor = style.hoverColor;
        }

        // 7. Draw Background and Border
        if (style.drawBackground) {
            SDL_SetRenderDrawColor(renderer, currentColor.r, currentColor.g, currentColor.b, currentColor.a);
            SDL_RenderFillRect(renderer, &rect);
        }
        if (style.drawBorder) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White border
            SDL_RenderRect(renderer, &rect);
        }

        // 8. Draw Centered Text
        int textDrawX = static_cast<int>(finalX + (width - textW) / 2);
        int textDrawY = static_cast<int>(finalY + (height - textH) / 2);
        TextRenderer::getInstance().renderText(text, textDrawX, textDrawY, style.textColor, fontSize);

        // 9. Handle Input (Query raw mouse state)
        float mouseX = 0.0f, mouseY = 0.0f;
        Uint32 mouseState = SDL_GetMouseState(&mouseX, &mouseY);

        // Check if raw mouse position is over the button's absolute screen bounds
        bool isCurrentlyInside = (mouseX >= finalX && mouseX < (finalX + width) &&
                                  mouseY >= finalY && mouseY < (finalY + height));

        // Update hover state in the persistent state object
        state.isHovered = isCurrentlyInside;

        bool clicked = false;
        if ((mouseState & SDL_BUTTON_MASK(SDL_BUTTON_LEFT))) { // Mouse Left button is down
            if (isCurrentlyInside && !state.isPressed) {
                state.isPressed = true;
                state.wasInside = true;
                if (triggerOnPress) {
                    clicked = true; // Trigger immediately on press if configured
                }
            }
        } else { // Mouse Left button is UP
            if (state.isPressed) { // If it was previously pressed
                if (state.wasInside && isCurrentlyInside) { // Released inside after pressing inside
                    if (!triggerOnPress) {
                        clicked = true; // Trigger on release if configured
                    }
                }
                // Reset press state
                state.isPressed = false;
                state.wasInside = false;
            }
        }

        return clicked;
    }
} // namespace XenUI