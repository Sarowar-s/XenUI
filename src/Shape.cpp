//
// SPDX-License-Identifier: Apache-2.0
/*
 *  Copyright (C) 2025 MD S M Sarowar Hossain
 */
//
// Implements retained-mode primitive shapes: Rectangle and Circle.
// These shapes adhere to the IControl interface for layout and drawing within the framework.
//

#include "Shape.h"
#include "WindowUtil.h" 
#include <cmath>       

namespace XenUI {

// ---------------- Retained Mode Rectangle Implementation Starts Here ----------------


Rectangle::Rectangle(XenUI::PositionParams posParams, int width, int height, SDL_Color color)
    : m_posParams(posParams),
      m_reqWidth(width), m_reqHeight(height),
      m_color(color),
      m_dynamicWidth(width == -1), m_dynamicHeight(height == -1)
{
    
    recalculateLayout();
}


bool Rectangle::handleEvent(const SDL_Event& /*e*/) {
   
    return false;
}


void Rectangle::recalculateLayout(int parentWidth, int parentHeight) {
    
    m_width  = m_dynamicWidth  ? parentWidth : m_reqWidth;
   
    m_height = m_dynamicHeight ? parentHeight : m_reqHeight;

   
    SDL_Point pos = CalculateFinalPosition(m_posParams, m_width, m_height, parentWidth, parentHeight);
    m_x = pos.x;
    m_y = pos.y;
}


SDL_FRect Rectangle::getBounds() const {
    return SDL_FRect{ (float)m_x, (float)m_y, (float)m_width, (float)m_height };
}


void Rectangle::draw(SDL_Renderer* renderer, const SDL_FPoint& viewOffset) {
    SDL_SetRenderDrawColor(renderer, m_color.r, m_color.g, m_color.b, m_color.a);
    
    SDL_FRect dst = { (float)m_x + viewOffset.x, (float)m_y + viewOffset.y, (float)m_width, (float)m_height };
    SDL_RenderFillRect(renderer, &dst);
}

// ---------------- Retained Mode Rectangle Implementation Ends Here ----------------




// ---------------- Retained Mode Circle Implementation Starts Here ----------------


Circle::Circle(XenUI::PositionParams posParams, int radius, SDL_Color color)
    : m_posParams(posParams), m_radius(radius), m_color(color)
{
   
    recalculateLayout();
}


bool Circle::handleEvent(const SDL_Event& /*e*/) {
    return false;
}


void Circle::recalculateLayout(int parentWidth, int parentHeight) {
   
    int size = m_radius * 2;
   
    SDL_Point pos = CalculateFinalPosition(m_posParams, size, size, parentWidth, parentHeight);
   
    m_x = pos.x;
    m_y = pos.y;
}

SDL_FRect Circle::getBounds() const {
    return SDL_FRect{ (float)m_x, (float)m_y, (float)(m_radius*2), (float)(m_radius*2) };
}

void Circle::draw(SDL_Renderer* renderer, const SDL_FPoint& viewOffset) {
    if (!renderer) return;

    int cx = (int)std::round((float)m_x + m_radius + viewOffset.x);
    int cy = (int)std::round((float)m_y + m_radius + viewOffset.y);

    if (m_radius <= 0) return;

    SDL_SetRenderDrawColor(renderer, m_color.r, m_color.g, m_color.b, m_color.a);

    for (int dy = -m_radius; dy <= m_radius; ++dy) {
        int yy = cy + dy; 
       
        
        int dx = (int)std::floor(std::sqrt((double)(m_radius * m_radius - dy * dy)));   
        int x1 = cx - dx;
        int x2 = cx + dx; 
        SDL_RenderLine(renderer, x1, yy, x2, yy);
    }
}

// ---------------- Retained Mode Circle Implementation Ends Here ----------------

} // namespace XenUI
