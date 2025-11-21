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
#include "WindowUtil.h"     // For XenUI::CalculateFinalPosition
#include "TextRenderer.h"   // For text drawing and rendering checks
#include <SDL3/SDL.h>
#include <cmath>
#include <cstdio>
#include <utility>
#include <unordered_map>
#include <SDL3/SDL_render.h>
#include <iostream>


namespace XenUI
{
// -----------------------------------------------------------------------------
// ====================== Retained Mode Switch Implementation Starts Here ======================
// -----------------------------------------------------------------------------

/**
 * @brief Constructs a retained-mode Switch control.
 *
 * @param posParams The positional constraints (anchors, offsets) of the switch's bounding box.
 * @param style The visual style parameters.
 * @param onToggle Optional callback function invoked when the switch state changes.
 * @param initialState The initial ON/OFF state of the switch.
 */
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
      // The switch's bounding box width/height are determined by the track dimensions in the style
      m_width(m_style.trackWidth),
      m_height(m_style.trackHeight)
{
    // Perform initial layout calculation
    recalculateLayout();
}

/**
 * @brief Programmatically sets the ON/OFF state of the switch.
 *
 * @param on The desired state (true for ON, false for OFF).
 */
void Switch::setOn(bool on)
{
    m_isOn = on;
    // Execute callback if provided
    if (m_onToggle) m_onToggle(m_isOn);
}

/**
 * @brief Calculates the content-space X coordinate of the center of the thumb.
 *
 * The center position depends on the current ON/OFF state.
 *
 * @return The X coordinate of the thumb center in content-space.
 */
float Switch::getThumbCenterX_Content() const
{
    // Thumb is a square whose size is constrained by track height and padding
    const float thumbRadius = m_style.trackHeight / 2.0f - m_style.thumbPadding;
    // Calculate center for OFF state (left side)
    const float leftCenter  = m_posX + m_style.thumbPadding + thumbRadius;
    // Calculate center for ON state (right side)
    const float rightCenter = m_posX + m_width - m_style.thumbPadding - thumbRadius;
    return m_isOn ? rightCenter : leftCenter;
}

/**
 * @brief Calculates the content-space bounding box (SDL_FRect) of the switch thumb.
 *
 * @return The SDL_FRect defining the thumb's size and content-space position.
 */
SDL_FRect Switch::getThumbRect_Content() const
{
    const float thumbRadius = m_style.trackHeight / 2.0f - m_style.thumbPadding;
    const float cx = getThumbCenterX_Content();
    const float cy = m_posY + m_height / 2.0f; // Y center is always in the middle of the track
    // Thumb is a square with size 2*radius
    return SDL_FRect{ cx - thumbRadius, cy - thumbRadius, thumbRadius * 2.0f, thumbRadius * 2.0f };
}

/**
 * @brief Renders the switch, including the track, the thumb, and optional label text.
 *
 * @param renderer The SDL_Renderer context.
 * @param viewOffset The screen-space offset inherited from parent containers (e.g., ScrollView).
 */
void Switch::draw(SDL_Renderer* renderer, const SDL_FPoint& viewOffset)
{
    if (!renderer) return;

    // Determine colors based on ON/OFF state and hover state
    SDL_Color trackCol = m_isOn ? m_style.trackColorOn : m_style.trackColorOff;
    SDL_Color thumbCol = m_isOn ? m_style.thumbColorOn : m_style.thumbColorOff;
    if (m_hovered) {
        trackCol = m_style.hoverTrackColor;
        thumbCol = m_style.hoverThumbColor;
    }

    // Track drawing (screen-space)
    SDL_FRect trackRect = { m_posX + viewOffset.x, m_posY + viewOffset.y, m_width, m_height };
    SDL_SetRenderDrawColor(renderer, trackCol.r, trackCol.g, trackCol.b, trackCol.a);
    SDL_RenderFillRect(renderer, &trackRect);

    // Thumb drawing (screen-space)
    SDL_FRect thumb = getThumbRect_Content();
    // Apply viewOffset to the content-space thumb rect
    SDL_FRect thumbScreen = { thumb.x + viewOffset.x, thumb.y + viewOffset.y, thumb.w, thumb.h };
    SDL_SetRenderDrawColor(renderer, thumbCol.r, thumbCol.g, thumbCol.b, thumbCol.a);
    SDL_RenderFillRect(renderer, &thumbScreen);

    // Optional labels rendering via TextRenderer
    if (!TextRenderer::getInstance().isInitialized()) return;

    // Use the label corresponding to the current state
    const std::string& label = m_isOn ? m_style.labelOn : m_style.labelOff;
    if (label.empty()) return;

    const int fontSize = m_style.labelFontSize;
    const SDL_Color labelColor = m_style.labelColor;

    int tw = 0, th = 0;
    TextRenderer::getInstance().measureText(label, fontSize, tw, th);

    // Position text: centered on the thumb
    const float thumbCenterX = getThumbCenterX_Content() + viewOffset.x;
    const float thumbCenterY = m_posY + m_height / 2.0f + viewOffset.y;
    const float tx = thumbCenterX - tw / 2.0f;
    const float ty = thumbCenterY - th / 2.0f;
    TextRenderer::getInstance().renderText(label, static_cast<int>(tx), static_cast<int>(ty), labelColor, fontSize);
}

/**
 * @brief Handles input events for the switch.
 *
 * Supports click-to-toggle logic where the toggle occurs on the mouse button UP
 * event, provided the cursor started and ended inside the switch bounds.
 *
 * @param event The SDL_Event to process.
 * @return true if the switch state or visual state (hover/press) changed, false otherwise.
 */
bool Switch::handleEvent(const SDL_Event& event)
{
    // Expect content-space coordinates (must be pre-translated by parent if nested)
    float mx = 0.0f, my = 0.0f;
    if (event.type == SDL_EVENT_MOUSE_MOTION) {
        mx = static_cast<float>(event.motion.x); my = static_cast<float>(event.motion.y);
    } else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN || event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
        mx = static_cast<float>(event.button.x); my = static_cast<float>(event.button.y);
    } else {
        return false;
    }

