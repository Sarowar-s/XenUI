//
// SPDX-License-Identifier: Apache-2.0
/*
 *  Copyright (C) 2025 MD S M Sarowar Hossain
 */
//
// Implements both the retained-mode (class Switch) and immediate-mode (function SwitchImmediate)
// versions of a toggle switch control.
//

#include "Switch.h"
#include "WindowUtil.h"     
#include "TextRenderer.h"   
#include <SDL3/SDL.h>
#include <cmath>
#include <cstdio>
#include <utility>
#include <unordered_map>
#include <SDL3/SDL_render.h>
#include <iostream>


namespace XenUI
{

// ====================== Retained Mode Switch Implementation Starts Here ======================



Switch::Switch(const XenUI::PositionParams& posParams,
               const SwitchStyle& style,
               std::function<void(bool)> onToggle,
               bool initialState)
    : m_posParams(posParams),
      m_style(style),
      m_onToggle(std::move(onToggle)),
      m_isOn(initialState),
      m_hovered(false),
      m_isPressed(false),
      m_wasInside(false),
      m_posX(0.0f), m_posY(0.0f),
     
      m_width(m_style.trackWidth),
      m_height(m_style.trackHeight)
{
    
    recalculateLayout();
}


void Switch::setOn(bool on)
{
    m_isOn = on;
   
    if (m_onToggle) m_onToggle(m_isOn);
}

float Switch::getThumbCenterX_Content() const
{
    
    const float thumbRadius = m_style.trackHeight / 2.0f - m_style.thumbPadding;
   
    const float leftCenter  = m_posX + m_style.thumbPadding + thumbRadius;
    const float rightCenter = m_posX + m_width - m_style.thumbPadding - thumbRadius;
    return m_isOn ? rightCenter : leftCenter;
}


SDL_FRect Switch::getThumbRect_Content() const
{
    const float thumbRadius = m_style.trackHeight / 2.0f - m_style.thumbPadding;
    const float cx = getThumbCenterX_Content();
    const float cy = m_posY + m_height / 2.0f; 
   
    return SDL_FRect{ cx - thumbRadius, cy - thumbRadius, thumbRadius * 2.0f, thumbRadius * 2.0f };
}


void Switch::draw(SDL_Renderer* renderer, const SDL_FPoint& viewOffset)
{
    if (!renderer) return;

    SDL_Color trackCol = m_isOn ? m_style.trackColorOn : m_style.trackColorOff;
    SDL_Color thumbCol = m_isOn ? m_style.thumbColorOn : m_style.thumbColorOff;
    if (m_hovered) {
        trackCol = m_style.hoverTrackColor;
        thumbCol = m_style.hoverThumbColor;
    }

   
    SDL_FRect trackRect = { m_posX + viewOffset.x, m_posY + viewOffset.y, m_width, m_height };
    SDL_SetRenderDrawColor(renderer, trackCol.r, trackCol.g, trackCol.b, trackCol.a);
    SDL_RenderFillRect(renderer, &trackRect);

    
    SDL_FRect thumb = getThumbRect_Content();
    
    SDL_FRect thumbScreen = { thumb.x + viewOffset.x, thumb.y + viewOffset.y, thumb.w, thumb.h };
    SDL_SetRenderDrawColor(renderer, thumbCol.r, thumbCol.g, thumbCol.b, thumbCol.a);
    SDL_RenderFillRect(renderer, &thumbScreen);

    
    if (!TextRenderer::getInstance().isInitialized()) return;

    
    const std::string& label = m_isOn ? m_style.labelOn : m_style.labelOff;
    if (label.empty()) return;

    const int fontSize = m_style.labelFontSize;
    const SDL_Color labelColor = m_style.labelColor;

    int tw = 0, th = 0;
    TextRenderer::getInstance().measureText(label, fontSize, tw, th);

    
    const float thumbCenterX = getThumbCenterX_Content() + viewOffset.x;
    const float thumbCenterY = m_posY + m_height / 2.0f + viewOffset.y;
    const float tx = thumbCenterX - tw / 2.0f;
    const float ty = thumbCenterY - th / 2.0f;
    TextRenderer::getInstance().renderText(label, static_cast<int>(tx), static_cast<int>(ty), labelColor, fontSize);
}

bool Switch::handleEvent(const SDL_Event& event)
{
   
    float mx = 0.0f, my = 0.0f;
    if (event.type == SDL_EVENT_MOUSE_MOTION) {
        mx = static_cast<float>(event.motion.x); my = static_cast<float>(event.motion.y);
    } else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN || event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
        mx = static_cast<float>(event.button.x); my = static_cast<float>(event.button.y);
    } else {
        return false;
    }

   
    const bool inside = (mx >= m_posX && mx <= (m_posX + m_width) &&
                         my >= m_posY && my <= (m_posY + m_height));

    bool changed = false;

   
    if (inside != m_hovered) { m_hovered = inside; changed = true; }

   
    if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button == SDL_BUTTON_LEFT) {
        if (inside) {
            
            m_isPressed = true;
            m_wasInside = true;
            changed = true;
        }
    }
    
    else if (event.type == SDL_EVENT_MOUSE_BUTTON_UP && event.button.button == SDL_BUTTON_LEFT) {
       
        if (m_isPressed && m_wasInside && inside) {
            m_isOn = !m_isOn;
            if (m_onToggle) m_onToggle(m_isOn);
            changed = true;
        }
        
        m_isPressed = false;
        m_wasInside = false;
    }

    return changed;
}

