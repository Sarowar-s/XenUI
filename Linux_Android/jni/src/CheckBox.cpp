// SPDX-License-Identifier: Apache-2.0
/*
 *  Copyright (C) 2025 MD S M Sarowar Hossain
 *
 */
// Implementation of the Checkbox control, supporting both retained and immediate modes.
//
#include "CheckBox.h"
#include <SDL3_ttf/SDL_ttf.h>
#include <algorithm>
#include <map>

// Ensure the global textRenderer instance is accessible
//extern TextRenderer& textRenderer;


/**
 * @brief Constructs a Checkbox object (Retained Mode).
 *
 * Initializes the checkbox with its label, positioning parameters, initial state,
 * styling, font size, and a toggle callback function.
 *
 * @param label The text displayed next to the checkbox.
 * @param posParams Parameters determining the checkbox's position relative to its parent/container.
 * @param initialState The initial checked state (true for checked, false for unchecked).
 * @param style Visual style parameters for the checkbox (colors, sizes, etc.).
 * @param fontSize Font size for the label. If 0 or negative, uses DEFAULT_CHECKBOX_FONT_SIZE.
 * @param onToggle A function to call when the checkbox state is toggled.
 */
Checkbox::Checkbox(const std::string& label,
                   const XenUI::PositionParams& posParams,
                   bool initialState,
                   CheckboxStyle style,
                   int fontSize,
                   std::function<void(bool)> onToggle)
    : m_label(label),
      m_posParams(posParams),
      m_style(style),
      m_fontSize(fontSize > 0 ? fontSize : DEFAULT_CHECKBOX_FONT_SIZE),
      m_onToggleCallback(std::move(onToggle)),
      m_isChecked(initialState),
      m_isHovered(false),
      m_isPressed(false),
      m_textWidth(0.0f),
      m_textHeight(0.0f)
{
    // Initial layout calculation based on current parameters (assuming parent is full screen/container)
    recalculateLayout();
}

/**
 * @brief Draws the checkmark symbol inside the checkbox bounding box.
 *
 * This function calculates the coordinates for a 'V' shaped checkmark and draws it
 * using SDL_RenderLine, taking into account the defined checkmark thickness.
 *
 * @param renderer The SDL_Renderer context to draw upon.
 * @param boxRect The absolute screen coordinates and dimensions of the checkbox box.
 */
void Checkbox::drawCheckmark(SDL_Renderer* renderer, const SDL_FRect& boxRect) const {
    if (!renderer) return;

    // Set the checkmark drawing color from the style definition
    SDL_SetRenderDrawColor(renderer, m_style.checkmarkColor.r,
                           m_style.checkmarkColor.g, m_style.checkmarkColor.b,
                           m_style.checkmarkColor.a);

    // Calculate three points for the 'V' shape checkmark (relative to boxRect)
    float x1 = boxRect.x + boxRect.w * 0.2f;
    float y1 = boxRect.y + boxRect.h * 0.5f;
    float x2 = boxRect.x + boxRect.w * 0.45f;
    float y2 = boxRect.y + boxRect.h * 0.75f;
    float x3 = boxRect.x + boxRect.w * 0.8f;
    float y3 = boxRect.y + boxRect.h * 0.25f;

    // Draw lines with thickness by offsetting the line coordinates
    for (int i = 0; i < m_style.checkmarkThickness; ++i) {
        // Calculate the offset required to simulate thickness, centering the lines
        float offset = (float)i - (m_style.checkmarkThickness - 1) / 2.0f;
        // First segment of the 'V'
        SDL_RenderLine(renderer, x1 + offset, y1, x2 + offset, y2);
        // Second segment of the 'V'
        SDL_RenderLine(renderer, x2 + offset, y2, x3 + offset, y3);
    }
}

/**
 * @brief Renders the checkbox (Retained Mode Draw Function).
 *
 * Draws the checkbox box (background and border), the checkmark (if checked),
 * and the label text. All local coordinates are translated by the viewOffset
 * to get absolute screen coordinates.
 *
 * @param renderer The SDL_Renderer context.
 * @param viewOffset The coordinate space offset (e.g., from a scroll view) to apply.
 */