    // Check if the cursor is currently inside the switch's bounds
    const bool inside = (mx >= m_posX && mx <= (m_posX + m_width) &&
                         my >= m_posY && my <= (m_posY + m_height));

    bool changed = false;

    // Update hover state
    if (inside != m_hovered) { m_hovered = inside; changed = true; }

    // Mouse Button Down (LMB)
    if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button == SDL_BUTTON_LEFT) {
        if (inside) {
            // Start press tracking
            m_isPressed = true;
            m_wasInside = true;
            changed = true;
        }
    }
    // Mouse Button Up (LMB)
    else if (event.type == SDL_EVENT_MOUSE_BUTTON_UP && event.button.button == SDL_BUTTON_LEFT) {
        // Only toggle if the press started inside, the release happened inside, and the button was pressed
        if (m_isPressed && m_wasInside && inside) {
            m_isOn = !m_isOn;
            if (m_onToggle) m_onToggle(m_isOn);
            changed = true;
        }
        // Reset press tracking regardless of toggle outcome
        m_isPressed = false;
        m_wasInside = false;
    }

    return changed;
}

/**
 * @brief Recalculates the switch's position based on its PositionParams and parent dimensions.
 *
 * The switch's size (m_width, m_height) is fixed by the style.
 *
 * @param parentWidth The available width in the parent container.
 * @param parentHeight The available height in the parent container.
 */
void Switch::recalculateLayout(int parentWidth, int parentHeight)
{
    // Resolve final position using PositionParams, fixed size, and parent size
    SDL_Point p = XenUI::CalculateFinalPosition(m_posParams,
                                                static_cast<int>(m_width),
                                                static_cast<int>(m_height), parentWidth, parentHeight);
    m_posX = static_cast<float>(p.x);
    m_posY = static_cast<float>(p.y);
}

/**
 * @brief Returns the content-space bounding box of the entire switch control.
 *
 * @return The SDL_FRect defining the bounds.
 */
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
    // Definition for the extern unordered_map declared in the header file
    std::unordered_map<std::string, ImmediateSwitchState> immediateSwitchStates;
}

