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



//============================ RETAINED MODE IMPLEMENTATION =====================================

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
        // Set border color 
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


bool Button::handleEvent(const SDL_Event& event) {
    bool changed = false;


    float mouseX = (float)event.button.x;
    float mouseY = (float)event.button.y;

    // Check if the relative mouse position is over the button's relative bounds
    bool isCurrentlyInside =
        (mouseX >= m_posX && mouseX < (m_posX + m_width) &&
         mouseY >= m_posY && mouseY < (m_posY + m_height));

    // Check and update hover state
    if (m_isHovered != isCurrentlyInside) {
        m_isHovered = isCurrentlyInside;
        changed = true; 
    }

    if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        if (event.button.button == SDL_BUTTON_LEFT) {
            if (isCurrentlyInside) {
                m_isPressed = true;
                m_wasInside = true;
                changed = true;
                return true; 
            }
        }
    } else if (event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
        if (event.button.button == SDL_BUTTON_LEFT) {
            if (m_isPressed) { // Check if we were tracking a press
                if (m_wasInside && isCurrentlyInside) { // Check for a successful click
                    if (m_onClick) {
                        m_onClick();
                        changed = true; 
                    }
                }
                
                m_isPressed = false;
                m_wasInside = false;
                changed = true;
                return true; 
            }
        }
    }


    return changed;
}


void Button::recalculateLayout(int parentWidth, int parentHeight) {
    //  Re-measure text and calculate button dimensions
    SDL_Point size = TextRenderer::getInstance().getTextSize(m_text, m_fontSize);
    m_width = size.x + m_style.paddingX * 2;
    m_height = size.y + m_style.paddingY * 2;

    //  Compute final relative position using anchor logic
    SDL_Point pos = XenUI::CalculateFinalPosition(m_posParams, m_width, m_height, parentWidth, parentHeight);
    m_posX = pos.x;
    m_posY = pos.y;


}


 //============================ IMMEDIATE MODE IMPLEMENTATION =====================================


namespace XenUI {

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

        //  Calculate Dimensions
        int textW = 0, textH = 0;
        TextRenderer::getInstance().measureText(text, fontSize, textW, textH);
        int width = textW + 2 * style.paddingX;
        int height = textH + 2 * style.paddingY;

        //  Resolve Parent Dimensions (fallback to window size if not explicitly provided)
        int pW = parentWidth;
        int pH = parentHeight;
        if (pW <= 0 || pH <= 0) {
            SDL_Point win = XenUI::GetWindowSize();
            pW = win.x;
            pH = win.y;
        }

        //  Calculate Relative Position (using Anchor logic)
        SDL_Point relativePos = CalculateFinalPosition(posParams, width, height, pW, pH);

        //  Calculate Absolute Screen Position (by applying viewOffset)
        float finalX = static_cast<float>(relativePos.x) + viewOffset.x;
        float finalY = static_cast<float>(relativePos.y) + viewOffset.y;
        SDL_FRect rect = {finalX, finalY, static_cast<float>(width), static_cast<float>(height)};



        Detail::ButtonState& state = Detail::buttonStates[id];

        //  Determine Color based on current state
        SDL_Color currentColor = style.bgColor;
        if (state.isPressed) {
            currentColor = style.pressedColor;
        } else if (state.isHovered) {
            currentColor = style.hoverColor;
        }

        //  Draw Background and Border
        if (style.drawBackground) {
            SDL_SetRenderDrawColor(renderer, currentColor.r, currentColor.g, currentColor.b, currentColor.a);
            SDL_RenderFillRect(renderer, &rect);
        }
        if (style.drawBorder) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White border
            SDL_RenderRect(renderer, &rect);
        }

        //  Draw Centered Text
        int textDrawX = static_cast<int>(finalX + (width - textW) / 2);
        int textDrawY = static_cast<int>(finalY + (height - textH) / 2);
        TextRenderer::getInstance().renderText(text, textDrawX, textDrawY, style.textColor, fontSize);

        //  Handle Input 
        float mouseX = 0.0f, mouseY = 0.0f;
        Uint32 mouseState = SDL_GetMouseState(&mouseX, &mouseY);

        // Check if raw mouse position is over the button's absolute screen bounds
        bool isCurrentlyInside = (mouseX >= finalX && mouseX < (finalX + width) &&
                                  mouseY >= finalY && mouseY < (finalY + height));

        // Update hover state in the persistent state object
        state.isHovered = isCurrentlyInside;

        bool clicked = false;
        if ((mouseState & SDL_BUTTON_MASK(SDL_BUTTON_LEFT))) { 
            if (isCurrentlyInside && !state.isPressed) {
                state.isPressed = true;
                state.wasInside = true;
                if (triggerOnPress) {
                    clicked = true; 
                }
            }
        } else { 
            if (state.isPressed) { 
                if (state.wasInside && isCurrentlyInside) {
                    if (!triggerOnPress) {
                        clicked = true; 
                    }
                }

                state.isPressed = false;
                state.wasInside = false;
            }
        }

        return clicked;
    }
} // namespace XenUI