void Checkbox::draw(SDL_Renderer* renderer, const SDL_FPoint& viewOffset) {
    if (!renderer) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Checkbox::draw: Renderer is null.");
        return;
    }

    // Compose absolute (screen) rects by adding viewOffset to local box rect
    SDL_FRect absBoxRect = m_boxRect;
    absBoxRect.x += viewOffset.x;
    absBoxRect.y += viewOffset.y;

    // Background / box
    SDL_Color currentBgColor = m_style.boxBgColor;
    // Determine background color based on interaction state
    if (m_isPressed) currentBgColor = m_style.boxPressedColor;
    else if (m_isHovered) currentBgColor = m_style.boxHoverColor;

    // Draw box fill
    SDL_SetRenderDrawColor(renderer,
                           currentBgColor.r, currentBgColor.g, currentBgColor.b, currentBgColor.a);
    SDL_RenderFillRect(renderer, &absBoxRect);

    // Border
    SDL_SetRenderDrawColor(renderer,
                           m_style.boxBorderColor.r, m_style.boxBorderColor.g, m_style.boxBorderColor.b, m_style.boxBorderColor.a);
    SDL_RenderRect(renderer, &absBoxRect);

    // Checkmark
    if (m_isChecked) {
        drawCheckmark(renderer, absBoxRect);
    }

    // Label: compute absolute label position
    SDL_FPoint absLabelPos = m_labelPos;
    absLabelPos.x += viewOffset.x;
    absLabelPos.y += viewOffset.y;

    // Render the label text if TextRenderer is initialized
    if (!m_label.empty() && TextRenderer::getInstance().isInitialized()) {
        TextRenderer::getInstance().renderText(m_label, (int)absLabelPos.x, (int)absLabelPos.y,
                                m_style.labelColor, m_fontSize);
    } else if (!m_label.empty()) {
        // Log a warning if the label cannot be drawn
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Checkbox::draw: TextRenderer not initialized, cannot draw label '%s'.", m_label.c_str());
    }
}

/**
 * @brief Processes SDL input events for the checkbox (Retained Mode Event Handler).
 *
 * Updates the internal state (hovered, pressed, checked) based on mouse events.
 * It is assumed that event coordinates are already translated into the local
 * coordinate space of the checkbox (e.g., by a parent container like a ScrollView).
 *
 * @param e The SDL_Event to handle.
 * @return true if the internal state of the checkbox changed (e.g., hover, press, check state), false otherwise.
 */
bool Checkbox::handleEvent(const SDL_Event& e) {
    bool changed = false;

    // We'll extract mouse coordinates from the incoming event (if available).
    // These coordinates are assumed to be relative to the control's coordinate space (m_bounds).
    float mx = 0.0f, my = 0.0f;
    bool havePos = false;

    // Extract mouse position from relevant event types
    if (e.type == SDL_EVENT_MOUSE_MOTION) {
        mx = (float)e.motion.x;
        my = (float)e.motion.y;
        havePos = true;
    } else if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN || e.type == SDL_EVENT_MOUSE_BUTTON_UP) {
        mx = (float)e.button.x;
        my = (float)e.button.y;
        havePos = true;
    } else if (e.type == SDL_EVENT_MOUSE_WHEEL) {
        // wheel events are typically handled by ScrollView; no position extraction needed here
    }

    // Update hover state if we have a position sample
    if (havePos) {
        SDL_FPoint mp{ mx, my };
        bool wasHovered = m_isHovered;
        // Check if the mouse position is inside the checkbox bounds
        m_isHovered = SDL_PointInRectFloat(&mp, &m_bounds);
        if (wasHovered != m_isHovered) changed = true;
    }

    // Handle presses/releases (based on event coordinates that are assumed to be in our coordinate space)
    if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN && e.button.button == SDL_BUTTON_LEFT) {
        // Only start press if the event's coordinates are inside bounds
        SDL_FPoint mp{ mx, my };
        if (SDL_PointInRectFloat(&mp, &m_bounds)) {
            m_isPressed = true;
            changed = true;
        }
    } else if (e.type == SDL_EVENT_MOUSE_BUTTON_UP && e.button.button == SDL_BUTTON_LEFT) {
        if (m_isPressed) {
            m_isPressed = false;
            SDL_FPoint mp{ mx, my };
            if (SDL_PointInRectFloat(&mp, &m_bounds)) {
                // Released inside bounds: toggle the check state and call the callback
                m_isChecked = !m_isChecked;
                if (m_onToggleCallback) m_onToggleCallback(m_isChecked);
                changed = true;
            } else {
                // Released outside: just a visual state change (no longer pressed)
                changed = true;
            }
        }
    } else if (e.type == SDL_EVENT_MOUSE_WHEEL) {
        // Wheel events are ignored by the Checkbox itself; handled by containers
    }

    return changed;
}


/**
 * @brief Recalculates the internal layout and positioning of the checkbox elements.
 *
 * This function is responsible for measuring the label text, calculating the total
 * widget dimensions, resolving the final position within the parent/content area,
 * and determining the local rectangles for the check box and the label text position.
 *
 * @param parentWidth The width of the parent container/content area (default 0).
 * @param parentHeight The height of the parent container/content area (default 0).
 */