/**
 * @brief Immediate Mode: Renders and manages a toggle switch control.
 *
 * Handles state persistence, input processing, and rendering for the switch.
 *
 * @param id Unique identifier (for state persistence).
 * @param posParams Positional constraints.
 * @param style Visual style.
 * @param pValue Pointer to the external boolean value (read/write).
 * @param triggerOnPress If true, toggle happens on mouse down; otherwise, it happens on mouse up (standard click).
 * @param parentWidth The width of the parent container for layout resolution.
 * @param parentHeight The height of the parent container for layout resolution.
 * @param viewOffset Screen-space offset from parent (applied to drawing).
 * @param event The current SDL_Event pointer (optional: not directly used as mouse state is polled).
 * @return true if the external value (*pValue) was toggled this frame, false otherwise.
 */
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
    // Must have a pointer to the user's state variable
    if (!pValue) return false;

    // --- State Initialization and Retrieval ---
    auto it = Detail::immediateSwitchStates.find(id);
    if (it == Detail::immediateSwitchStates.end()) {
        // Initialize state if it does not exist
        Detail::immediateSwitchStates[id] = {};
    }
    Detail::ImmediateSwitchState& st = Detail::immediateSwitchStates[id];

    // --- Geometry Calculation (Content-Space) ---
    const float width  = style.trackWidth;
    const float height = style.trackHeight;
    SDL_Point fp;

    // Resolve final position using parent dimensions if provided, otherwise use global context
    if (parentWidth > 0 && parentHeight > 0) {
        fp = XenUI::CalculateFinalPosition(posParams,
                                           static_cast<int>(width),
                                           static_cast<int>(height),
                                           parentWidth,
                                           parentHeight);
    } else {
        // Fallback: uses the current window size context internally in CalculateFinalPosition
        fp = XenUI::CalculateFinalPosition(posParams,
                                           static_cast<int>(width),
                                           static_cast<int>(height));
    }

    const float finalX = (float)fp.x; // Final content-space X
    const float finalY = (float)fp.y; // Final content-space Y

    // --- Input Processing (Uses content-space coordinates) ---
    float mx, my;
    Uint32 mstate = SDL_GetMouseState(&mx, &my);
    // Convert mouse position from window/screen space to content-space
    const float mouseX = mx - viewOffset.x;
    const float mouseY = my - viewOffset.y;
    const bool  leftDown = (mstate & SDL_BUTTON_MASK(SDL_BUTTON_LEFT)) != 0;

    // Check if mouse is over the switch's bounds (content-space check)
    const bool inside = (mouseX >= finalX && mouseX <= (finalX + width) &&
                         mouseY >= finalY && mouseY <= (finalY + height));
    st.hovered = inside;

    bool toggledThisFrame = false;

    // Input handling logic
    if (leftDown && !st.isPressed) {
        // Mouse Down: Start press tracking
        if (inside) {
            st.isPressed = true;
            st.wasInside = true;
            if (triggerOnPress) {
                // Toggle immediately on press
                *pValue = !*pValue;
                toggledThisFrame = true;
            }
        }
    } else if (!leftDown && st.isPressed) {
        // Mouse Up: End press tracking
        if (st.wasInside && inside && !triggerOnPress) {
            // Toggle on release if not set to triggerOnPress
            *pValue = !*pValue;
            toggledThisFrame = true;
        }
        st.isPressed = false;
        st.wasInside = false;
    }

    // --- Drawing (Uses screen-space coordinates) ---
    SDL_Renderer* r = TextRenderer::getInstance().isInitialized() ? TextRenderer::getInstance().getRenderer() : nullptr;
    if (!r) return toggledThisFrame; // Cannot draw without a renderer

    // Determine colors based on external value and hover state
    SDL_Color trackCol = *pValue ? style.trackColorOn : style.trackColorOff;
    SDL_Color thumbCol = *pValue ? style.thumbColorOn : style.thumbColorOff;
    if (st.hovered) {
        trackCol = style.hoverTrackColor;
        thumbCol = style.hoverThumbColor;
    }

    // Track drawing (screen-space)
    // Apply finalX/finalY (content-space) + viewOffset (scroll offset)
    SDL_FRect trackRect = { finalX + viewOffset.x, finalY + viewOffset.y, width, height };
    SDL_SetRenderDrawColor(r, trackCol.r, trackCol.g, trackCol.b, trackCol.a);
    SDL_RenderFillRect(r, &trackRect);

    // Thumb position calculation (content-space center)
    const float thumbRadius = height / 2.0f - style.thumbPadding;
    const float leftCenter  = finalX + style.thumbPadding + thumbRadius;
    const float rightCenter = finalX + width - style.thumbPadding - thumbRadius;
    // Current center X based on external value
    const float cx          = *pValue ? rightCenter : leftCenter;
    const float cy          = finalY + height / 2.0f;

    // Thumb drawing (screen-space)
    SDL_FRect thumbRect = {
        (cx - thumbRadius) + viewOffset.x, // Center X minus radius, plus scroll offset
        (cy - thumbRadius) + viewOffset.y, // Center Y minus radius, plus scroll offset
        thumbRadius * 2.0f,
        thumbRadius * 2.0f
    };
    SDL_SetRenderDrawColor(r, thumbCol.r, thumbCol.g, thumbCol.b, thumbCol.a);
    SDL_RenderFillRect(r, &thumbRect);

    // Label drawing (only active label, centered on thumb)
    if (TextRenderer::getInstance().isInitialized()) {
        const std::string& label = *pValue ? style.labelOn : style.labelOff;
        if (!label.empty()) {
            const int fontSize   = style.labelFontSize;
            const SDL_Color col  = style.labelColor;

            int tw = 0, th = 0;
            TextRenderer::getInstance().measureText(label, fontSize, tw, th);
            // Position text center at thumb center (cx + viewOffset.x)
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