//
// SPDX-License-Identifier: Apache-2.0
/*
 *  Copyright (C) 2025 MD S M Sarowar Hossain
 */
//
// Implements both Retained Mode (RadioButton, RadioButtonGroup) and Immediate Mode
// (XenUI::RadioGroupImmediate) functionalities for radio button selection.
//

#include "RadioButton.h"
#include <cmath>
#include <algorithm>
#include <vector>
#include <SDL3/SDL.h>

static inline bool pointInRectF(float px, float py, const SDL_FRect& r) {
    return (px >= r.x && px < (r.x + r.w) && py >= r.y && py < (r.y + r.h));
}

static void DrawCircle(SDL_Renderer* renderer, int32_t centreX, int32_t centreY, int32_t radius) {
    if (!renderer || radius <= 0) return;
    const int32_t diameter = radius * 2;
    int32_t x = radius - 1;
    int32_t y = 0;
    int32_t tx = 1;
    int32_t ty = 1;
    int32_t err = tx - diameter;
   
    std::vector<SDL_FPoint> pts; pts.reserve(radius * 8);
    
    while (x >= y) {
         pts.push_back({ (float)(centreX + x), (float)(centreY - y) });
         pts.push_back({ (float)(centreX + x), (float)(centreY + y) });
         pts.push_back({ (float)(centreX - x), (float)(centreY - y) });
         pts.push_back({ (float)(centreX - x), (float)(centreY + y) });
         pts.push_back({ (float)(centreX + y), (float)(centreY - x) });
         pts.push_back({ (float)(centreX + y), (float)(centreY + x) });
         pts.push_back({ (float)(centreX - y), (float)(centreY - x) });
         pts.push_back({ (float)(centreX - y), (float)(centreY + x) });
        if (err <= 0) {
            ++y;
            err += ty;
            ty += 2;
        }
        if (err > 0) {
            --x;
            tx += 2;
            err += (tx - diameter);
        }
    }
    // Draw all calculated points
    if (!pts.empty()) SDL_RenderPoints(renderer, pts.data(), (int)pts.size());
}

static void DrawFilledCircle(SDL_Renderer* renderer, int centerX, int centerY, int radius) {
    if (!renderer || radius <= 0) return;
   
    for (int y = -radius; y <= radius; ++y) {
      
        int dx = (int)std::floor(std::sqrt((float)(radius*radius - y*y)));
       
        SDL_RenderLine(renderer, centerX - dx, centerY + y, centerX + dx, centerY + y);
    }
}

// ---------------- Retained Mode Implementation Starts Here ----------------


RadioButton::RadioButton(RadioButtonGroup& group, std::string label, int value,
                         XenUI::PositionParams posParams, RadioButtonStyle style, int fontSize)
    : m_group(group),
      m_label(std::move(label)),
      m_value(value),
      m_posParams(posParams),
      m_style(style),
      m_fontSize(fontSize > 0 ? fontSize : DEFAULT_RADIO_FONT_SIZE),
      m_isHovered(false) 
{
   
    SDL_Point winSize = XenUI::GetWindowSize();
    int parentWidth = winSize.x > 0 ? winSize.x : 800; // Fallback width
    int parentHeight = winSize.y > 0 ? winSize.y : 600; // Fallback height
    recalculateLayout(parentWidth, parentHeight);
}



void RadioButton::recalculateLayout(int parentWidth, int parentHeight) {
    int textW = 0, textH = 0;
    // Measure the label text dimensions
    if (TextRenderer::getInstance().isInitialized()) {
        TextRenderer::getInstance().measureText(m_label, m_fontSize, textW, textH);
    }
    m_textWidth = static_cast<float>(textW);
    m_textHeight = static_cast<float>(textH);

    // Calculate total required width and height
    int totalW = (m_style.circleRadius * 2) + m_style.circlePadding + textW;
    int totalH = std::max((m_style.circleRadius * 2), textH); // Height is max of circle diameter or text height

    
    SDL_Point pos = XenUI::CalculateFinalPosition(m_posParams, totalW, totalH, parentWidth, parentHeight);

    
    m_bounds.x = static_cast<float>(pos.x);
    m_bounds.y = static_cast<float>(pos.y);
    m_bounds.w = static_cast<float>(totalW);
    m_bounds.h = static_cast<float>(totalH);

   
    m_circleCenter.x = m_bounds.x + m_style.circleRadius;
    m_circleCenter.y = m_bounds.y + m_bounds.h / 2.0f;
}



bool RadioButton::handleEvent(const SDL_Event& e) {
    bool changed = false;

    float mx = 0.0f, my = 0.0f;
    if (e.type == SDL_EVENT_MOUSE_MOTION) {
        mx = (float)e.motion.x; my = (float)e.motion.y;
    } else if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN || e.type == SDL_EVENT_MOUSE_BUTTON_UP) {
        mx = (float)e.button.x; my = (float)e.button.y;
    } else {
        return false;
    }

    
    bool inside = pointInRectF(mx, my, m_bounds);
    if (inside != m_isHovered) {
        m_isHovered = inside;
        changed = true; 
    }

   
    if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN && e.button.button == SDL_BUTTON_LEFT) {
        if (inside) {
            
            m_group.notifySelection(m_value);
            changed = true;
        }
    }
    return changed;
}


