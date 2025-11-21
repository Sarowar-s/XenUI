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

/**
 * @brief Checks if a float point (px, py) is inside an SDL_FRect (r).
 * @param px The X coordinate of the point.
 * @param py The Y coordinate of the point.
 * @param r The SDL_FRect bounding box.
 * @return true if the point is within the rectangle (exclusive right/bottom edge), false otherwise.
 */
static inline bool pointInRectF(float px, float py, const SDL_FRect& r) {
    return (px >= r.x && px < (r.x + r.w) && py >= r.y && py < (r.y + r.h));
}

/**
 * @brief Draws a circle outline using SDL_RenderPoints.
 * @param renderer The SDL_Renderer context.
 * @param centreX The X coordinate of the circle center.
 * @param centreY The Y coordinate of the circle center.
 * @param radius The radius of the circle.
 */
static void DrawCircle(SDL_Renderer* renderer, int32_t centreX, int32_t centreY, int32_t radius) {
    if (!renderer || radius <= 0) return;
    const int32_t diameter = radius * 2;
    int32_t x = radius - 1;
    int32_t y = 0;
    int32_t tx = 1;
    int32_t ty = 1;
    int32_t err = tx - diameter;
    // Reserve space for potential points (max 8 points per quadrant iteration)
    std::vector<SDL_FPoint> pts; pts.reserve(radius * 8);
    // Bresenham's circle algorithm to find points for the outline
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

/**
 * @brief Draws a filled circle by rendering horizontal lines.
 * @param renderer The SDL_Renderer context.
 * @param centerX The X coordinate of the circle center.
 * @param centerY The Y coordinate of the circle center.
 * @param radius The radius of the circle.
 */
static void DrawFilledCircle(SDL_Renderer* renderer, int centerX, int centerY, int radius) {
    if (!renderer || radius <= 0) return;
    // Iterate from bottom to top (or vice versa) of the circle's bounding box
    for (int y = -radius; y <= radius; ++y) {
        // Calculate the maximum horizontal displacement (dx) for the current y-level using Pythagoras
        int dx = (int)std::floor(std::sqrt((float)(radius*radius - y*y)));
        // Draw a horizontal line segment
        SDL_RenderLine(renderer, centerX - dx, centerY + y, centerX + dx, centerY + y);
    }
}

// ---------------- Retained Mode Implementation Starts Here ----------------

// ---------------- RadioButton ----------------
/**
 * @brief Constructs a retained-mode RadioButton.
 *
 * This control belongs to a RadioButtonGroup and manages its own state, label, and drawing.
 *
 * @param group The RadioButtonGroup instance this button belongs to (reference).
 * @param label The text displayed next to the radio button circle.
 * @param value The integer value associated with this specific button's selection.
 * @param posParams The positional constraints for layout calculation.
 * @param style The visual style parameters.
 * @param fontSize The font size for the label text.
 */
RadioButton::RadioButton(RadioButtonGroup& group, std::string label, int value,
                         XenUI::PositionParams posParams, RadioButtonStyle style, int fontSize)
    : m_group(group),
      m_label(std::move(label)),
      m_value(value),
      m_posParams(posParams),
      m_style(style),
      m_fontSize(fontSize > 0 ? fontSize : DEFAULT_RADIO_FONT_SIZE),
      m_isHovered(false) // Initial state: not hovered
{
    // Initial layout calculation using current window size as parent
    SDL_Point winSize = XenUI::GetWindowSize();
    int parentWidth = winSize.x > 0 ? winSize.x : 800; // Fallback width
    int parentHeight = winSize.y > 0 ? winSize.y : 600; // Fallback height
    recalculateLayout(parentWidth, parentHeight);
}


/**
 * @brief Recalculates the button's size and content-space position.
 *
 * Measures the label text, determines the total size (circle + padding + text),
 * and resolves the final position using PositionParams.
 *
 * @param parentWidth The width of the parent container or content area.
 * @param parentHeight The height of the parent container or content area.
 */
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

    // Resolve the top-left position (x, y) based on PositionParams and the total size
    SDL_Point pos = XenUI::CalculateFinalPosition(m_posParams, totalW, totalH, parentWidth, parentHeight);

    // Store the final content-space bounds
    m_bounds.x = static_cast<float>(pos.x);
    m_bounds.y = static_cast<float>(pos.y);
    m_bounds.w = static_cast<float>(totalW);
    m_bounds.h = static_cast<float>(totalH);

    // Calculate the center point of the circular radio element within the bounds
    m_circleCenter.x = m_bounds.x + m_style.circleRadius;
    m_circleCenter.y = m_bounds.y + m_bounds.h / 2.0f;
}


