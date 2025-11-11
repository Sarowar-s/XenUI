// SPDX-License-Identifier: Apache-2.0
/*
 *  Copyright (C) 2025 MD S M Sarowar Hossain
 *
 */
//
// Defines the base class for retained-mode graphical primitive shapes (Rectangle, Circle)
// and their implementations which derive from the IControl interface.
//

#pragma once
#include <SDL3/SDL.h>
#include "Anchor.h"      // For anchoring types
#include "Position.h"    // For XenUI::PositionParams
#include "UIElement.h"   // Defines IControl interface
#include "WindowUtil.h"  // For GetWindowSize(), CalculateFinalPosition (used in base wrappers)

namespace XenUI {

    /**
     * @brief Base class for retained-mode primitive shapes.
     *
     * Provides a common interface for layout calculation, drawing, and bounds retrieval,
     * deriving from IControl. Shapes are non-interactive by default.
     */
    class Shape : public IControl {
    public:
        // IControl provides virtual destructor already

        /**
         * @brief Pure virtual function to render the shape.
         *
         * @param renderer The SDL_Renderer context.
         * @param viewOffset The screen-space offset inherited from parent containers (e.g., ScrollView).
         */
        virtual void draw(SDL_Renderer* renderer, const SDL_FPoint& viewOffset) = 0;

        /**
         * @brief Backwards-compatibility wrapper for drawing without an explicit offset.
         *
         * Assumes zero offset, suitable for top-level or absolute drawing.
         *
         * @param renderer The SDL_Renderer context.
         */
        virtual void draw(SDL_Renderer* renderer) { SDL_FPoint zero{0,0}; draw(renderer, zero); }

        /**
         * @brief Pure virtual function to recalculate the shape's final position and size.
         *
         * This is the mandatory signature defined by the IControl interface.
         *
         * @param parentWidth The available width in the parent container.
         * @param parentHeight The available height in the parent container.
         */
        virtual void recalculateLayout(int parentWidth, int parentHeight) = 0;

        /**
         * @brief Backwards-compatible wrapper for layout calculation using current window size.
         *
         * This allows older code or simple top-level usage to call `recalculateLayout()`
         * without explicitly providing parent dimensions.
         */
        virtual void recalculateLayout() {
            SDL_Point s = GetWindowSize();
            recalculateLayout(s.x, s.y);
        }

        /**
         * @brief Pure virtual function to get the shape's content-space bounding box.
         *
         * @return The SDL_FRect defining the shape's bounds relative to its container's content area.
         */
        virtual SDL_FRect getBounds() const override = 0;
    };

    /**
     * @brief Retained-mode rectangular shape control.
     *
     * Supports fixed or dynamic (parent-filling) dimensions.
     */
    class Rectangle : public Shape {
    public:
        /**
         * @brief Constructs a Rectangle control.
         *
         * @param posParams The positional constraints for the rectangle.
         * @param width The requested width in pixels. Pass -1 for dynamic width (fills parent).
         * @param height The requested height in pixels. Pass -1 for dynamic height (fills parent).
         * @param color The SDL_Color used for filling the rectangle.
         */
        Rectangle(XenUI::PositionParams posParams, int width, int height, SDL_Color color);

        // --- IControl Overrides ---

        /**
         * @brief Event handler. Shapes are non-interactive by default.
         * @param e The SDL_Event to process.
         * @return Always returns false.
         */
        bool handleEvent(const SDL_Event& e) override;

        /**
         * @brief Renders the filled rectangle.
         *
         * @param renderer The SDL_Renderer context.
         * @param viewOffset The screen-space offset from the container.
         */
        void draw(SDL_Renderer* renderer, const SDL_FPoint& viewOffset) override;

        /**
         * @brief Recalculates the final position and resolved size of the rectangle.
         *
         * @param parentWidth The available width in the parent container.
         * @param parentHeight The available height in the parent container.
         */
        void recalculateLayout(int parentWidth, int parentHeight) override;

        /**
         * @brief Returns the rectangle's content-space bounding box.
         * @return The SDL_FRect defining the bounds.
         */
        SDL_FRect getBounds() const override;

        // --- Compatibility Wrappers ---
        /**
         * @brief Compatibility wrapper for layout using global window size.
         */
        void recalculateLayout() { Shape::recalculateLayout(); }
        /**
         * @brief Compatibility wrapper for drawing without explicit offset.
         */
        void draw(SDL_Renderer* renderer) { Shape::draw(renderer); }

    private:
        XenUI::PositionParams m_posParams;          ///< Positional configuration.
        int m_reqWidth, m_reqHeight;                ///< The size requested during construction (-1 if dynamic).
        int m_width, m_height;                      ///< The final resolved size.
        int m_x, m_y;                               ///< The content-space top-left coordinates.
        SDL_Color m_color;                          ///< The fill color.
        bool m_dynamicWidth, m_dynamicHeight;       ///< Flags indicating if size should fill the parent.
    };


    /**
     * @brief Retained-mode circular shape control.
     *
     * Position is determined by the bounding box of the circle.
     */
    class Circle : public Shape {
    public:
        /**
         * @brief Constructs a Circle control.
         *
         * @param posParams The positional constraints for the circle's bounding box.
         * @param radius The radius of the circle in pixels.
         * @param color The SDL_Color used for filling the circle.
         */
        Circle(XenUI::PositionParams posParams, int radius, SDL_Color color);

        // --- IControl Overrides ---

        /**
         * @brief Event handler. Shapes are non-interactive by default.
         * @param e The SDL_Event to process.
         * @return Always returns false.
         */
        bool handleEvent(const SDL_Event& e) override;

        /**
         * @brief Renders the filled circle.
         *
         * @param renderer The SDL_Renderer context.
         * @param viewOffset The screen-space offset from the container.
         */
        void draw(SDL_Renderer* renderer, const SDL_FPoint& viewOffset) override;

        /**
         * @brief Recalculates the final position of the circle's bounding box.
         *
         * @param parentWidth The available width in the parent container.
         * @param parentHeight The available height in the parent container.
         */
        void recalculateLayout(int parentWidth, int parentHeight) override;

        /**
         * @brief Returns the bounding box of the circle (a square of size 2*radius).
         * @return The SDL_FRect defining the bounds.
         */
        SDL_FRect getBounds() const override;

        // --- Compatibility Wrappers ---
        /**
         * @brief Compatibility wrapper for layout using global window size.
         */
        void recalculateLayout() { Shape::recalculateLayout(); }
        /**
         * @brief Compatibility wrapper for drawing without explicit offset.
         */
        void draw(SDL_Renderer* renderer) { Shape::draw(renderer); }

    private:
        XenUI::PositionParams m_posParams;  ///< Positional configuration of the bounding box.
        int m_radius;                       ///< The circle's radius in pixels.
        int m_x, m_y;                       ///< The content-space top-left coordinates of the bounding box.
        SDL_Color m_color;                  ///< The fill color.
    };

} // namespace XenUI