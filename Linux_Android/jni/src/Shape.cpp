// SPDX-License-Identifier: Apache-2.0
/*
 *  Copyright (C) 2025 MD S M Sarowar Hossain
 *
 */
//
// Implements retained-mode primitive shapes: Rectangle and Circle.
// These shapes adhere to the IControl interface for layout and drawing within the framework.
//

#include "Shape.h"
#include "WindowUtil.h" // Provides XenUI::GetWindowSize, XenUI::CalculateFinalPosition
#include <cmath>        // For std::sqrt, std::round, std::floor

namespace XenUI {

// ---------------- Retained Mode Rectangle Implementation Starts Here ----------------

/**
 * @brief Constructs a retained-mode rectangular shape control.
 *
 * @param posParams The positional constraints (anchors, offsets) of the rectangle's bounding box.
 * @param width The requested width in pixels. Use -1 for dynamic width (fills parent).
 * @param height The requested height in pixels. Use -1 for dynamic height (fills parent).
 * @param color The SDL_Color used to fill the rectangle.
 */
Rectangle::Rectangle(XenUI::PositionParams posParams, int width, int height, SDL_Color color)
    : m_posParams(posParams),
      m_reqWidth(width), m_reqHeight(height),
      m_color(color),
      m_dynamicWidth(width == -1), m_dynamicHeight(height == -1)
{
    // Perform initial layout calculation (assumes parent is the window for top-level instantiation)
    recalculateLayout();
}

/**
 * @brief Handles events for the Rectangle control.
 *
 * Primitive shapes are non-interactive by default, so this always returns false.
 *
 * @param e The SDL_Event to process.
 * @return false, as the event is not consumed.
 */
bool Rectangle::handleEvent(const SDL_Event& /*e*/) {
    // Non-interactive by default
    return false;
}

/**
 * @brief Recalculates the position and size of the Rectangle based on parent dimensions.
 *
 * Resolves dynamic size requests against the provided parent dimensions.
 *
 * @param parentWidth The available width in the parent container.
 * @param parentHeight The available height in the parent container.
 */
void Rectangle::recalculateLayout(int parentWidth, int parentHeight) {
    // Resolve width: use parent width if dynamic is set, otherwise use requested width
    m_width  = m_dynamicWidth  ? parentWidth : m_reqWidth;
    // Resolve height: use parent height if dynamic is set, otherwise use requested height
    m_height = m_dynamicHeight ? parentHeight : m_reqHeight;

    // Calculate the final top-left position based on PositionParams, resolved size, and parent size
    SDL_Point pos = CalculateFinalPosition(m_posParams, m_width, m_height, parentWidth, parentHeight);
    m_x = pos.x;
    m_y = pos.y;
}

/**
 * @brief Returns the absolute screen-space bounds of the rectangle.
 * @return An SDL_FRect defining the position and size.
 */
SDL_FRect Rectangle::getBounds() const {
    return SDL_FRect{ (float)m_x, (float)m_y, (float)m_width, (float)m_height };
}

/**
 * @brief Renders the filled rectangle to the renderer.
 * @param renderer The SDL_Renderer context.
 * @param viewOffset The screen-space offset inherited from parent containers (e.g., ScrollView).
 */
void Rectangle::draw(SDL_Renderer* renderer, const SDL_FPoint& viewOffset) {
    SDL_SetRenderDrawColor(renderer, m_color.r, m_color.g, m_color.b, m_color.a);
    // Apply view offset to the control's calculated position (m_x, m_y)
    SDL_FRect dst = { (float)m_x + viewOffset.x, (float)m_y + viewOffset.y, (float)m_width, (float)m_height };
    SDL_RenderFillRect(renderer, &dst);
}

// ---------------- Retained Mode Rectangle Implementation Ends Here ----------------
// ---------------- Retained Mode Circle Implementation Starts Here ----------------

/**
 * @brief Constructs a retained-mode circular shape control.
 *
 * @param posParams The positional constraints (anchors, offsets) of the circle's bounding box.
 * @param radius The radius of the circle in pixels.
 * @param color The SDL_Color used to fill the circle.
 */
Circle::Circle(XenUI::PositionParams posParams, int radius, SDL_Color color)
    : m_posParams(posParams), m_radius(radius), m_color(color)
{
    // Perform initial layout calculation
    recalculateLayout();
}

/**
 * @brief Handles events for the Circle control.
 *
 * Primitive shapes are non-interactive by default.
 *
 * @param e The SDL_Event to process.
 * @return false, as the event is not consumed.
 */
bool Circle::handleEvent(const SDL_Event& /*e*/) {
    return false;
}

/**
 * @brief Recalculates the position of the Circle's bounding box.
 *
 * The layout calculation treats the circle as a square bounding box of size (2*radius) x (2*radius).
 *
 * @param parentWidth The available width in the parent container.
 * @param parentHeight The available height in the parent container.
 */
void Circle::recalculateLayout(int parentWidth, int parentHeight) {
    // The control size is the bounding box (diameter)
    int size = m_radius * 2;
    // Calculate the top-left position of the bounding box
    SDL_Point pos = CalculateFinalPosition(m_posParams, size, size, parentWidth, parentHeight);
    // Store top-left of the bounding box
    m_x = pos.x;
    m_y = pos.y;
}

/**
 * @brief Returns the absolute screen-space bounds of the circle's square bounding box.
 * @return An SDL_FRect defining the position and size.
 */
SDL_FRect Circle::getBounds() const {
    // Bounding box has size (2*radius) x (2*radius)
    return SDL_FRect{ (float)m_x, (float)m_y, (float)(m_radius*2), (float)(m_radius*2) };
}

/**
 * @brief Renders the filled circle to the renderer.
 *
 * The circle is drawn using a horizontal span rendering algorithm (line segment per row).
 *
 * @param renderer The SDL_Renderer context.
 * @param viewOffset The screen-space offset inherited from parent containers.
 */
void Circle::draw(SDL_Renderer* renderer, const SDL_FPoint& viewOffset) {
    if (!renderer) return;

    // Calculate the resolved center coordinates in screen space
    // The center is at m_x + m_radius, m_y + m_radius, plus the view offset
    int cx = (int)std::round((float)m_x + m_radius + viewOffset.x);
    int cy = (int)std::round((float)m_y + m_radius + viewOffset.y);

    // Defensive check: no need to draw if radius is non-positive
    if (m_radius <= 0) return;

    SDL_SetRenderDrawColor(renderer, m_color.r, m_color.g, m_color.b, m_color.a);

    // Draw the filled circle using a horizontal line segment for each vertical row (dy)
    // dy iterates from -radius to +radius (from top edge to bottom edge of the circle)
    for (int dy = -m_radius; dy <= m_radius; ++dy) {
        int yy = cy + dy; // Current vertical row position (screen y)
        // Calculate half-width (dx) of the line segment for this row using Pythagorean theorem:
        // dx^2 + dy^2 = radius^2  =>  dx = sqrt(radius^2 - dy^2)
        int dx = (int)std::floor(std::sqrt((double)(m_radius * m_radius - dy * dy)));
        // Start X position: center X minus half-width
        int x1 = cx - dx;
        // End X position: center X plus half-width
        int x2 = cx + dx;
        // Draw the horizontal line segment
        SDL_RenderLine(renderer, x1, yy, x2, yy);
    }
}

// ---------------- Retained Mode Circle Implementation Ends Here ----------------

} // namespace XenUI