/**
 * @brief Handles mouse input events (motion and button down).
 *
 * Updates hover state and notifies the group of selection change on left-click if inside bounds.
 *
 * @param e The SDL_Event to process (expected to have content-space coordinates).
 * @return true if the button's state changed (hover/selection), false otherwise.
 */
bool RadioButton::handleEvent(const SDL_Event& e) {
    bool changed = false;

    float mx = 0.0f, my = 0.0f;
    // Extract mouse coordinates from the event
    if (e.type == SDL_EVENT_MOUSE_MOTION) {
        mx = (float)e.motion.x; my = (float)e.motion.y;
    } else if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN || e.type == SDL_EVENT_MOUSE_BUTTON_UP) {
        mx = (float)e.button.x; my = (float)e.button.y;
    } else {
        return false;
    }

    // Hit test: check if the mouse is over the button's total bounds
    bool inside = pointInRectF(mx, my, m_bounds);
    if (inside != m_isHovered) {
        m_isHovered = inside;
        changed = true; // State change due to hover
    }

    // Handle left mouse button click
    if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN && e.button.button == SDL_BUTTON_LEFT) {
        if (inside) {
            // If clicked inside, notify the group about the new selection
            m_group.notifySelection(m_value);
            changed = true; // State change due to selection
        }
    }
    return changed;
}

/**
 * @brief Draws the radio button (circle, selected state, and label).
 *
 * Converts content-space coordinates (m_circleCenter) to screen-space using viewOffset.
 *
 * @param renderer The SDL_Renderer context.
 * @param viewOffset The content-to-screen translation offset.
 */
void RadioButton::draw(SDL_Renderer* renderer, const SDL_FPoint& viewOffset) {
    if (!renderer) return;
    bool selected = m_group.isSelected(m_value);

    // Calculate screen-space center point for drawing the circle
    float drawCenterX = m_circleCenter.x + viewOffset.x;
    float drawCenterY = m_circleCenter.y + viewOffset.y;

    // Draw outer circle outline
    SDL_SetRenderDrawColor(renderer, m_style.circleColor.r, m_style.circleColor.g, m_style.circleColor.b, m_style.circleColor.a);
    DrawCircle(renderer, (int)std::round(drawCenterX), (int)std::round(drawCenterY), m_style.circleRadius);

    // If selected, draw the inner filled circle
    if (selected) {
        SDL_SetRenderDrawColor(renderer, m_style.selectedColor.r, m_style.selectedColor.g, m_style.selectedColor.b, m_style.selectedColor.a);
        DrawFilledCircle(renderer, (int)std::round(drawCenterX), (int)std::round(drawCenterY), m_style.circleRadius - m_style.innerCirclePadding);
    }

    // Draw label text
    if (TextRenderer::getInstance().isInitialized() && !m_label.empty()) {
        // Calculate text position: right of the circle, vertically centered
        float textX = drawCenterX + m_style.circleRadius + m_style.circlePadding;
        float textY = drawCenterY - (m_textHeight / 2.0f);
       TextRenderer::getInstance().renderText(m_label, (int)std::round(textX), (int)std::round(textY), m_style.labelColor, m_fontSize);
    }
}

// ---------------- RadioButtonGroup ----------------
/**
 * @brief Constructs a retained-mode RadioButtonGroup.
 *
 * Manages a collection of RadioButton controls, tracks the currently selected value,
 * and provides a callback for selection changes.
 *
 * @param selectedValue Pointer to an integer that holds the currently selected value.
 * @param onSelectionChange Callback function invoked when the selection changes.
 */
RadioButtonGroup::RadioButtonGroup(int* selectedValue, std::function<void(int)> onSelectionChange)
    : m_selectedValue(selectedValue), m_onSelectionChangeCallback(std::move(onSelectionChange))
{
    // If no external pointer is supplied, use an internal static dummy variable to prevent null dereference
    if (!m_selectedValue) {
        static int dummy = -1;
        m_selectedValue = &dummy;
    }
}

/**
 * @brief Adds a new RadioButton to the group.
 *
 * Creates a new RadioButton instance and triggers a recalculation of the group's layout bounds.
 *
 * @param label The text for the new button.
 * @param value The value associated with the new button.
 * @param posParams Positional constraints for the new button.
 * @param style Visual style for the new button.
 * @param fontSize Font size for the new button's label.
 */