void Checkbox::recalculateLayout(int parentWidth, int parentHeight) {
    // 1) Measure text width/height
    if (TextRenderer::getInstance().isInitialized()) {
        TextRenderer::getInstance().measureText(m_label, m_fontSize, (int&)m_textWidth, (int&)m_textHeight);
    } else {
        // Fallback dimensions if text renderer is not ready
        m_textWidth  = 0;
        m_textHeight = (float)m_style.boxSize;
    }

    // 2) Get font metrics for baseline alignment
    TTF_Font* font = TextRenderer::getInstance().getFont(m_fontSize);
    int ascent  = font ? TTF_GetFontAscent(font)  : int(m_textHeight);
    int descent = font ? TTF_GetFontDescent(font) : 0;
    float glyphH = float(ascent + descent);

    // 3) Compute total widget size: (box + padding + text) width, (max(box, textH) + padding) height
    float totalWidth  = m_style.boxSize + m_style.paddingX + m_textWidth;
    float totalHeight = std::max(glyphH, float(m_style.boxSize)) + 2.0f * m_style.paddingY;

    // 4) Resolve widget position (finalPos is in parent/content coordinate space)
    SDL_Point finalPos = XenUI::CalculateFinalPosition(m_posParams, int(totalWidth), int(totalHeight), parentWidth, parentHeight);
   // SDL_Log("Checkbox::recalculateLayout label='%s' parent=(%d,%d) total=(%.1f,%.1f) final=(%d,%d)",
        // m_label.c_str(), parentWidth, parentHeight, totalWidth, totalHeight, finalPos.x, finalPos.y);

    // The bounding box in local (parent/content) coordinates
    m_bounds = { float(finalPos.x), float(finalPos.y), totalWidth, totalHeight };

    // 5) Center box vertically inside bounds
    m_boxRect = {
        m_bounds.x + m_style.paddingX,
        m_bounds.y + (m_bounds.h - m_style.boxSize) * 0.5f, // Vertical center calculation
        float(m_style.boxSize),
        float(m_style.boxSize)
    };

    // 6) Compute baseline so label is vertically aligned nicely with the box
    float rowCenterY = m_bounds.y + m_bounds.h * 0.5f;
    // The baseline is calculated from the center of the total height, using font ascent/descent
    float baselineY  = rowCenterY + (glyphH * 0.5f - float(ascent));

    // Label position starts after the box, plus padding
    m_labelPos = {
        m_boxRect.x + m_boxRect.w + m_style.paddingX,
        baselineY
    };
}


