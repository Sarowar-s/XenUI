// SPDX-License-Identifier: Apache-2.0
/*
 *  Copyright (C) 2025 MD S M Sarowar Hossain
 *
 * 
 * 
 * 
 * 
 */
// Implements both the Retained Mode (class Label) and Immediate Mode (function Label)
// functionalities for displaying static text.
//

#include "Label.h"
#include "SDL3/SDL_log.h"

//
// --- Retained Mode Implementation (class Label) starts here ---
//

/**
 * @brief Constructs a retained-mode Label object.
 *
 * Measures the initial text size and calculates the label's final position
 * within the layout based on PositionParams.
 *
 * @param text The initial text string to display.
 * @param posParams The positional constraints (anchors, offsets) for layout calculation.
 * @param fontSize The size of the font in points.
 * @param color The color of the text.
 */
Label::Label(const std::string& text,
             const XenUI::PositionParams& posParams,
             int fontSize,
             const SDL_Color& color)
: m_text(text),
  m_posParams(posParams),
  m_fontSize(fontSize > 0 ? fontSize : 12),
  m_color(color),
  m_textRenderer(TextRenderer::getInstance()), // Get reference to the singleton TextRenderer
  m_width(0), m_height(0), m_x(0), m_y(0)
{
    if (!m_textRenderer.isInitialized()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Label Constructor: TextRenderer not initialized!");
        return;
    }

    // Measure the dimensions of the text to establish the control's size
    SDL_Point size = m_textRenderer.getTextSize(m_text, m_fontSize);
    m_width = size.x;
    m_height = size.y;

    // Calculate the content-space position based on the dimensions and position parameters
    SDL_Point pos = XenUI::CalculateFinalPosition(m_posParams, m_width, m_height);
    m_x = pos.x;
    m_y = pos.y;

    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Label created: '%s' at (%d,%d) with size (%d,%d)",
                 m_text.c_str(), m_x, m_y, m_width, m_height);
}

/**
 * @brief Sets new text content for the label.
 *
 * If the text changes, the label's required dimensions are re-measured, and
 * the content-space layout is recalculated.
 *
 * @param text The new text string.
 */
void Label::setText(const std::string& text) {
    if (m_text == text) return;
    m_text = text;

    if (!m_textRenderer.isInitialized()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Label::setText: TextRenderer not initialized!");
        m_width = 0;
        m_height = 0;
    } else {
        // Re-measure text size
        SDL_Point size = m_textRenderer.getTextSize(m_text, m_fontSize);
        m_width = size.x;
        m_height = size.y;
    }

    // Recompute content-space layout (position) based on new dimensions
    recalculateLayout();
}

/**
 * @brief Manually sets the content-space top-left position of the label.
 *
 * This bypasses recalculation based on PositionParams.
 *
 * @param x The new X coordinate in content-space.
 * @param y The new Y coordinate in content-space.
 */
void Label::setPosition(int x, int y) {
    m_x = x;
    m_y = y;
}

/**
 * @brief Sets the text color of the label.
 *
 * @param color The new SDL_Color for the text.
 */
void Label::setColor(const SDL_Color& color) {
    m_color = color;
}

/**
 * @brief Draws the label text using a zero view offset (for backwards compatibility).
 *
 * This convenience overload is suitable when the label is not nested in a scrolling container.
 *
 * @param renderer The SDL_Renderer context.
 */
void Label::draw(SDL_Renderer* renderer) {
    SDL_FPoint zero = {0.0f, 0.0f};
    draw(renderer, zero);
}

/**
 * @brief Draws the label text, offset by the parent's view offset. (IControl override).
 *
 * The final on-screen position is calculated by combining the content-space position (m_x, m_y)
 * with the translation offset.
 *
 * @param renderer The SDL_Renderer context.
 * @param viewOffset The content-to-screen translation offset (e.g., scroll position).
 */
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

/**
 * @brief Handles incoming SDL events.
 *
 * Labels are non-interactive controls; this function currently does nothing but satisfy the IControl interface.
 *
 * @param e The SDL_Event to process.
 * @return Always returns false, indicating the event was not consumed or handled.
 */
bool Label::handleEvent(const SDL_Event& /*e*/) {
    // Label is non-interactive by default.
    return false;
}

/**
 * @brief Recalculates the label's dimensions and content-space position based on parent layout. (IControl override).
 *
 * This is called when the parent container size changes or when the text content is updated.
 *
 * @param parentWidth The width of the parent container or content area.
 * @param parentHeight The height of the parent container or content area.
 */