void RadioButtonGroup::addButton(const std::string& label, int value, XenUI::PositionParams posParams, RadioButtonStyle style, int fontSize) {
    // Emplace a new RadioButton unique_ptr into the vector
    m_buttons.emplace_back(std::make_unique<RadioButton>(*this, label, value, posParams, style, fontSize));
    // Recalculate layout for all buttons and the group bounds
    SDL_Point winSize = XenUI::GetWindowSize();
    int parentWidth = winSize.x > 0 ? winSize.x : 800;
    int parentHeight = winSize.y > 0 ? winSize.y : 600;
    recalculateLayout(parentWidth, parentHeight);
}

/**
 * @brief Recalculates the layout for all child buttons and updates the overall group bounds.
 *
 * Iterates through all contained RadioButtons, recalculates their individual layouts,
 * and determines the minimum bounding box that encompasses all buttons.
 *
 * @param parentWidth The width of the parent container or content area.
 * @param parentHeight The height of the parent container or content area.
 */
void RadioButtonGroup::recalculateLayout(int parentWidth, int parentHeight) {
    bool first = true;
    float minX=0,minY=0,maxX=0,maxY=0;
    // Iterate over all buttons to recalculate their layout and compute the combined bounds
    for (auto& bptr : m_buttons) {
        bptr->recalculateLayout(parentWidth, parentHeight);
        SDL_FRect bb = bptr->getBounds();
        if (first) {
            // Initialize bounds with the first button's bounds
            minX = bb.x; minY = bb.y; maxX = bb.x + bb.w; maxY = bb.y + bb.h; first = false;
        } else {
            // Expand bounds to include subsequent buttons
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

/**
 * @brief Forwards the event to all child RadioButtons.
 *
 * Stops forwarding and returns true as soon as one child consumes the event.
 *
 * @param e The SDL_Event to process.
 * @return true if any child button handled the event, false otherwise.
 */
bool RadioButtonGroup::handleEvent(const SDL_Event& e) {
    // Forward to children (iteration order depends on desired Z-order/hit testing priority)
    for (auto& bptr : m_buttons) {
        if (bptr->handleEvent(e)) return true;
    }
    return false;
}

/**
 * @brief Draws all RadioButtons in the group.
 *
 * @param renderer The SDL_Renderer context.
 * @param viewOffset The content-to-screen translation offset.
 */
void RadioButtonGroup::draw(SDL_Renderer* renderer, const SDL_FPoint& viewOffset) {
    for (auto& bptr : m_buttons) {
        bptr->draw(renderer, viewOffset);
    }
}

/**
 * @brief Gets the minimum bounding box encompassing all buttons in the group.
 *
 * @return The SDL_FRect representing the group's combined bounds in content-space.
 */
SDL_FRect RadioButtonGroup::getBounds() const {
    return m_groupBounds;
}

/**
 * @brief Updates the selected value and triggers the selection change callback.
 *
 * Called internally by a RadioButton when it is clicked.
 *
 * @param value The value of the newly selected button.
 */
void RadioButtonGroup::notifySelection(int value) {
    if (m_selectedValue && *m_selectedValue != value) {
        // Update the value pointed to by m_selectedValue
        *m_selectedValue = value;
        // Invoke the external callback if registered
        if (m_onSelectionChangeCallback) m_onSelectionChangeCallback(value);
    }
}

/**
 * @brief Checks if a specific value corresponds to the currently selected radio button.
 *
 * @param value The value to check.
 * @return true if the value is currently selected, false otherwise.
 */
bool RadioButtonGroup::isSelected(int value) const {
    return (m_selectedValue && *m_selectedValue == value);
}

// ---------------- Retained Mode Implementation Ends Here ----------------


// ---------------- Immediate Mode Implementation Starts Here ----------------

namespace XenUI {

/**
 * @brief Renders an immediate-mode radio button group and handles input.
 *
 * This function handles layout calculation, hit-testing, input processing, and drawing
 * for a vertical list of radio options within a single call.
 *
 * @param id A unique identifier string (not strictly used here, but good practice).
 * @param options A vector of strings for the button labels.
 * @param selectedIndex Pointer to an integer holding the current 0-based index selection.
 * @param pos The PositionParams for the entire group.
 * @param style The visual style parameters.
 * @param fontSize Font size for the labels.
 * @param spacing Minimum vertical spacing between the items.
 * @param viewOffset The content-to-screen translation offset.
 * @param parentWidth The width of the content area.
 * @param parentHeight The height of the content area.
 * @param event Optional pointer to the current SDL_Event for event-based input processing.
 * @return true if the selected index was changed by user interaction, false otherwise.
 */
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

    // --- 1) Determine effective parent dimensions (fallback to window size) ---
    int pW = parentWidth, pH = parentHeight;
    if (pW <= 0 || pH <= 0) {
        SDL_Point ws = XenUI::GetWindowSize();
        pW = ws.x; pH = ws.y;
    }

    // --- 2) Measure options to compute required group size ---
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
    // Ensure spacing is sufficient for vertical arrangement
    int effectiveSpacing = std::max(spacing, itemHeight + style.circlePadding);

    int groupWidth = circleDiameter + style.circlePadding + maxTextW;
    int groupHeight = (int)options.size() * effectiveSpacing;

    // --- 3) Resolve group's top-left in content-space (anchor-aware) ---
    SDL_Point groupStart = XenUI::CalculateFinalPosition(pos, groupWidth, groupHeight, pW, pH);
    float baseX = (float)groupStart.x;
    float baseY = (float)groupStart.y;

    // --- 4) Determine mouse coordinates for hit testing ---
    float evtX = 0.0f, evtY = 0.0f;
    bool haveExplicitEvent = false;
    if (event) {
        if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN || event->type == SDL_EVENT_MOUSE_BUTTON_UP ||
            event->type == SDL_EVENT_MOUSE_MOTION) {
            haveExplicitEvent = true;
            // Get coordinates based on event type
            if (event->type == SDL_EVENT_MOUSE_MOTION) {
                evtX = (float)event->motion.x;
                evtY = (float)event->motion.y;
            } else {
                evtX = (float)event->button.x;
                evtY = (float)event->button.y;
            }
        }
    }
    // If no explicit event coordinates, fall back to polling the current mouse state
    if (!haveExplicitEvent) {
        SDL_GetMouseState(&evtX, &evtY);
    }

    // Translate screen-space mouse coords to content-space
    float contentX = evtX - viewOffset.x;
    float contentY = evtY - viewOffset.y;

    // --- 5) Iterate items: hit-test & draw ---
    SDL_Renderer* renderer = TextRenderer::getInstance().getRenderer();
    if (!renderer) return false;

    for (size_t i = 0; i < options.size(); ++i) {
        // Compute item top-left in content space
        float itemX = baseX;
        float itemY = baseY + i * effectiveSpacing;

        // Compute bounds and circle center for the current item
        int textW = 0, textH = 0;
        TextRenderer::getInstance().measureText(options[i], fontSize, textW, textH);

        SDL_FRect bounds = {
            itemX,
            itemY,
            (float)(circleDiameter + style.circlePadding + textW),
            (float)std::max(circleDiameter, textH)
        };

        SDL_FPoint circleCenter = { bounds.x + (float)style.circleRadius, bounds.y + bounds.h * 0.5f };

        // Hit test using content-space coordinates
        bool inside = (contentX >= bounds.x && contentX <= (bounds.x + bounds.w)
                       && contentY >= bounds.y && contentY <= (bounds.y + bounds.h));

        // Input handling
        if (event && (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN) && event->button.button == SDL_BUTTON_LEFT && inside) {
            // Event-based click detected
            if (*selectedIndex != (int)i) { *selectedIndex = (int)i; changed = true; }
        } else if (!event) {
            // Fallback: If no event is provided, poll for mouse button down at current position
            Uint32 mState = SDL_GetMouseState(nullptr, nullptr);
            if ((mState & SDL_BUTTON_MASK(SDL_BUTTON_LEFT)) && inside) {
                if (*selectedIndex != (int)i) { *selectedIndex = (int)i; changed = true; }
            }
        }

        // --- Drawing (Screen-space: add viewOffset) ---
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

        // Draw label (vertically centered)
        float textX = circleCenter.x + style.circleRadius + style.circlePadding;
        float textY = circleCenter.y - (textH / 2.0f);
        TextRenderer::getInstance().renderText(options[i], int(std::round(textX + viewOffset.x)), int(std::round(textY + viewOffset.y)), style.labelColor, fontSize);
    }

    return changed;
}

} // namespace XenUI

// ---------------- Immediate Mode Implementation Ends Here ----------------