//-----------------------------------------------------------------------------
//--------------------- Immediate-mode Checkbox Implementation ----------------
//-----------------------------------------------------------------------------
namespace XenUI {
    /**
     * @brief Renders and handles an immediate-mode Checkbox control.
     *
     * This function draws a checkbox and processes mouse input in a single call,
     * directly affecting the state pointed to by 'isChecked'. State is transiently
     * tracked using a static map based on the unique 'id'.
     *
     * @param id A unique string identifier for the checkbox instance.
     * @param label The text displayed next to the checkbox.
     * @param isChecked Pointer to the boolean variable holding the check state.
     * @param posParams Parameters for positioning the control relative to its parent/content.
     * @param style Visual style parameters.
     * @param fontSize Font size for the label.
     * @param viewOffset The coordinate space offset (e.g., scroll position) to apply.
     * @param parentWidth The width of the parent container/content area.
     * @param parentHeight The height of the parent container/content area.
     * @return true if the check state (*isChecked) was changed by this call, false otherwise.
     */
    bool Checkbox(const char* id,
                  const std::string& label,
                  bool* isChecked,
                  const PositionParams& posParams,
                  const CheckboxStyle& style,
                  int fontSize,

                  const SDL_FPoint& viewOffset,
                int parentWidth,
                  int parentHeight) // <-- accepts parent dims now
    {
        SDL_Renderer* renderer = TextRenderer::getInstance().getRenderer();
        if (!renderer || !isChecked || !TextRenderer::getInstance().isInitialized()) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "XenUI::Checkbox: Invalid renderer, isChecked pointer, or TextRenderer not initialized.");
            return false;
        }

        bool stateChanged = false;

        // 1. Measure text dimensions (in glyph space)
        float textW = 0.0f, textH = 0.0f;
        TextRenderer::getInstance().measureText(label, fontSize, (int&)textW, (int&)textH);

        // 2. Compute total sizes
        float totalWidth = (float)style.boxSize + style.paddingX + textW;
        float totalHeight = std::max((float)style.boxSize, textH) + 2 * style.paddingY;

        // 3. Normalize parent dims (fallback to window size when not provided)
        int pW = parentWidth;
        int pH = parentHeight;
        if (pW <= 0 || pH <= 0) {
            SDL_Point win = XenUI::GetWindowSize();
            pW = win.x;
            pH = win.y;
        }

        // 4. Resolve final local position (relative to parent/content origin)
        SDL_Point localPos = CalculateFinalPosition(posParams, (int)totalWidth, (int)totalHeight, pW, pH);

        // 5. Convert to absolute screen coordinates by adding viewOffset for the main bounds
        SDL_FRect bounds = {
            (float)localPos.x + viewOffset.x,
            (float)localPos.y + viewOffset.y,
            totalWidth,
            totalHeight
        };

        // 6. Box rect (absolute coordinates)
        SDL_FRect boxRect = {
            bounds.x + style.paddingX,
            bounds.y + (bounds.h - style.boxSize) / 2.0f, // Vertically centered
            (float)style.boxSize,
            (float)style.boxSize
        };

        // 7. Baseline alignment for label (absolute coordinates)
        TTF_Font* font = TextRenderer::getInstance().getFont(fontSize);
        int ascent  = font ? TTF_GetFontAscent(font)  : int(textH);
        int descent = font ? TTF_GetFontDescent(font) : 0;
        float glyphH = float(ascent + descent);
        float rowCY     = bounds.y + bounds.h * 0.5f;
        float baselineY = rowCY + (glyphH * 0.5f - float(ascent));

        SDL_FPoint labelPos = {
            boxRect.x + boxRect.w + style.paddingX,
            baselineY
        };

        // 8. Input: absolute mouse pos (SDL_GetMouseState returns absolute coords)
        SDL_FPoint mousePos;
        // Get current mouse state to check for hover/press immediately
        Uint32 mouseState = SDL_GetMouseState(&mousePos.x, &mousePos.y);
        bool isHovered = SDL_PointInRectFloat(&mousePos, &bounds);
        // Pressed state: mouse left button down AND mouse is over the control
        bool isPressed = (mouseState & SDL_BUTTON_MASK(SDL_BUTTON_LEFT)) && isHovered;

        // 9. Click detection (simple immediate-mode released pattern)
        // Static map stores the pressed state from the previous frame for each control ID
        static std::unordered_map<std::string, bool> prevPressed;
        bool wasPressedLast = prevPressed[std::string(id)];

        // Click detected on Left-Button-Up (release) inside bounds
        if (isHovered && !isPressed && wasPressedLast) {
            *isChecked = !(*isChecked); // Toggle state
            stateChanged = true;
        }
        // Update the previous state for the next frame
        prevPressed[std::string(id)] = isPressed;

        // 10. Drawing (using absolute positions already computed)
        SDL_Color currentBgColor = style.boxBgColor;
        if (isPressed) currentBgColor = style.boxPressedColor;
        else if (isHovered) currentBgColor = style.boxHoverColor;

        // Box background
        SDL_SetRenderDrawColor(renderer, currentBgColor.r, currentBgColor.g, currentBgColor.b, currentBgColor.a);
        SDL_RenderFillRect(renderer, &boxRect);

        // Box border
        SDL_SetRenderDrawColor(renderer, style.boxBorderColor.r, style.boxBorderColor.g, style.boxBorderColor.b, style.boxBorderColor.a);
        SDL_RenderRect(renderer, &boxRect);

        // Checkmark
        if (*isChecked) {
            // Checkmark drawing logic, copied from Checkbox::drawCheckmark
            SDL_SetRenderDrawColor(renderer, style.checkmarkColor.r, style.checkmarkColor.g, style.checkmarkColor.b, style.checkmarkColor.a);
            for (int i = 0; i < style.checkmarkThickness; ++i) {
                float offset = (float)i - (style.checkmarkThickness - 1) / 2.0f;
                // Checkmark points (absolute coordinates)
                float x1 = boxRect.x + boxRect.w * 0.2f;
                float y1 = boxRect.y + boxRect.h * 0.5f;
                float x2 = boxRect.x + boxRect.w * 0.45f;
                float y2 = boxRect.y + boxRect.h * 0.75f;
                float x3 = boxRect.x + boxRect.w * 0.8f;
                float y3 = boxRect.y + boxRect.h * 0.25f;
                SDL_RenderLine(renderer, x1 + offset, y1, x2 + offset, y2);
                SDL_RenderLine(renderer, x2 + offset, y2, x3 + offset, y3);
            }
        }

        // Label text (absolute position)
        TextRenderer::getInstance().renderText(label, (int)labelPos.x, (int)labelPos.y, style.labelColor, fontSize);

        return stateChanged;
    }
} // namespace XenUI
//--------------------- Immediate-mode Checkbox Ends --------------------------
//-----------------------------------------------------------------------------