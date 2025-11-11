//
// SPDX-License-Identifier: Apache-2.0
/*
 *  Copyright (C) 2025 MD S M Sarowar Hossain
 */
//
#ifndef LABEL_H
#define LABEL_H

// --- Standard Library and SDL Includes ---
#include <string>
#include <SDL3/SDL.h>
#include "TextRenderer.h" // Provides text rendering and measurement utilities
#include "Anchor.h"       // Layout anchors (top, bottom, left, right, etc.)
#include "Position.h"     // PositionParams and utility for calculating final position
#include "UIElement.h"    // Base class for retained-mode controls (IControl)

/**
 * @brief A retained-mode UI control for displaying static, non-interactive text.
 *
 * Inherits from IControl, allowing it to participate in layout and rendering
 * within a UI hierarchy (e.g., inside a ScrollView).
 */
class Label : public IControl {
public:
    /**
     * @brief Constructs a retained-mode Label object.
     *
     * Measures text size to determine its bounds and calculates its position based on
     * the provided layout parameters.
     *
     * @param text The initial text string to display.
     * @param posParams The positional constraints (anchors, offsets) for layout calculation.
     * @param fontSize The size of the font in points.
     * @param color The color of the text (defaults to opaque white).
     */
    Label(const std::string& text,
          const XenUI::PositionParams& posParams,
          int fontSize,
          const SDL_Color& color = {255, 255, 255, 255}); // Opaque white

    /**
     * @brief Default destructor.
     */
    ~Label() override = default;

    // --- Backwards-compatible API (Retained Mode) ---

    /**
     * @brief Updates the text content, forcing a re-measurement and layout recalculation.
     * @param newText The new text string.
     */
    void setText(const std::string& newText);

    /**
     * @brief Manually overrides the calculated content-space position.
     *
     * @param x The new X coordinate in content-space.
     * @param y The new Y coordinate in content-space.
     */
    void setPosition(int x, int y);

    /**
     * @brief Sets the color of the text.
     * @param color The new SDL_Color for the text.
     */
    void setColor(const SDL_Color& color);

    /**
     * @brief Draws the label text without applying a view offset (assumes {0,0} offset).
     * @param renderer The SDL_Renderer context.
     */
    void draw(SDL_Renderer* renderer);

    // --- IControl Interface Overrides (Retained Mode) ---

    /**
     * @brief Renders the label text, offset by the parent's view offset.
     *
     * @param renderer The SDL_Renderer context.
     * @param viewOffset The content-to-screen translation offset (e.g., scroll position).
     */
    void draw(SDL_Renderer* renderer, const SDL_FPoint& viewOffset) override;

    /**
     * @brief Handles incoming SDL events. Labels are non-interactive.
     *
     * @param e The SDL_Event to process.
     * @return Always returns false as the label does not consume events.
     */
    bool handleEvent(const SDL_Event& e) override;

    /**
     * @brief Recalculates the label's dimensions and content-space position based on parent layout.
     *
     * Measures the text and uses PositionParams to determine m_x and m_y relative to the parent area.
     *
     * @param parentWidth The width of the content area (used for position calculation).
     * @param parentHeight The height of the content area (used for position calculation).
     */
    void recalculateLayout(int parentWidth, int parentHeight) override;

    /**
     * @brief Backwards-compatible wrapper that recalculates layout using the current window size as parent dimensions.
     */
    void recalculateLayout() {
        SDL_Point s = XenUI::GetWindowSize();
        recalculateLayout(s.x, s.y);
    }

    /**
     * @brief Returns the bounding box of the label in content-space.
     * @return The SDL_FRect defining the label's position and size.
     */
    SDL_FRect getBounds() const override;

    /**
     * @brief Sets the SDL_Window context (optional for a non-interactive control).
     * @param window The current SDL_Window.
     */
    void setWindow(SDL_Window* window) override;

    /**
     * @brief Sets the parent's view offset (optional, primarily used in draw()).
     * @param viewOffset The content-to-screen translation offset.
     */
    void setViewOffset(const SDL_FPoint& viewOffset) override;

    // --- Legacy Getters ---
    int getX() const { return m_x; }             ///< Retrieves the calculated content-space X position.
    int getY() const { return m_y; }             ///< Retrieves the calculated content-space Y position.
    int getWidth() const { return m_width; }     ///< Retrieves the calculated text width.
    int getHeight() const { return m_height; }   ///< Retrieves the calculated text height.
    const std::string& getText() const { return m_text; } ///< Retrieves the current text string.

private:
    std::string m_text;                         ///< The text content.
    XenUI::PositionParams m_posParams;          ///< Positional configuration.
    int m_x = 0;                                ///< Calculated content-space X position.
    int m_y = 0;                                ///< Calculated content-space Y position.
    int m_width = 0;                            ///< Calculated text width.
    int m_height = 0;                           ///< Calculated text height.
    SDL_Color m_color;                          ///< Text rendering color.
    TextRenderer& m_textRenderer;               ///< Reference to the singleton renderer.
    int m_fontSize = 12;                        ///< Font size in points.

    // Optional cached context, maintained for consistency with IControl
    SDL_Window* m_window = nullptr;             ///< Cached SDL_Window pointer.
    SDL_FPoint m_viewOffset{0.0f, 0.0f};        ///< Cached parent view offset.
};


// -----------------------------------------------------------------------------
// === Immediate Mode Label API (within XenUI namespace) starts here ===
// -----------------------------------------------------------------------------

namespace XenUI {

    /**
     * @brief Renders a label immediately without creating a persistent Label object.
     *
     * This function calculates the final screen position by resolving `posParams` relative
     * to the provided parent dimensions and applying the `viewOffset`.
     * It uses TextRenderer's internal texture caching for efficiency.
     *
     * @param text The text string to render.
     * @param posParams The positional constraints for layout calculation.
     * @param fontSize The size of the font in points.
     * @param color The color of the text (defaults to opaque white).
     * @param parentWidth The width of the content area (or -1 to use window size).
     * @param parentHeight The height of the content area (or -1 to use window size).
     * @param viewOffset The content-to-screen translation offset (e.g., scroll position, defaults to {0,0}).
     */
void Label(const std::string& text,
           const XenUI::PositionParams& posParams,
           int fontSize,
           const SDL_Color& color = {255,255,255,255},
            int parentWidth = -1,
           int parentHeight = -1,
           const SDL_FPoint& viewOffset = {0.0f, 0.0f});

} // namespace XenUI

// -----------------------------------------------------------------------------
// === Immediate Mode Label API (within XenUI namespace) ends here ===
// -----------------------------------------------------------------------------

#endif // LABEL_H