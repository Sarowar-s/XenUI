//
// SPDX-License-Identifier: Apache-2.0
/*
 *  Copyright (C) 2025 MD S M Sarowar Hossain
 */
//
// Implements both the retained-mode (class Slider) and immediate-mode (function Slider)
// versions of a user interface slider control.
//

#include "Slider.h"
#include "WindowUtil.h"
#include "TextRenderer.h"

using namespace XenUI;

// ---------------- Retained Mode Slider Implementation Starts Here ----------------


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
   
    m_currentValue = std::clamp(m_currentValue, m_minValue, m_maxValue);

    
    if (m_orientation == XenUI::Orientation::HORIZONTAL) {
        m_width = static_cast<int>(length);
       
        m_height = m_style.thumbSize + 2 * m_style.padding;
    } else {
        
        m_width = m_style.thumbSize + 2 * m_style.padding;
        m_height = static_cast<int>(length);
    }

  
    recalculateLayout();
}


SDL_FRect Slider::getThumbRectContent() const {
    float thumbW = static_cast<float>(m_style.thumbSize);
    float thumbH = static_cast<float>(m_style.thumbSize);
   
    float normalized = 0.0f;
    if (m_maxValue != m_minValue)
        normalized = (m_currentValue - m_minValue) / (m_maxValue - m_minValue);
    normalized = std::clamp(normalized, 0.0f, 1.0f);

    if (m_orientation == XenUI::Orientation::HORIZONTAL) {
        float trackLength = static_cast<float>(m_width - 2 * m_style.padding);
       
        float tx = static_cast<float>(m_posX) + m_style.padding + (trackLength * normalized) - (thumbW / 2.0f);
      
        float ty = static_cast<float>(m_posY) + (m_height - thumbH) / 2.0f;
        return SDL_FRect{tx, ty, thumbW, thumbH};
    } else {
        float trackLength = static_cast<float>(m_height - 2 * m_style.padding);
       
        // Calculate Y position: start of track + inverted normalized displacement - half thumb height
        float ty = static_cast<float>(m_posY) + m_style.padding + (trackLength * (1.0f - normalized)) - (thumbH / 2.0f);
        // Calculate X position: centered horizontally
        float tx = static_cast<float>(m_posX) + (m_width - thumbW) / 2.0f;
        return SDL_FRect{tx, ty, thumbW, thumbH};
    }
}


bool Slider::isPointInThumbContent(float x, float y) const {
    SDL_FRect r = getThumbRectContent();
    return (x >= r.x && x <= (r.x + r.w) && y >= r.y && y <= (r.y + r.h));
}


void Slider::updateValueFromMouseContent(float mouseX, float mouseY) {
    float newValue = m_currentValue;
    if (m_orientation == XenUI::Orientation::HORIZONTAL) {
        
        float trackStart = static_cast<float>(m_posX + m_style.padding);
        float trackEnd = static_cast<float>(m_posX + m_width - m_style.padding);
        float trackLen = trackEnd - trackStart;
       
        float posInTrack = mouseX - trackStart - m_dragOffset;
       
        newValue = m_minValue + (m_maxValue - m_minValue) * (posInTrack / trackLen);
    } else {
        float trackStart = static_cast<float>(m_posY + m_style.padding);
        float trackEnd = static_cast<float>(m_posY + m_height - m_style.padding);
        float trackLen = trackEnd - trackStart;
        float posInTrack = mouseY - trackStart - m_dragOffset;
        newValue = m_minValue + (m_maxValue - m_minValue) * (1.0f - (posInTrack / trackLen));
    }
   
    newValue = std::clamp(newValue, m_minValue, m_maxValue);
    if (newValue != m_currentValue) {
        m_currentValue = newValue;
        if (m_onValueChanged) m_onValueChanged(m_currentValue);
    }
}

