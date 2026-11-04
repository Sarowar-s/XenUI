// SPDX-License-Identifier: Apache-2.0
/*
 *  Copyright (C) 2025 MD S M Sarowar Hossain
 */
//
// Implements both the Retained Mode (class Label) and Immediate Mode (function Label)
// functionalities for displaying static text.
//

#include "Label.h"
#include "SDL3/SDL_log.h"

//
// --- Retained Mode Implementation (class Label) starts here ---
//


Label::Label(const std::string& text,
             const XenUI::PositionParams& posParams,
             int fontSize,
             const SDL_Color& color)
: m_text(text),
  m_posParams(posParams),
  m_fontSize(fontSize > 0 ? fontSize : 12),
  m_color(color),
  m_textRenderer(TextRenderer::getInstance()), 
  m_width(0), m_height(0), m_x(0), m_y(0)
{
    if (!m_textRenderer.isInitialized()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Label Constructor: TextRenderer not initialized!");
        return;
    }

   
    SDL_Point size = m_textRenderer.getTextSize(m_text, m_fontSize);
    m_width = size.x;
    m_height = size.y;

   
    SDL_Point pos = XenUI::CalculateFinalPosition(m_posParams, m_width, m_height);
    m_x = pos.x;
    m_y = pos.y;

    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Label created: '%s' at (%d,%d) with size (%d,%d)",
                 m_text.c_str(), m_x, m_y, m_width, m_height);
}

void Label::setText(const std::string& text) {
    if (m_text == text) return;
    m_text = text;

    if (!m_textRenderer.isInitialized()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Label::setText: TextRenderer not initialized!");
        m_width = 0;
        m_height = 0;
    } else {
      
        SDL_Point size = m_textRenderer.getTextSize(m_text, m_fontSize);
        m_width = size.x;
        m_height = size.y;
    }

 
    recalculateLayout();
}


void Label::setPosition(int x, int y) {
    m_x = x;
    m_y = y;
}


void Label::setColor(const SDL_Color& color) {
    m_color = color;
}


void Label::draw(SDL_Renderer* renderer) {
    SDL_FPoint zero = {0.0f, 0.0f};
    draw(renderer, zero);
}


void Label::draw(SDL_Renderer* renderer, const SDL_FPoint& viewOffset) {
    if (!m_textRenderer.isInitialized()) {
        return;
    }
    if (m_text.empty()) return;

    // Calculate the final screen position
    float finalX = static_cast<float>(m_x) + viewOffset.x;
    float finalY = static_cast<float>(m_y) + viewOffset.y;

    // Render text using the TextRenderer singleton
    m_textRenderer.getInstance().renderText(m_text, static_cast<int>(finalX), static_cast<int>(finalY), m_color, m_fontSize);
}


bool Label::handleEvent(const SDL_Event& /*e*/) {
    // Label is non-interactive by default.
    return false;
}


void Label::recalculateLayout(int parentWidth, int parentHeight) {
    
    if (!m_textRenderer.isInitialized()) {
        m_width = m_height = 0;
    } else {
        SDL_Point size = m_textRenderer.getTextSize(m_text, m_fontSize );
        m_width = size.x;
        m_height = size.y;
    }

    
    SDL_Point pos = XenUI::CalculateFinalPosition(m_posParams, m_width, m_height, parentWidth, parentHeight);
    m_x = pos.x;
    m_y = pos.y;
}


SDL_FRect Label::getBounds() const {
    return SDL_FRect{ static_cast<float>(m_x), static_cast<float>(m_y),
                      static_cast<float>(m_width), static_cast<float>(m_height) };
}


void Label::setWindow(SDL_Window* window) {
    m_window = window;
    
}


void Label::setViewOffset(const SDL_FPoint& viewOffset) {
    m_viewOffset = viewOffset;
  
}


//
// --- Retained Mode Implementation (class Label) ends here ---
//


//
// --- Immediate Mode Implementation (namespace XenUI functions) starts here ---
//

namespace XenUI {

void Label(const std::string& text,
           const XenUI::PositionParams& posParams,
           int fontSize,

           const SDL_Color& color,
           int parentWidth,
           int parentHeight,
           const SDL_FPoint& viewOffset)
{
    auto& textRenderer = TextRenderer::getInstance();
    if (!textRenderer.isInitialized() || text.empty() || fontSize <= 0) {
        return;
    }

    int w = 0, h = 0;

    SDL_Texture* tex = textRenderer.getInstance().renderTextToTexture(text, color, fontSize, w, h);
    if (!tex) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "XenUI::Label (Immediate): Failed to create texture for '%s'", text.c_str());
        return;
    }

    
    int pW = parentWidth;
    int pH = parentHeight;
    if (pW <= 0 || pH <= 0) {
        SDL_Point win = XenUI::GetWindowSize();
        pW = win.x;
        pH = win.y;
    }

  
    SDL_Point pos = XenUI::CalculateFinalPosition(posParams, w, h, pW, pH);

    float finalX = (float)pos.x + viewOffset.x;
    float finalY = (float)pos.y + viewOffset.y;

   
    textRenderer.getInstance().renderText(text, (int)finalX, (int)finalY, color, fontSize);
}


void Label(const std::string& text,
           const XenUI::PositionParams& posParams,
           int fontSize,
           const SDL_Color& color)
{
    SDL_FPoint zero = {0.0f, 0.0f};
   
    Label(text, posParams, fontSize,  color, -1, -1, zero);
}

} // namespace XenUI

//
// --- Immediate Mode Implementation (namespace XenUI functions) ends here ---