void Switch::recalculateLayout(int parentWidth, int parentHeight)
{
   
    SDL_Point p = XenUI::CalculateFinalPosition(m_posParams,
                                                static_cast<int>(m_width),
                                                static_cast<int>(m_height), parentWidth, parentHeight);
    m_posX = static_cast<float>(p.x);
    m_posY = static_cast<float>(p.y);
}
SDL_FRect Switch::getBounds() const
{
    return SDL_FRect{ m_posX, m_posY, m_width, m_height };
}

// -----------------------------------------------------------------------------
// ====================== Retained Mode Switch Implementation Ends Here ======================
// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------
// ====================== Immediate Mode Switch Implementation Starts Here ======================
// -----------------------------------------------------------------------------

namespace Detail {
    
    std::unordered_map<std::string, ImmediateSwitchState> immediateSwitchStates;
}


bool SwitchImmediate(const std::string& id,
                     const XenUI::PositionParams& posParams,
                     const XenUI::SwitchStyle& style,
                     bool* pValue,
                     bool triggerOnPress,
                     int parentWidth ,
                     int parentHeight,
                     const SDL_FPoint& viewOffset,
                     const SDL_Event* /*event*/)
{
  
    if (!pValue) return false;

    
    auto it = Detail::immediateSwitchStates.find(id);
    if (it == Detail::immediateSwitchStates.end()) {
     
        Detail::immediateSwitchStates[id] = {};
    }
    Detail::ImmediateSwitchState& st = Detail::immediateSwitchStates[id];

  
    const float width  = style.trackWidth;
    const float height = style.trackHeight;
    SDL_Point fp;

   
    if (parentWidth > 0 && parentHeight > 0) {
        fp = XenUI::CalculateFinalPosition(posParams,
                                           static_cast<int>(width),
                                           static_cast<int>(height),
                                           parentWidth,
                                           parentHeight);
    } else {
        
        fp = XenUI::CalculateFinalPosition(posParams,
                                           static_cast<int>(width),
                                           static_cast<int>(height));
    }

    const float finalX = (float)fp.x; 
    const float finalY = (float)fp.y; 

    
    float mx, my;
    Uint32 mstate = SDL_GetMouseState(&mx, &my);
   
    const float mouseX = mx - viewOffset.x;
    const float mouseY = my - viewOffset.y;
    const bool  leftDown = (mstate & SDL_BUTTON_MASK(SDL_BUTTON_LEFT)) != 0;

    
    const bool inside = (mouseX >= finalX && mouseX <= (finalX + width) &&
                         mouseY >= finalY && mouseY <= (finalY + height));
    st.hovered = inside;

    bool toggledThisFrame = false;

    
    if (leftDown && !st.isPressed) {
      
        if (inside) {
            st.isPressed = true;
            st.wasInside = true;
            if (triggerOnPress) {
               
                *pValue = !*pValue;
                toggledThisFrame = true;
            }
        }
    } else if (!leftDown && st.isPressed) {
        
        if (st.wasInside && inside && !triggerOnPress) {
            
            *pValue = !*pValue;
            toggledThisFrame = true;
        }
        st.isPressed = false;
        st.wasInside = false;
    }

    
    SDL_Renderer* r = TextRenderer::getInstance().isInitialized() ? TextRenderer::getInstance().getRenderer() : nullptr;
    if (!r) return toggledThisFrame;

    
    SDL_Color trackCol = *pValue ? style.trackColorOn : style.trackColorOff;
    SDL_Color thumbCol = *pValue ? style.thumbColorOn : style.thumbColorOff;
    if (st.hovered) {
        trackCol = style.hoverTrackColor;
        thumbCol = style.hoverThumbColor;
    }

      SDL_FRect trackRect = { finalX + viewOffset.x, finalY + viewOffset.y, width, height };
    SDL_SetRenderDrawColor(r, trackCol.r, trackCol.g, trackCol.b, trackCol.a);
    SDL_RenderFillRect(r, &trackRect);

  
    const float thumbRadius = height / 2.0f - style.thumbPadding;
    const float leftCenter  = finalX + style.thumbPadding + thumbRadius;
    const float rightCenter = finalX + width - style.thumbPadding - thumbRadius;
   
    const float cx          = *pValue ? rightCenter : leftCenter;
    const float cy          = finalY + height / 2.0f;

    
    SDL_FRect thumbRect = {
        (cx - thumbRadius) + viewOffset.x, 
        (cy - thumbRadius) + viewOffset.y,
        thumbRadius * 2.0f,
        thumbRadius * 2.0f
    };
    SDL_SetRenderDrawColor(r, thumbCol.r, thumbCol.g, thumbCol.b, thumbCol.a);
    SDL_RenderFillRect(r, &thumbRect);

    
    if (TextRenderer::getInstance().isInitialized()) {
        const std::string& label = *pValue ? style.labelOn : style.labelOff;
        if (!label.empty()) {
            const int fontSize   = style.labelFontSize;
            const SDL_Color col  = style.labelColor;

            int tw = 0, th = 0;
            TextRenderer::getInstance().measureText(label, fontSize, tw, th);
            
            float tx = cx + viewOffset.x - tw / 2.0f;
            float ty = cy + viewOffset.y - th / 2.0f;
            TextRenderer::getInstance().renderText(label, (int)tx, (int)ty, col, fontSize);
        }
    }

    return toggledThisFrame;
}
} // namespace XenUI
// -----------------------------------------------------------------------------
// ====================== Immediate Mode Switch Implementation Ends Here ======================
// -----------------------------------------------------------------------------