void Slider::draw(SDL_Renderer* renderer, const SDL_FPoint& viewOffset) {
    
    if (!renderer || !TextRenderer::getInstance().isInitialized()) return;

   
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

    
    SDL_FRect thumb = getThumbRectContent();
    SDL_FRect thumbScreen = { thumb.x + viewOffset.x, thumb.y + viewOffset.y, thumb.w, thumb.h };

    
    SDL_Color curThumbColor = m_isHovered ? m_style.thumbHoverColor : m_style.thumbColor;
    SDL_SetRenderDrawColor(renderer, curThumbColor.r, curThumbColor.g, curThumbColor.b, curThumbColor.a);
    SDL_RenderFillRect(renderer, &thumbScreen);

   
    if (m_style.drawValueText) {
        std::string valueText = std::to_string(static_cast<int>(m_currentValue)); 
        int tw = 0, th = 0;
        TextRenderer::getInstance().measureText(valueText, m_style.valueTextFontSize, tw, th);
        int tx, ty;

       
        if (m_orientation == XenUI::Orientation::HORIZONTAL) {
         
            tx = static_cast<int>(thumbScreen.x + (thumbScreen.w - tw) / 2.0f); 
            ty = static_cast<int>(m_posY + m_height - th - 2 + viewOffset.y);
        } else {
          
            tx = static_cast<int>(m_posX + m_width + 2 + viewOffset.x);
            ty = static_cast<int>(thumbScreen.y + (thumbScreen.h - th) / 2.0f); 
        }
        TextRenderer::getInstance().getInstance().renderText(valueText, tx, ty, m_style.valueTextColor, m_style.valueTextFontSize);
    }
}


bool Slider::handleEvent(const SDL_Event& event) {
    bool changed = false;

   
    float mx=0.0f, my=0.0f;
    if (event.type == SDL_EVENT_MOUSE_MOTION) {
        mx = (float)event.motion.x; my = (float)event.motion.y;
    } else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN || event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
        mx = (float)event.button.x; my = (float)event.button.y;
    } else {
       
        return false;
    }

   
    bool wasHovered = m_isHovered;
    m_isHovered = isPointInThumbContent(mx, my);
    if (wasHovered != m_isHovered) changed = true;

   
    if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button == SDL_BUTTON_LEFT) {
        if (m_isHovered) {
           
            m_isDragging = true;
            SDL_FRect thumb = getThumbRectContent();
           
            if (m_orientation == XenUI::Orientation::HORIZONTAL)
                m_dragOffset = mx - (thumb.x + thumb.w / 2.0f);
            else
                m_dragOffset = my - (thumb.y + thumb.h / 2.0f);
            changed = true;
        } else {
           
            SDL_FRect bounds = { (float)m_posX, (float)m_posY, (float)m_width, (float)m_height };
            if (mx >= bounds.x && mx <= bounds.x + bounds.w && my >= bounds.y && my <= bounds.y + bounds.h) {
              
                m_isDragging = true;
                m_dragOffset = 0.0f; 
                updateValueFromMouseContent(mx, my);
                changed = true;
            }
        }
    }
   
    else if (event.type == SDL_EVENT_MOUSE_BUTTON_UP && event.button.button == SDL_BUTTON_LEFT) {
        if (m_isDragging) {
            m_isDragging = false;
            changed = true;
        }
    }
    
    else if (event.type == SDL_EVENT_MOUSE_MOTION) {
        if (m_isDragging) {
           
            updateValueFromMouseContent(mx, my);
            changed = true;
        }
    }

    return changed;
}

void Slider::recalculateLayout(int parentWidth, int parentHeight) {
   
    SDL_Point p = XenUI::CalculateFinalPosition(m_posParams, m_width, m_height, parentWidth, parentHeight);
    m_posX = p.x; m_posY = p.y;
}