void RadioButton::draw(SDL_Renderer* renderer, const SDL_FPoint& viewOffset) {
    if (!renderer) return;
    bool selected = m_group.isSelected(m_value);

   
    float drawCenterX = m_circleCenter.x + viewOffset.x;
    float drawCenterY = m_circleCenter.y + viewOffset.y;

   
    SDL_SetRenderDrawColor(renderer, m_style.circleColor.r, m_style.circleColor.g, m_style.circleColor.b, m_style.circleColor.a);
    DrawCircle(renderer, (int)std::round(drawCenterX), (int)std::round(drawCenterY), m_style.circleRadius);

   
    if (selected) {
        SDL_SetRenderDrawColor(renderer, m_style.selectedColor.r, m_style.selectedColor.g, m_style.selectedColor.b, m_style.selectedColor.a);
        DrawFilledCircle(renderer, (int)std::round(drawCenterX), (int)std::round(drawCenterY), m_style.circleRadius - m_style.innerCirclePadding);
    }

  
    if (TextRenderer::getInstance().isInitialized() && !m_label.empty()) {
       
        float textX = drawCenterX + m_style.circleRadius + m_style.circlePadding;
        float textY = drawCenterY - (m_textHeight / 2.0f);
       TextRenderer::getInstance().renderText(m_label, (int)std::round(textX), (int)std::round(textY), m_style.labelColor, m_fontSize);
    }
}

RadioButtonGroup::RadioButtonGroup(int* selectedValue, std::function<void(int)> onSelectionChange)
    : m_selectedValue(selectedValue), m_onSelectionChangeCallback(std::move(onSelectionChange))
{
   
    if (!m_selectedValue) {
        static int dummy = -1;
        m_selectedValue = &dummy;
    }
}
void RadioButtonGroup::addButton(const std::string& label, int value, XenUI::PositionParams posParams, RadioButtonStyle style, int fontSize) {
    
    m_buttons.emplace_back(std::make_unique<RadioButton>(*this, label, value, posParams, style, fontSize));
   
    SDL_Point winSize = XenUI::GetWindowSize();
    int parentWidth = winSize.x > 0 ? winSize.x : 800;
    int parentHeight = winSize.y > 0 ? winSize.y : 600;
    recalculateLayout(parentWidth, parentHeight);
}


void RadioButtonGroup::recalculateLayout(int parentWidth, int parentHeight) {
    bool first = true;
    float minX=0,minY=0,maxX=0,maxY=0;
   
    for (auto& bptr : m_buttons) {
        bptr->recalculateLayout(parentWidth, parentHeight);
        SDL_FRect bb = bptr->getBounds();
        if (first) {
           
            minX = bb.x; minY = bb.y; maxX = bb.x + bb.w; maxY = bb.y + bb.h; first = false;
        } else {
           
            minX = std::min(minX, bb.x);
            minY = std::min(minY, bb.y);
            maxX = std::max(maxX, bb.x + bb.w);
            maxY = std::max(maxY, bb.y + bb.h);
        }
    }
    // Finalize group bounds
    if (!first) {
        m_groupBounds.x = minX; m_groupBounds.y = minY;
        m_groupBounds.w = maxX - minX; m_groupBounds.h = maxY - minY;
    } else {
        // No buttons in the group
        m_groupBounds = {0,0,0,0};
    }
}

bool RadioButtonGroup::handleEvent(const SDL_Event& e) {
    
    for (auto& bptr : m_buttons) {
        if (bptr->handleEvent(e)) return true;
    }
    return false;
}

void RadioButtonGroup::draw(SDL_Renderer* renderer, const SDL_FPoint& viewOffset) {
    for (auto& bptr : m_buttons) {
        bptr->draw(renderer, viewOffset);
    }
}

SDL_FRect RadioButtonGroup::getBounds() const {
    return m_groupBounds;
}

void RadioButtonGroup::notifySelection(int value) {
    if (m_selectedValue && *m_selectedValue != value) {
       
        *m_selectedValue = value;
        
        if (m_onSelectionChangeCallback) m_onSelectionChangeCallback(value);
    }
}

bool RadioButtonGroup::isSelected(int value) const {
    return (m_selectedValue && *m_selectedValue == value);
}

// ---------------- Retained Mode Implementation Ends Here ----------------






// ---------------- Immediate Mode Implementation Starts Here ----------------