void Label::recalculateLayout(int parentWidth, int parentHeight) {
    // 1. Re-measure size based on current text
    if (!m_textRenderer.isInitialized()) {
        m_width = m_height = 0;
    } else {
        SDL_Point size = m_textRenderer.getTextSize(m_text, m_fontSize );
        m_width = size.x;
        m_height = size.y;
    }

    // 2. Calculate content-space position using PositionParams
    SDL_Point pos = XenUI::CalculateFinalPosition(m_posParams, m_width, m_height, parentWidth, parentHeight);
    m_x = pos.x;
    m_y = pos.y;
}

/**
 * @brief Retrieves the bounding box of the label in content-space. (IControl override).
 *
 * @return The SDL_FRect defining the label's top-left (m_x, m_y) and size (m_width, m_height).
 */
SDL_FRect Label::getBounds() const {
    return SDL_FRect{ static_cast<float>(m_x), static_cast<float>(m_y),
                      static_cast<float>(m_width), static_cast<float>(m_height) };
}

/**
 * @brief Sets the SDL_Window context. (IControl override).
 *
 * @param window The current SDL_Window.
 */
void Label::setWindow(SDL_Window* window) {
    m_window = window;
    // Window context is stored primarily for consistency with IControl; not strictly needed for rendering static text.
}

/**
 * @brief Sets the parent's view offset. (IControl override).
 *
 * @param viewOffset The content-to-screen translation offset.
 */
void Label::setViewOffset(const SDL_FPoint& viewOffset) {
    m_viewOffset = viewOffset;
    // Offset is stored primarily for consistency with IControl; only used in draw() when rendering.
}


//
// --- Retained Mode Implementation (class Label) ends here ---
//


//
// --- Immediate Mode Implementation (namespace XenUI functions) starts here ---
//

namespace XenUI {

/**
 * @brief Renders a label immediately at the calculated position.
 *
 * This function handles text measurement, position calculation relative to parent/content,
 * and rendering, all within a single function call.
 *
 * @param text The text string to render.
 * @param posParams The positional constraints (anchors, offsets) for layout calculation.
 * @param fontSize The size of the font in points.
 * @param color The color of the text.
 * @param parentWidth The width of the content area (used for position calculation).
 * @param parentHeight The height of the content area (used for position calculation).
 * @param viewOffset The content-to-screen translation offset (e.g., scroll position).
 */
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
    // Note: Immediate mode often relies on pre-rendering/caching the texture explicitly.
    // The following call fetches/creates the texture and measures its size (w, h).
    SDL_Texture* tex = textRenderer.getInstance().renderTextToTexture(text, color, fontSize, w, h);
    if (!tex) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "XenUI::Label (Immediate): Failed to create texture for '%s'", text.c_str());
        return;
    }

    // Normalize parent dimensions (use window size if parent dims are invalid)
    int pW = parentWidth;
    int pH = parentHeight;
    if (pW <= 0 || pH <= 0) {
        SDL_Point win = XenUI::GetWindowSize();
        pW = win.x;
        pH = win.y;
    }

    // 1. Compute content-space position (pos.x, pos.y) based on posParams, size (w, h), and parent dimensions.
    SDL_Point pos = XenUI::CalculateFinalPosition(posParams, w, h, pW, pH);

    // 2. Calculate final on-screen position (screen-space)
    float finalX = (float)pos.x + viewOffset.x;
    float finalY = (float)pos.y + viewOffset.y;

    // 3. Render the text using the calculated screen coordinates
    textRenderer.getInstance().renderText(text, (int)finalX, (int)finalY, color, fontSize);
}

/**
 * @brief Convenience overload for immediate mode Label, assuming no parent/window size context and zero view offset.
 *
 * This overload is suitable for simple, non-nested use cases.
 *
 * @param text The text string to render.
 * @param posParams The positional constraints for layout calculation.
 * @param fontSize The size of the font in points.
 * @param color The color of the text.
 */
void Label(const std::string& text,
           const XenUI::PositionParams& posParams,
           int fontSize,
           const SDL_Color& color)
{
    SDL_FPoint zero = {0.0f, 0.0f};
    // Calls the main Label function with fallback dimensions (-1) and zero offset.
    Label(text, posParams, fontSize,  color, -1, -1, zero);
}

} // namespace XenUI

//
// --- Immediate Mode Implementation (namespace XenUI functions) ends here ---