SDL_FRect Slider::getBounds() const {
    return SDL_FRect{ (float)m_posX, (float)m_posY, (float)m_width, (float)m_height };
}


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
  
    SDL_Point ws = XenUI::GetWindowSize();
    return Slider(id, orientation, posParams, length, value, minValue, maxValue,
                  ws.x, ws.y, style, viewOffset, event);
}


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
    
    if (!value || !TextRenderer::getInstance().getInstance().isInitialized()) return false;

   
    Detail::SliderState& st = Detail::sliderStates[id];
    bool changed = false;

  
    if (st.currentValue != *value) st.currentValue = *value;

    
    int sliderW, sliderH;
    if (orientation == XenUI::Orientation::HORIZONTAL) {
        sliderW = static_cast<int>(length);
        sliderH = style.thumbSize + 2 * style.padding;
    } else {
        sliderW = style.thumbSize + 2 * style.padding;
        sliderH = static_cast<int>(length);
    }

    
    SDL_Point finalPos = CalculateFinalPosition(posParams, sliderW, sliderH, parentWidth, parentHeight);
    float finalX = static_cast<float>(finalPos.x); // content-space X
    float finalY = static_cast<float>(finalPos.y); // content-space Y

   
    float mx_win = 0, my_win = 0;
    Uint32 mstate = SDL_GetMouseState(&mx_win, &my_win);
    float mouseX = mx_win - viewOffset.x; // CONTENT-space X
    float mouseY = my_win - viewOffset.y; // CONTENT-space Y
    bool mouseLeftDown = (mstate & SDL_BUTTON_MASK(SDL_BUTTON_LEFT)) != 0;

    
    float thumbW = static_cast<float>(style.thumbSize);
    float thumbH = static_cast<float>(style.thumbSize);
    float normalized = 0.0f;
    if (maxValue != minValue) normalized = (st.currentValue - minValue) / (maxValue - minValue);
    normalized = std::clamp(normalized, 0.0f, 1.0f);

    float trackLen;
    SDL_FRect thumbRect;
    thumbRect.w = thumbW; thumbRect.h = thumbH;

    if (orientation == XenUI::Orientation::HORIZONTAL) {
        trackLen = static_cast<float>(sliderW - 2 * style.padding);
        thumbRect.x = finalX + style.padding + (trackLen * normalized) - (thumbW / 2.0f);
        thumbRect.y = finalY + (sliderH - thumbH) / 2.0f;
    } else {
        trackLen = static_cast<float>(sliderH - 2 * style.padding);
        
        thumbRect.y = finalY + style.padding + (trackLen * (1.0f - normalized)) - (thumbH / 2.0f);
        thumbRect.x = finalX + (sliderW - thumbW) / 2.0f;
    }

   

   
    st.isHovered = (mouseX >= thumbRect.x && mouseX <= (thumbRect.x + thumbRect.w) &&
                    mouseY >= thumbRect.y && mouseY <= (thumbRect.y + thumbRect.h));

   
    if (mouseLeftDown) {
        if (!st.isDragging && st.isHovered) {
           
            st.isDragging = true;
            
            if (orientation == XenUI::Orientation::HORIZONTAL)
                st.dragOffset = mouseX - (thumbRect.x + thumbRect.w / 2.0f);
            else
                st.dragOffset = mouseY - (thumbRect.y + thumbRect.h / 2.0f);
        } else if (!st.isDragging) {
            
            SDL_FRect sliderBounds = { finalX, finalY, static_cast<float>(sliderW), static_cast<float>(sliderH) };
            if (mouseX >= sliderBounds.x && mouseX <= (sliderBounds.x + sliderBounds.w) &&
                mouseY >= sliderBounds.y && mouseY <= (sliderBounds.y + sliderBounds.h)) {
                
                st.isDragging = true;
                st.dragOffset = 0.0f;
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
    
    else {
        if (st.isDragging) {
            st.isDragging = false;
            st.dragOffset = 0.0f;
        }
    }

   
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

    
    SDL_Renderer* renderer = TextRenderer::getInstance().getRenderer();
    if (!renderer) return changed; 

   
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

    
    normalized = std::clamp((st.currentValue - minValue) / (maxValue - minValue), 0.0f, 1.0f);
    if (orientation == XenUI::Orientation::HORIZONTAL) {
        thumbRect.x = finalX + style.padding + (trackLen * normalized) - (thumbW / 2.0f);
        thumbRect.y = finalY + (sliderH - thumbH) / 2.0f;
    } else {
        thumbRect.y = finalY + style.padding + (trackLen * (1.0f - normalized)) - (thumbH / 2.0f);
        thumbRect.x = finalX + (sliderW - thumbW) / 2.0f;
    }

    
    SDL_FRect thumbScreen = { thumbRect.x + viewOffset.x, thumbRect.y + viewOffset.y, thumbRect.w, thumbRect.h };
    SDL_Color curThumbColor = st.isHovered ? style.thumbHoverColor : style.thumbColor;
    SDL_SetRenderDrawColor(renderer, curThumbColor.r, curThumbColor.g, curThumbColor.b, curThumbColor.a);
    SDL_RenderFillRect(renderer, &thumbScreen);

    
    if (style.drawValueText) {
        std::string valueText = std::to_string(static_cast<int>(st.currentValue));
        int tw=0, th=0;
        TextRenderer::getInstance().measureText(valueText, style.valueTextFontSize, tw, th);
        int tx, ty;

       
        if (orientation == XenUI::Orientation::HORIZONTAL) {
           
            tx = (int)std::round(thumbScreen.x + (thumbScreen.w - tw) / 2.0f);
            ty = (int)std::round(finalY + sliderH - th - 2 + viewOffset.y);
        } else {
          
            tx = (int)std::round(finalX + sliderW + 2 + viewOffset.x);
            ty = (int)std::round(thumbScreen.y + (thumbScreen.h - th) / 2.0f);
        }
        TextRenderer::getInstance().getInstance().renderText(valueText, tx, ty, style.valueTextColor, style.valueTextFontSize);
    }

    return changed;
}

} // namespace XenUI
// ---------------- Immediate Mode Slider Implementation Ends Here ---------------