namespace XenUI {
bool RadioGroupImmediate(
    const char* id,
    const std::vector<std::string>& options,
    int* selectedIndex,
    const XenUI::PositionParams& pos,
    const RadioButtonStyle& style,
    int fontSize,
    int spacing,

    const SDL_FPoint& viewOffset,
    int parentWidth,
    int parentHeight,
    const SDL_Event* event
) {
    if (!selectedIndex || !TextRenderer::getInstance().isInitialized()) return false;
    bool changed = false;

   
    int pW = parentWidth, pH = parentHeight;
    if (pW <= 0 || pH <= 0) {
        SDL_Point ws = XenUI::GetWindowSize();
        pW = ws.x; pH = ws.y;
    }

  
    int maxTextW = 0;
    int maxTextH = 0;
    for (const auto &opt : options) {
        int tw = 0, th = 0;
        TextRenderer::getInstance().measureText(opt, fontSize, tw, th);
        maxTextW = std::max(maxTextW, tw);
        maxTextH = std::max(maxTextH, th);
    }

    int circleDiameter = style.circleRadius * 2;
    int itemHeight = std::max(circleDiameter, maxTextH);
   
    int effectiveSpacing = std::max(spacing, itemHeight + style.circlePadding);

    int groupWidth = circleDiameter + style.circlePadding + maxTextW;
    int groupHeight = (int)options.size() * effectiveSpacing;

   
    SDL_Point groupStart = XenUI::CalculateFinalPosition(pos, groupWidth, groupHeight, pW, pH);
    float baseX = (float)groupStart.x;
    float baseY = (float)groupStart.y;

  
    float evtX = 0.0f, evtY = 0.0f;
    bool haveExplicitEvent = false;
    if (event) {
        if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN || event->type == SDL_EVENT_MOUSE_BUTTON_UP ||
            event->type == SDL_EVENT_MOUSE_MOTION) {
            haveExplicitEvent = true;
         
            if (event->type == SDL_EVENT_MOUSE_MOTION) {
                evtX = (float)event->motion.x;
                evtY = (float)event->motion.y;
            } else {
                evtX = (float)event->button.x;
                evtY = (float)event->button.y;
            }
        }
    }
   
    if (!haveExplicitEvent) {
        SDL_GetMouseState(&evtX, &evtY);
    }

    
    float contentX = evtX - viewOffset.x;
    float contentY = evtY - viewOffset.y;

    
    SDL_Renderer* renderer = TextRenderer::getInstance().getRenderer();
    if (!renderer) return false;

    for (size_t i = 0; i < options.size(); ++i) {
      
        float itemX = baseX;
        float itemY = baseY + i * effectiveSpacing;

      
        int textW = 0, textH = 0;
        TextRenderer::getInstance().measureText(options[i], fontSize, textW, textH);

        SDL_FRect bounds = {
            itemX,
            itemY,
            (float)(circleDiameter + style.circlePadding + textW),
            (float)std::max(circleDiameter, textH)
        };

        SDL_FPoint circleCenter = { bounds.x + (float)style.circleRadius, bounds.y + bounds.h * 0.5f };

       
        bool inside = (contentX >= bounds.x && contentX <= (bounds.x + bounds.w)
                       && contentY >= bounds.y && contentY <= (bounds.y + bounds.h));

       
        if (event && (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN) && event->button.button == SDL_BUTTON_LEFT && inside) {
         
            if (*selectedIndex != (int)i) { *selectedIndex = (int)i; changed = true; }
        } else if (!event) {
            
            Uint32 mState = SDL_GetMouseState(nullptr, nullptr);
            if ((mState & SDL_BUTTON_MASK(SDL_BUTTON_LEFT)) && inside) {
                if (*selectedIndex != (int)i) { *selectedIndex = (int)i; changed = true; }
            }
        }

       
        const float drawCx = circleCenter.x + viewOffset.x;
        const float drawCy = circleCenter.y + viewOffset.y;

        // Draw outer circle
        SDL_SetRenderDrawColor(renderer, style.circleColor.r, style.circleColor.g, style.circleColor.b, style.circleColor.a);
        DrawCircle(renderer, int(std::round(drawCx)), int(std::round(drawCy)), style.circleRadius);

        // Draw selected fill
        if (*selectedIndex == (int)i) {
            SDL_SetRenderDrawColor(renderer, style.selectedColor.r, style.selectedColor.g, style.selectedColor.b, style.selectedColor.a);
            DrawFilledCircle(renderer, int(std::round(drawCx)), int(std::round(drawCy)), style.circleRadius - style.innerCirclePadding);
        }

       
        float textX = circleCenter.x + style.circleRadius + style.circlePadding;
        float textY = circleCenter.y - (textH / 2.0f);
        TextRenderer::getInstance().renderText(options[i], int(std::round(textX + viewOffset.x)), int(std::round(textY + viewOffset.y)), style.labelColor, fontSize);
    }

    return changed;
}

} // namespace XenUI

// ---------------- Immediate Mode Implementation Ends Here ----------------
