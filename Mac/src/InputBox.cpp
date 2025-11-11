


// SPDX-License-Identifier: Apache-2.0
/*
 *  Copyright (C) 2025 MD S M Sarowar Hossain
 */
//
//
// Implementation of the InputBox retained-mode UI control, managing text input,
// cursor blinking, scrolling, and rendering.
//
#include "InputBox.h"
#include <algorithm> // For std::min/max
// #include <iostream> // For debugging
// Forwarding IControl signature to your convenience implementation


namespace XenUI {

// Helper to check for Ctrl (Windows/Linux) or Cmd (macOS)
/**
 * @brief Checks if the primary platform modifier key (Ctrl or Cmd/GUI) is active.
 *
 * @param mod The SDL_Keymod flags from the event.
 * @return true if the Left/Right Ctrl or Left/Right GUI (Command) modifier is set.
 */
static bool isPrimaryModifier(SDL_Keymod mod) {
    return (mod & (SDL_KMOD_LCTRL | SDL_KMOD_RCTRL | SDL_KMOD_LGUI | SDL_KMOD_RGUI)) != 0;

}

/**
 * @brief Constructor for the InputBox control (Retained Mode).
 *
 * @param posParams Parameters for resolving the control's position.
 * @param initialText The initial text content of the input box.
 * @param width The fixed width of the input box (must be > 0).
 * @param fontSize The size of the text font.
 * @param style The visual style parameters.
 * @param isPassword Flag to display input as mask characters (e.g., '*').
 */
InputBox::InputBox(const PositionParams& posParams, const std::string& initialText,
                   int width, int fontSize, InputBoxStyle style, bool isPassword)
    : m_posParams(posParams),
      m_text(""), // Initialize empty, then use setText
      m_style(style),
      m_fontSize(fontSize > 0 ? fontSize : DEFAULT_INPUT_FONT_SIZE),
      m_isPassword(isPassword),
      m_width(width > 0 ? width : 200),
      m_textRenderer(TextRenderer::getInstance()),
      m_window(nullptr)
{
    calculateHeight();     // Calculate height based on font/padding (needs m_fontSize)
    setText(initialText);  // Sets initial text, cursor, display text, and clamps
    recalculatePosition(); // Calculate initial screen position based on window size
                           // m_cursorPos, m_scrollX, m_selectionStart are handled by setText & default initializers
}


/**
 * @brief Handles an incoming SDL event, routing it to the full event handler.
 *
 * This implementation uses the window and view offset stored from the last draw/event cycle
 * or internal state if a window was explicitly set.
 *
 * @param event The SDL_Event to process.
 * @return true if the control consumed or reacted to the event, false otherwise.
 */
bool InputBox::handleEvent(const SDL_Event& event) {
    // forward to the two-arg form using stored window (if any)
    SDL_Window* win = m_forwardedWindow ? m_forwardedWindow : m_window;
    return handleEvent(event, win, m_lastViewOffset);
}

// Backwards-compatible draw()
/**
 * @brief Renders the InputBox with a zero view offset (for non-scrolling contexts).
 * @param renderer The SDL_Renderer context.
 */
void InputBox::draw(SDL_Renderer* renderer) {
    draw(renderer, SDL_FPoint{0.0f, 0.0f});
}

// // Backwards-compat recalc name
// void InputBox::recalculatePosition() {
// //     recalculateLayout();
// // }

/**
 * @brief Updates the text string displayed to the user (m_displayText).
 *
 * If the input box is in password mode, the display text is replaced with '*' characters.
 */
void InputBox::updateDisplayText() {
    if (m_isPassword) {
        // Use '*' for every character if password mode is active
        m_displayText = std::string(m_text.length(), '*');
    } else {
        // Otherwise, the display text is the same as the actual text
        m_displayText = m_text;
    }
}

/**
 * @brief Calculates the fixed height of the input box based on font size and padding.
 *
 * Ensures a minimum height if font measurement fails or is too small.
 */
void InputBox::calculateHeight() {
    // Fallback if the TextRenderer hasn't been initialized yet
    if (!m_textRenderer.isInitialized()) {
        m_height = (m_fontSize > 0 ? m_fontSize : 16) + 2 * m_style.paddingY;
        return;
    }
    int text_w, text_h;
    // Measure a standard character ('M') to get the appropriate line height
    m_textRenderer.measureText("M", m_fontSize, text_w, text_h);
    // Height is calculated as the text height plus vertical padding
    m_height = text_h + 2 * m_style.paddingY;
    // Ensure the height meets a minimum standard
    if (m_height < (m_fontSize + 2 * m_style.paddingY)) {
        m_height = m_fontSize + 2 * m_style.paddingY;
    }
}

/**
 * @brief Recalculates the position (m_posX, m_posY) using the window size as the parent bounds.
 *
 * Used for initial position setup when parent dimensions are not yet known.
 */
void InputBox::recalculatePosition() {
    // Calculates position relative to the overall window bounds
    SDL_Point finalPos = XenUI::CalculateFinalPosition(m_posParams, m_width, m_height);
    m_posX = finalPos.x;
    m_posY = finalPos.y;
}

/**
 * @brief Updates the input box's state over time.
 *
 * This function primarily manages the cursor blinking animation.
 *
 * @param deltaTime The time elapsed since the last update call (in seconds).
 * @return true if the control's visual state changed (i.e., cursor toggled visibility), false otherwise.
 */
bool XenUI::InputBox::update(float deltaTime) {
    bool changed = false;

    // Only handle blinking if the control currently has focus
    if (IControl::hasFocus()) {
        m_cursorBlinkTimer += deltaTime;
        if (m_cursorBlinkTimer >= CURSOR_BLINK_RATE) {
            // Blink the cursor by toggling its visibility
            m_isCursorVisible = !m_isCursorVisible;
            // Reset the timer, ensuring to keep synchronization accurate by subtracting full multiples
            m_cursorBlinkTimer -= CURSOR_BLINK_RATE
                              * static_cast<int>(m_cursorBlinkTimer / CURSOR_BLINK_RATE);
            changed = true;
        }
    } else {
        // If focus is lost, ensure the cursor is not visible
        if (m_isCursorVisible) {
            m_isCursorVisible = false;
            changed = true;
        }
    }

    return changed;
}

/**
 * @brief Indicates whether the control requires periodic updates or redraws.
 *
 * @return true if the control has focus (due to cursor blinking), false otherwise.
 */
bool XenUI::InputBox::isAnimating() const {
        // While the box has focus, the cursor is blinking, hence it is "animating".
        return IControl::hasFocus();
    }


/**
 * @brief Recalculates the layout, including height, absolute position, and clamping scroll/cursor.
 *
 * @param parentWidth The width of the parent container or content area.
 * @param parentHeight The height of the parent container or content area.
 */
void InputBox::recalculateLayout(int parentWidth, int parentHeight) {
    // 1. Compute height based on font metrics + padding
    calculateHeight();

    // 2. Compute final absolute position from PositionParams
    SDL_Point finalPos = XenUI::CalculateFinalPosition(m_posParams, m_width, m_height, parentWidth, parentHeight);
    m_posX = finalPos.x;
    m_posY = finalPos.y;

    // 3. Ensure cursor and scroll offsets remain valid after layout change
    clampCursorAndScroll();
}

// IControl-style getBounds
/**
 * @brief Returns the content-space bounding rectangle of the control.
 *
 * @return The SDL_FRect representing the control's position and size relative to its content area.
 */
SDL_FRect InputBox::getBounds() const {
    return SDL_FRect{ float(m_posX), float(m_posY), float(m_width), float(m_height) };
}

// additional function


/**
 * @brief Renders the InputBox control to the screen.
 *
 * @param renderer The SDL_Renderer context.
 * @param viewOffset The offset applied by a parent container (e.g., ScrollView) to content-space coordinates.
 */
void InputBox::draw(SDL_Renderer* renderer, const SDL_FPoint& viewOffset) {
    if (!renderer) return;

    // 1. Calculate absolute screen coordinates of the control's overall bounding box
    float absX = viewOffset.x + float(m_posX);
    float absY = viewOffset.y + float(m_posY);

    SDL_FRect boxRect = {
        absX,
        absY,
        (float)m_width,
        (float)m_height
    };

    // 2. Draw Background
    if (m_style.drawBackground) {
        SDL_SetRenderDrawColor(renderer,
            m_style.bgColor.r, m_style.bgColor.g, m_style.bgColor.b, m_style.bgColor.a);
        SDL_RenderFillRect(renderer, &boxRect);
    }

    // 3. Draw Border
    if (m_style.drawBorder) {
        // Use focused color if the control has focus
        SDL_Color borderColor = m_hasFocus ? m_style.focusedBorderColor : m_style.borderColor;
        SDL_SetRenderDrawColor(renderer, borderColor.r, borderColor.g, borderColor.b, borderColor.a);
        SDL_RenderRect(renderer, &boxRect);
    }

    // 4. Define Inner Text Area (where text/cursor are drawn and clipped)
    SDL_FRect innerRect = {
        absX + (float)m_style.paddingX,
        absY + (float)m_style.paddingY,
        (float)(m_width - 2 * m_style.paddingX),
        (float)(m_height - 2 * m_style.paddingY)
    };
    // Ensure minimum dimensions
    if (innerRect.w < 1.0f) innerRect.w = 1.0f;
    if (innerRect.h < 1.0f) innerRect.h = 1.0f;

    // 5. Manage Clip Rectangles (Handle intersection with parent's clip if active)

    // Save existing clip state
    SDL_Rect oldClip;
    bool hadOldClip = SDL_RenderClipEnabled(renderer);
    if (hadOldClip) SDL_GetRenderClipRect(renderer, &oldClip);

    // Build inner rect in integer coordinates for the clip function
    SDL_Rect innerClip = {
        static_cast<int>(std::floor(innerRect.x + 0.5f)),
        static_cast<int>(std::floor(innerRect.y + 0.5f)),
        static_cast<int>(std::floor(innerRect.w + 0.5f)),
        static_cast<int>(std::floor(innerRect.h + 0.5f))
    };

    // If there was a prior clip (e.g., from a ScrollView), compute the intersection.
    // Otherwise, the final clip is just the inner rect of the input box.
    SDL_Rect finalClip = innerClip;
    if (hadOldClip) {
        // compute intersection coordinates
        int x1 = std::max(oldClip.x, innerClip.x);
        int y1 = std::max(oldClip.y, innerClip.y);
        int x2 = std::min(oldClip.x + oldClip.w, innerClip.x + innerClip.w);
        int y2 = std::min(oldClip.y + oldClip.h, innerClip.y + innerClip.h);
        if (x2 <= x1 || y2 <= y1) {
            // No intersection -> set an empty clip to hide content
            finalClip = {0,0,0,0};
        } else {
            // Set the intersection rectangle
            finalClip = { x1, y1, x2 - x1, y2 - y1 };
        }
    }

    // Apply the final clip rectangle
    SDL_SetRenderClipRect(renderer, &finalClip);

    // Base X for text rendering (absolute screen-space), applying horizontal scroll
    float textRenderBaseX = innerRect.x - (float)m_scrollX;

    // 6. Draw selection background (if text is selected)
    if (m_selectionStart != -1 && m_selectionStart != m_cursorPos) {
        int selS = std::min(m_selectionStart, m_cursorPos);
        int selE = std::max(m_selectionStart, m_cursorPos);

        // Calculate screen-space X coordinates for the selection start and end positions
        float selStartX = textRenderBaseX + static_cast<float>(getTextXPosition(selS));
        float selEndX   = textRenderBaseX + static_cast<float>(getTextXPosition(selE));

        SDL_FRect selectionDrawRect = {
            selStartX,
            innerRect.y,
            selEndX - selStartX,
            innerRect.h
        };
        SDL_SetRenderDrawColor(renderer,
            m_style.selectionBgColor.r,
            m_style.selectionBgColor.g,
            m_style.selectionBgColor.b,
            m_style.selectionBgColor.a);
        SDL_RenderFillRect(renderer, &selectionDrawRect);
    }

    // 7. Draw text (using TextRenderer caching)
    if (m_textRenderer.isInitialized() && !m_displayText.empty()) {
        int textTextureW = 0, textTextureH = 0;
        // Request the text texture from the cache or generate it
        SDL_Texture* textTexture = m_textRenderer.getInstance().renderTextToTexture(
            m_displayText, m_style.textColor, m_fontSize, textTextureW, textTextureH);

        if (textTexture) {
            // Calculate vertical center alignment
            float textRenderH = static_cast<float>(textTextureH);
            float textRenderY = innerRect.y + (innerRect.h - textRenderH) / 2.0f;
            if (textTextureH > static_cast<int>(innerRect.h)) {
                // If text is taller than the area, clamp it to the top edge
                textRenderY = innerRect.y;
                textRenderH = innerRect.h;
            }

            SDL_FRect textDestRect = {
                textRenderBaseX,
                textRenderY,
                static_cast<float>(textTextureW),
                textRenderH
            };

            // Render the text texture
            SDL_RenderTexture(renderer, textTexture, nullptr, &textDestRect);

            // ---- debug: draw clipped red outline (inside current clip) ----
#ifdef INPUTBOX_DEBUG_RECT
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 128); // semi-transparent red
            SDL_RenderRect(renderer, &textDestRect);
#endif

            // Note: The TextRenderer manages the texture lifecycle (caching).
        }
    }

    // 8. Draw caret (cursor) if focused and visible
    if (m_hasFocus && m_isCursorVisible) {
        // Calculate the screen X position of the cursor
        float cursorScreenX = textRenderBaseX + static_cast<float>(getTextXPosition(m_cursorPos));
        SDL_SetRenderDrawColor(renderer,
            m_style.cursorColor.r, m_style.cursorColor.g, m_style.cursorColor.b, m_style.cursorColor.a);

        // Draw a vertical line for the cursor, spanning the height of the inner rect
        SDL_RenderLine(renderer, cursorScreenX, innerRect.y, cursorScreenX, innerRect.y + innerRect.h - 1.0f);
    }

    // 9. Restore original clip rectangle
    // Restore the clip to the previously saved clip (or to null if none existed)
    if (hadOldClip) SDL_SetRenderClipRect(renderer, &oldClip);
    else SDL_SetRenderClipRect(renderer, nullptr);

// ---- debug: draw un-clipped green outline (shows actual texture coords on screen) ----
#ifdef INPUTBOX_DEBUG_RECT
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 96); // translucent green
    SDL_RenderRect(renderer, &textDestRect);
#endif

}

// ScrollView::handleEvent(event, window, viewOffset)

/**
 * @brief Handles an incoming SDL event for the InputBox control.
 *
 * This is the primary event handling function, designed to work correctly when
 * nested inside containers like ScrollView by using translated content-space coordinates
 * and incorporating the view offset for setting the IME area.
 *
 * @param event The SDL_Event to process (mouse coordinates must be in content-space if translated).
 * @param window The SDL_Window associated with the event/control, necessary for SDL_StartTextInput/IME setup.
 * @param viewOffset The offset applied by the parent container (e.g., scroll position) to convert content-space to screen-space.
 * @return true if the control's state changed or the event was handled, false otherwise.
 */
// New IControl-style handleEvent implementation (use the forwarded/translated events)
bool InputBox::handleEvent(const SDL_Event& event, SDL_Window* window, const SDL_FPoint& viewOffset) {
    bool changed = false;

    // Remember the provided window for later use in single-arg handleEvent and draw functions.
    if (window) m_window = window;

    // NOTE: events forwarded by ScrollView are already translated into content-space,
    // so hit-testing and local coordinates comparison use m_posX/m_posY as content-space positions.

    float mx = 0.f, my = 0.f;
    bool haveMouseCoords = false;
    // Extract mouse coordinates from relevant event types
    if (event.type == SDL_EVENT_MOUSE_MOTION) {
        mx = static_cast<float>(event.motion.x);
        my = static_cast<float>(event.motion.y);
        haveMouseCoords = true;
    } else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN || event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
        mx = static_cast<float>(event.button.x);
        my = static_cast<float>(event.button.y);
        haveMouseCoords = true;
    }

    // Define the local content-space bounds of the input box
    const float left   = static_cast<float>(m_posX);
    const float top    = static_cast<float>(m_posY);
    const float right  = left + static_cast<float>(m_width);
    const float bottom = top  + static_cast<float>(m_height);

    // Lambda to check if a content-space point is within the control's bounds
    auto pointInside = [&](float x, float y) {
        return (x >= left && x <= right && y >= top && y <= bottom);
    };

    // --- Mouse down: focus & start selection/dragging ---
    if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button == SDL_BUTTON_LEFT && haveMouseCoords) {
        if (pointInside(mx, my)) {
            if (!IControl::hasFocus()) {
                // If the box didn't have focus, focus it now (which handles SDL_StartTextInput)
                focus(window, viewOffset);
            }

            // mx is content-space X; convert this content-space X coordinate to a character index
            m_cursorPos = getIndexFromXCoord(static_cast<int>(mx));
            m_selectionStart = m_cursorPos; // Anchor the selection start
            m_isDragging = true;
            changed = true;

            // Update IME (Input Method Editor) area using the combined content position and view offset
            if (m_window) {
                SDL_FPoint useOffset = viewOffset; // Use the current offset provided by the caller

                // Calculate the screen-space rectangle of the input box
                SDL_Rect inputRect = {
                    static_cast<int>(std::floor(useOffset.x + static_cast<float>(m_posX) + 0.5f)),
                    static_cast<int>(std::floor(useOffset.y + static_cast<float>(m_posY) + 0.5f)),
                    m_width,
                    m_height
                };
                // Inform SDL where the text input area is and the current cursor position
                SDL_SetTextInputArea(m_window, &inputRect, m_cursorPos);
                SDL_Log("InputBox::focus rect=(%d,%d,%d,%d) cursor=%d useOffset=(%.1f,%.1f) window=%p",
                        inputRect.x, inputRect.y, inputRect.w, inputRect.h, m_cursorPos,
                        useOffset.x, useOffset.y, (void*)m_window);
            }

        }
//          else {
//     // Clicked outside: unfocus (logic commented out but noted to be here in principle)
//     if (m_hasFocus) {
//         // This is the existing unfocus logic
//         m_hasFocus = false;
//         m_isDragging = false;
//         m_selectionStart = -1;

//         // --- ADD THIS LINE ---
//         if (window) SDL_StopTextInput(window); // Use the window passed to the function

//         changed = true;
//     }
// }
    }
    // --- Mouse up ---
    else if (event.type == SDL_EVENT_MOUSE_BUTTON_UP && event.button.button == SDL_BUTTON_LEFT && haveMouseCoords) {
        if (m_isDragging) {
            // Stop the dragging state
            m_isDragging = false;
            changed = true;
        }
    }
    // --- Mouse motion while dragging ---
    else if (event.type == SDL_EVENT_MOUSE_MOTION && m_isDragging && haveMouseCoords) {
        // Determine the new character index under the mouse
        int idx = getIndexFromXCoord(static_cast<int>(mx));
        if (idx != m_cursorPos) {
            m_cursorPos = idx;
            // When dragging, the selection anchor (m_selectionStart) remains fixed
            changed = true;

            // Update IME area caret pos (required for accurate IME cursor placement)
            if (m_window) {
                // Use the last stored offset, as the viewOffset passed here might not be the latest if the control is not in a scrolling area
                SDL_FPoint useOffset = m_lastViewOffset;
                SDL_Rect inputRect = {
                    static_cast<int>(std::floor(useOffset.x + static_cast<float>(m_posX) + 0.5f)),
                    static_cast<int>(std::floor(useOffset.y + static_cast<float>(m_posY) + 0.5f)),
                    m_width,
                    m_height
                };
                // Only update the cursor position within the existing text input area
                SDL_SetTextInputArea(m_window, &inputRect, m_cursorPos);
            }
        }
    }

    // Keyboard & text input when focused
    if (m_hasFocus) {
        if (event.type == SDL_EVENT_TEXT_INPUT) {
            // Handle insertion of characters from standard keyboard input or IME composition result
            handleTextInput(event.text.text);
            changed = true;
        } else if (event.type == SDL_EVENT_KEY_DOWN) {
            // Handle special keys (e.g., backspace, arrows, copy/paste shortcuts)
            handleKeyDown(event.key.key, static_cast<SDL_Keymod>(event.key.mod));
            changed = true;
        }else if (event.type == SDL_EVENT_TEXT_EDITING) {
            // Log IME pre-edit text updates (when text is actively being composed)
            SDL_Log(
                "App: SDL_EVENT_TEXT_EDITING --- text: '%s', start: %d, length: %d",
                event.edit.text ? event.edit.text : "NULL",
                event.edit.start,
                event.edit.length
            );
        }
        // SDL_EVENT_TEXT_EDITING can be handled where IME pre-edit support is desired
    }

    // After state changes, ensure the cursor is scrolled into the visible text area
    clampCursorAndScroll();

    return changed;
}



/**
 * @brief Sets the SDL_Window associated with this input box.
 *
 * This is primarily used for the `SDL_StartTextInput` and IME functions.
 * The `m_forwardedWindow` stores the window context used for input.
 *
 * @param window The SDL_Window pointer.
 */
void InputBox::setWindow(SDL_Window* window) {
    m_forwardedWindow = window;
    // Optionally update m_window if currently focused to ensure IME area update uses the correct context
    if (IControl::hasFocus() && window) m_window = window;
}

/**
 * @brief Updates the stored view offset and potentially updates the IME area location.
 *
 * This function must be called by a parent (like ScrollView) whenever the content
 * offset changes, even if the input box itself didn't cause the scroll.
 *
 * @param viewOffset The new scroll offset (content-to-screen translation).
 */
void InputBox::setViewOffset(const SDL_FPoint& viewOffset) {
    m_lastViewOffset = viewOffset;
    // If the input box is currently focused, update the IME rectangle immediately
    if (IControl::hasFocus() && m_window) {
        // Calculate the new screen-space input rectangle
        SDL_Rect inputRect = {
            static_cast<int>(std::floor(m_lastViewOffset.x + static_cast<float>(m_posX) + 0.5f)),
            static_cast<int>(std::floor(m_lastViewOffset.y + static_cast<float>(m_posY) + 0.5f)),
            m_width,
            m_height
        };
        // Set the new text input area to follow the scrolling box on screen
        SDL_SetTextInputArea(m_window, &inputRect, m_cursorPos);
    }
}


/**
 * @brief Attempts to set the focus to this input box using the stored view offset.
 *
 * This is a convenience overload that retrieves the last known view offset.
 *
 * @param window The SDL_Window context required for starting text input.
 */
void InputBox::focus(SDL_Window* window) {
    // use stored view offset if we have one
    focus(window, m_lastViewOffset);
}


// InputBox::focus(window, viewOffset)
/**
 * @brief Sets focus to this InputBox, enabling keyboard input and initiating cursor blinking.
 *
 * This function handles `SDL_StartTextInput` and updates the Input Method Editor (IME)
 * area to reflect the input box's screen position, especially crucial when the box
 * is nested within a scrolling container.
 *
 * @param window The current SDL_Window.
 * @param viewOffset The content-space translation offset (from a parent ScrollView) to determine screen-space position.
 */
void InputBox::focus(SDL_Window* window, const SDL_FPoint& viewOffset) {
    // If the control already has focus, only update the necessary window/offset/IME rect
    if (IControl::hasFocus()) {
        if (window) m_window = window;
        m_lastViewOffset = viewOffset;
        if (m_window) {
            // Recalculate the screen-space IME input rectangle
            SDL_Rect inputRect = {
                static_cast<int>(std::floor(m_lastViewOffset.x + static_cast<float>(m_posX) + 0.5f)),
                static_cast<int>(std::floor(m_lastViewOffset.y + static_cast<float>(m_posY) + 0.5f)),
                m_width,
                m_height
            };
            // Update the IME position without re-starting text input
            SDL_SetTextInputArea(m_window, &inputRect, m_cursorPos);
        }
        return;
    }

    // 1. Mark focus in base class (polymorphic notification)
    IControl::focus(window);

    // 2. Store the current window and view offset
    if (window) {
        m_forwardedWindow = window;
        m_window = window;
    }
    m_lastViewOffset = viewOffset;

    // 3. Show caret and reset blink timer
    m_isCursorVisible = true;
    m_cursorBlinkTimer = 0.0f;

    // 4. Start SDL text input, enabling TEXT_INPUT and TEXT_EDITING events
    if (m_window) {
        SDL_StartTextInput(m_window);
    } else {
        // Fallback if no window is explicitly set (should generally use a window pointer)
        SDL_StartTextInput(nullptr);
    }

    // 5. Ensure the cursor position and horizontal scroll offset are valid
    clampCursorAndScroll();

    // 6. Set IME caret rect (screen-space using the viewOffset)
    SDL_Rect inputRect = {
        static_cast<int>(std::floor(m_lastViewOffset.x + static_cast<float>(m_posX) + 0.5f)),
        static_cast<int>(std::floor(m_lastViewOffset.y + static_cast<float>(m_posY) + 0.5f)),
        m_width,
        m_height
    };
    if (m_window) {
        SDL_SetTextInputArea(m_window, &inputRect, m_cursorPos);
    } else {
        SDL_SetTextInputArea(nullptr, &inputRect, m_cursorPos);
    }

    // Log the successful focus and IME area setting
    SDL_Log("InputBox::focus rect=(%d,%d,%d,%d) cursor=%d offset=(%.1f,%.1f) window=%p",
            inputRect.x, inputRect.y, inputRect.w, inputRect.h, m_cursorPos,
            m_lastViewOffset.x, m_lastViewOffset.y, (void*)m_window);
}



/**
 * @brief Removes focus from the InputBox, stopping text input and clearing selection.
 *
 * This function calls `SDL_StopTextInput` to disable keyboard events directed
 * towards this control.
 *
 * @param window The current SDL_Window (optional, used to determine which window's input to stop).
 */
void InputBox::unfocus(SDL_Window* window) {
    if (!IControl::hasFocus()) return;

    // Clear any active text selection
    m_selectionStart = -1;
    // Determine the window to stop text input on
    SDL_Window* w = window ? window : m_window;
    if (w) SDL_StopTextInput(w);
    else SDL_StopTextInput(nullptr); // Stop global text input if no window is set

    // Remove focus in the base class
    IControl::unfocus(window);
    // Clear the internal window pointer, as text input is now stopped
    m_window = nullptr;
}



/**
 * @brief Handles key down events for non-textual input operations (navigation, selection, shortcuts).
 *
 * This includes handling Backspace, Delete, navigation keys (arrows, Home, End),
 * and common primary-modifier shortcuts (Ctrl/Cmd + A, C, X, V).
 *
 * @param key The SDL_Keycode of the pressed key.
 * @param mod The SDL_Keymod state of the modifiers (Shift, Ctrl/Cmd, Alt).
 */
void InputBox::handleKeyDown(SDL_Keycode key, SDL_Keymod mod) {
    // Only process keyboard events if the control has focus
    if (!IControl::hasFocus()) return;

    // Reset cursor blink state to make it immediately visible
    m_isCursorVisible = true;
    m_cursorBlinkTimer = 0.0f;

    const bool shiftPressed = (mod & SDL_KMOD_SHIFT) != 0;
    // Check for platform-appropriate primary modifier (Ctrl on Windows/Linux, Cmd on macOS)
    const bool primaryModPressed = isPrimaryModifier(mod);
    bool textChanged = false;
    bool selectionChangedByKey = false;

    const int oldCursorPos = m_cursorPos;
    const int oldSelectionStart = m_selectionStart;

    if (primaryModPressed) {
        // --- PRIMARY MODIFIER SHORTCUTS (Ctrl/Cmd) ---
        switch (key) {
            case SDLK_A: // Select All
                m_selectionStart = 0;
                m_cursorPos = static_cast<int>(m_text.length());
                selectionChangedByKey = true;
                break;
            case SDLK_C: // Copy
                copySelectionToClipboard();
                break;
            case SDLK_X: // Cut
                cutSelectionToClipboard(); // Text is modified if selection exists
                textChanged = true;
                break;
            case SDLK_V: // Paste
                pasteFromClipboard(); // Text is modified
                textChanged = true;
                break;
            default:
                break;
        }
    } else {
        // --- STANDARD KEY ACTIONS ---
        switch (key) {
            case SDLK_BACKSPACE:
                SDL_Log("InputBox: Backspace KEYDOWN. m_text_before: '%s', cursor: %d, selectionStart: %d", m_text.c_str(), m_cursorPos, m_selectionStart);

                if (m_selectionStart != -1 && m_selectionStart != m_cursorPos) {
                    // Delete selected text if a selection exists
                    deleteSelection();
                    textChanged = true;
                    SDL_Log("InputBox: Backspace deleted selection. m_text_after: '%s', cursor: %d, selectionStart: %d", m_text.c_str(), m_cursorPos, m_selectionStart);
                } else if (m_cursorPos > 0 && !m_text.empty()) {
                    // Delete character to the left of the cursor
                    m_text.erase(m_cursorPos - 1, 1);
                    m_cursorPos--;
                    m_selectionStart = -1; // Clear selection after deletion
                    textChanged = true;
                    SDL_Log("InputBox: Backspace erased char. m_text_after: '%s', new_cursor: %d, selectionStart: %d", m_text.c_str(), m_cursorPos, m_selectionStart);
                } else {
                    SDL_Log("InputBox: Backspace no action (cursor at 0 or text empty).");
                }
                break;

            case SDLK_DELETE:
                if (m_selectionStart != -1 && m_selectionStart != m_cursorPos) {
                    // Delete selected text
                    deleteSelection();
                    textChanged = true;
                } else if (m_cursorPos < static_cast<int>(m_text.length())) {
                    // Delete character to the right of the cursor
                    m_text.erase(m_cursorPos, 1);
                    m_selectionStart = -1;
                    textChanged = true;
                }
                break;

            case SDLK_RETURN:
            case SDLK_KP_ENTER:
                // Invoke the user-defined callback for Enter key press
                if (onEnterPressed) onEnterPressed(m_text);
                break;

            case SDLK_LEFT:
                // Handle selection anchoring if Shift is pressed
                if (shiftPressed) {
                    if (m_selectionStart == -1 || oldSelectionStart == oldCursorPos) {
                        // Start a new selection at the original cursor position
                        m_selectionStart = oldCursorPos;
                    }
                } else {
                    m_selectionStart = -1; // Clear selection if not shifting
                }
                // Move cursor left
                if (m_cursorPos > 0) m_cursorPos--;
                // Collapse selection if shift was not pressed
                if (!shiftPressed && m_selectionStart != -1) m_selectionStart = m_cursorPos;
                selectionChangedByKey = true;
                break;

            case SDLK_RIGHT:
                // Handle selection anchoring if Shift is pressed
                if (shiftPressed) {
                    if (m_selectionStart == -1 || oldSelectionStart == oldCursorPos) {
                        // Start a new selection at the original cursor position
                        m_selectionStart = oldCursorPos;
                    }
                } else {
                    m_selectionStart = -1;
                }
                // Move cursor right
                if (m_cursorPos < static_cast<int>(m_text.length())) m_cursorPos++;
                // Collapse selection if shift was not pressed
                if (!shiftPressed && m_selectionStart != -1) m_selectionStart = m_cursorPos;
                selectionChangedByKey = true;
                break;

            case SDLK_HOME:
                // Move to the start of the text
                if (shiftPressed) {
                    if (m_selectionStart == -1 || oldSelectionStart == oldCursorPos) {
                        m_selectionStart = oldCursorPos;
                    }
                } else {
                    m_selectionStart = -1;
                }
                m_cursorPos = 0;
                if (!shiftPressed && m_selectionStart != -1) m_selectionStart = m_cursorPos;
                selectionChangedByKey = true;
                break;

            case SDLK_END:
                // Move to the end of the text
                if (shiftPressed) {
                    if (m_selectionStart == -1 || oldSelectionStart == oldCursorPos) {
                        m_selectionStart = oldCursorPos;
                    }
                } else {
                    m_selectionStart = -1;
                }
                m_cursorPos = static_cast<int>(m_text.length());
                if (!shiftPressed && m_selectionStart != -1) m_selectionStart = m_cursorPos;
                selectionChangedByKey = true;
                break;

            default:
                // Other keys are typically handled by SDL_EVENT_TEXT_INPUT
                break;
        }
    }

    // Update display and notify listeners if the text content changed
    if (textChanged) {
        updateDisplayText();
        if (onTextChanged) onTextChanged(m_text);
    }

    // Ensure selection is truly cleared if navigation was non-shifting
    if (selectionChangedByKey && !shiftPressed && (key == SDLK_LEFT || key == SDLK_RIGHT || key == SDLK_HOME || key == SDLK_END)) {
        m_selectionStart = -1;
    }

    // Re-validate cursor position and scroll the text to make the cursor visible
    clampCursorAndScroll();

    // Update the IME caret position on the screen
    if (m_window) {
        SDL_FPoint useOffset = m_lastViewOffset;
        SDL_Rect inputRect = {
            static_cast<int>(std::floor(useOffset.x + static_cast<float>(m_posX) + 0.5f)),
            static_cast<int>(std::floor(useOffset.y + static_cast<float>(m_posY) + 0.5f)),
            m_width,
            m_height
        };
        // Update IME with the new cursor position
        SDL_SetTextInputArea(m_window, &inputRect, m_cursorPos);
        SDL_Log("InputBox::update IME rect=(%d,%d) cursor=%d", inputRect.x, inputRect.y, m_cursorPos);
    }

    if (key == SDLK_BACKSPACE) { // Log state after processing for Backspace debug
        SDL_Log("InputBox: Backspace KEYDOWN processed. Final m_text: '%s', cursor: %d, scrollX: %d", m_text.c_str(), m_cursorPos, m_scrollX);
    }
}


/**
 * @brief Handles text input from SDL_EVENT_TEXT_INPUT.
 *
 * This function inserts text at the current cursor position, replacing any existing
 * selection, and enforces the maximum length constraint.
 *
 * @param inputText The null-terminated C-string containing the input text (e.g., a single character or composed string).
 */
void InputBox::handleTextInput(const char* inputText) {
    SDL_Log("InputBox: TEXT_INPUT event. inputText: '%s', m_text_before: '%s', cursor_before: %d", inputText ? inputText : "NULL", m_text.c_str(), m_cursorPos);

    if (!m_hasFocus || !inputText || inputText[0] == '\0') return;

    // Reset cursor blink state
    m_isCursorVisible = true;
    m_cursorBlinkTimer = 0.0f;

    // If there is an active selection, delete it first (it sets the cursor correctly)
    if (m_selectionStart != -1 && m_selectionStart != m_cursorPos) {
        deleteSelection(); // This moves cursor and clears selection internally
    }

    std::string textToInsert(inputText);
    size_t CharsToInsertCount = textToInsert.length(); // Length in bytes (sufficient for simple insertion)

    // Check and enforce maximum length constraint
    if (m_maxLength > 0) {
        if (m_text.length() >= static_cast<size_t>(m_maxLength)) {
            return; // Max length reached
        }
        if (m_text.length() + CharsToInsertCount > static_cast<size_t>(m_maxLength)) {
            // Trim the text to be inserted
            CharsToInsertCount = m_maxLength - m_text.length();
            textToInsert = textToInsert.substr(0, CharsToInsertCount);
        }
    }

    if (CharsToInsertCount > 0) {
        // Insert the new text at the current cursor position
        m_text.insert(m_cursorPos, textToInsert);
        // Advance the cursor by the number of characters inserted
        m_cursorPos += CharsToInsertCount;
        m_selectionStart = -1; // Typing always clears any selection range

        updateDisplayText();
        // Notify listeners that the text content has changed
        if (onTextChanged) {
            onTextChanged(m_text);
        }
    }

    // Ensure cursor visibility and update scroll offset
    clampCursorAndScroll();

    // Update the IME caret position on the screen
    if (m_window) {
    SDL_FPoint useOffset = m_lastViewOffset;
    SDL_Rect inputRect = {
        static_cast<int>(std::floor(useOffset.x + static_cast<float>(m_posX) + 0.5f)),
        static_cast<int>(std::floor(useOffset.y + static_cast<float>(m_posY) + 0.5f)),
        m_width,
        m_height
    };
    // Update IME with the new cursor position
    SDL_SetTextInputArea(m_window, &inputRect, m_cursorPos);
    SDL_Log("InputBox::update IME rect=(%d,%d) cursor=%d", inputRect.x, inputRect.y, m_cursorPos);
}

    SDL_Log("InputBox: TEXT_INPUT processed. Final m_text: '%s', cursor_after: %d, scrollX: %d", m_text.c_str(), m_cursorPos, m_scrollX);

}




// InputBox implementation file (partial)
//
// Copyright (c) 2025 XenonUI
// Author: MD S M Sarowar Hossain
//
// Contains implementation for mouse event handling, cursor/scroll clamping,
// and text manipulation helper functions for the InputBox retained-mode control.
//

/**
 * @brief Handles mouse events specific to the InputBox control.
 *
 * This function performs hit-testing, manages focus, initiates dragging for selection,
 * and updates the cursor position based on mouse clicks and movements.
 *
 * @param event The SDL_Event (expected to have content-space coordinates if from a parent container).
 * @param window The SDL_Window pointer, used for SDL_StartTextInput/IME management.
 * @param viewOffset The scroll offset applied by the parent (content-space to screen-space translation).
 */
void InputBox::handleMouseInput(const SDL_Event& event, SDL_Window* window, const SDL_FPoint& viewOffset) {
    int mouseX = 0;
    int mouseY = 0;


    if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        // Coordinates in event.button are already in content-space (if translated by parent)
        mouseX = static_cast<int>(event.button.x);
        mouseY = static_cast<int>(event.button.y);
        // Check if the click is within the control's content-space bounds
        bool inside = (mouseX >= m_posX && mouseX <= (m_posX + m_width) &&
                       mouseY >= m_posY && mouseY <= (m_posY + m_height));

        // Determine the window context, preferring the explicitly passed window
        SDL_Window* currentWindow = window ? window : SDL_GetWindowFromID(event.button.windowID);

        if (event.button.button == SDL_BUTTON_LEFT) {
            // Attempt to focus regardless of 'inside' status, so unfocus logic runs later if outside
            if (!hasFocus()) {
                focus(currentWindow, viewOffset); // Focus handles SDL_StartTextInput and sets IME rect
            }

            if (inside) {
                if (!m_hasFocus) {
                    // Re-check focus and ensure it's set with the correct context
                    focus(currentWindow, viewOffset);
                } else if (!m_window) { // If somehow focused but m_window is null
                    m_window = currentWindow;
                }

                // Convert content-space X coordinate to a character index
                int clickedIndex = getIndexFromXCoord(mouseX);
                bool shiftPressed = (SDL_GetModState() & SDL_KMOD_SHIFT) != 0;

                // Selection logic based on Shift key
                if (shiftPressed) {
                    if (m_selectionStart == -1) { // Anchor selection if not already started
                        m_selectionStart = m_cursorPos;
                    }
                } else {
                    // Non-shift click: set selection anchor to the clicked position (start/collapse selection)
                    m_selectionStart = clickedIndex;
                }
                m_cursorPos = clickedIndex;
                m_isDragging = true;

                // Reset cursor blink state
                m_isCursorVisible = true;
                m_cursorBlinkTimer = 0.0f;
                // Adjust scroll view to ensure the new cursor position is visible
                clampCursorAndScroll();

                // Update IME area using screen coordinates: viewOffset + content pos
                if (m_window) {
    SDL_FPoint useOffset = m_lastViewOffset; // Use stored offset for screen-space calculation
    SDL_Rect inputRect = {
        static_cast<int>(std::floor(useOffset.x + static_cast<float>(m_posX) + 0.5f)),
        static_cast<int>(std::floor(useOffset.y + static_cast<float>(m_posY) + 0.5f)),
        m_width,
        m_height
    };
    // Inform SDL of the new cursor position within the IME area
    SDL_SetTextInputArea(m_window, &inputRect, m_cursorPos);
    SDL_Log("InputBox::update IME rect=(%d,%d) cursor=%d", inputRect.x, inputRect.y, m_cursorPos);
}

            } else { // Clicked outside
                if (m_hasFocus) {
                    // Unfocus the control and stop SDL text input
                    SDL_Window* winToUse = window ? window : SDL_GetWindowFromID(event.button.windowID);
                    unfocus(winToUse);
                }
            }
        }
    } else if (event.type == SDL_EVENT_MOUSE_MOTION) {
        // If left mouse button is down and dragging flag is set
        if (m_isDragging && (event.motion.state & SDL_BUTTON_LMASK)) {
            // Mouse X is in content-space
            mouseX = static_cast<int>(event.motion.x);

            // Update cursor position based on mouse X, extending selection from m_selectionStart
            m_cursorPos = getIndexFromXCoord(mouseX);

            // Reset cursor blink state
            m_isCursorVisible = true;
            m_cursorBlinkTimer = 0.0f;
            // Scroll to keep the cursor visible
            clampCursorAndScroll(); // may update m_scrollX

            if (m_window) {
                // Compute the screen-space rectangle for IME update
                SDL_Rect inputRect = {
    static_cast<int>(std::floor(m_lastViewOffset.x + static_cast<float>(m_posX) + 0.5f)),
    static_cast<int>(std::floor(m_lastViewOffset.y + static_cast<float>(m_posY) + 0.5f)),
    m_width,
    m_height
};
                // Update the IME caret position using the correct window context
SDL_SetTextInputArea(m_forwardedWindow ? m_forwardedWindow : SDL_GetWindowFromID(event.button.windowID), &inputRect, m_cursorPos);

            }
        }
    } else if (event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
        if (event.button.button == SDL_BUTTON_LEFT) {
            // Stop dragging state
            m_isDragging = false;
            // Selection range (m_selectionStart and m_cursorPos) is retained for rendering
        }
    }
}


/**
 * @brief Ensures the cursor position is within text bounds and scrolls the text
 * horizontally to keep the cursor visible within the input box's visible area.
 */
void InputBox::clampCursorAndScroll() {
    // 1. Clamp cursor position: ensure it is between 0 and text.length()
    m_cursorPos = std::max(0, std::min(static_cast<int>(m_text.length()), m_cursorPos));

    // 2. Adjust scroll based on cursor position
    int visibleTextWidth = m_width - 2 * m_style.paddingX;
    if (visibleTextWidth < 0) visibleTextWidth = 0;

    // Get the pixel offset of the cursor from the start of the *entire* text string
    int cursorPixelX = getTextXPosition(m_cursorPos);

    // If cursor is to the left of the current scrolled view (m_scrollX)
    if (cursorPixelX < m_scrollX) {
        // Scroll left to bring the cursor to the left edge of the visible area
        m_scrollX = cursorPixelX;
    }
    // If cursor is to the right of the current scrolled view
    else if (cursorPixelX > m_scrollX + visibleTextWidth) {
        // Scroll right to bring the cursor to the right edge of the visible area
        m_scrollX = cursorPixelX - visibleTextWidth;
    }

    // 3. Final clamp of scroll position
    int totalTextWidth = getTextXPosition(static_cast<int>(m_text.length()));
    if (totalTextWidth <= visibleTextWidth) { // Text fits entirely
        m_scrollX = 0;
    } else {
        // Clamp m_scrollX between 0 and (totalTextWidth - visibleTextWidth)
        m_scrollX = std::max(0, std::min(m_scrollX, totalTextWidth - visibleTextWidth));
    }
}

/**
 * @brief Converts a content-space X coordinate to a character index within the text string.
 *
 * This function determines where a mouse click landed in relation to the characters
 * to correctly place the text cursor. It accounts for padding and current scroll offset.
 *
 * @param globalMouseX The X coordinate of the mouse click in content-space (relative to the container/window content origin).
 * @return The 0-based index of the character *after* which the cursor should be placed.
 */
int InputBox::getIndexFromXCoord(int globalMouseX) {
    if (!m_textRenderer.isInitialized()) return 0;

    // Calculate mouse X relative to the text's render origin (m_posX + paddingX) and adjusted for current scroll
    float relativeMouseX = static_cast<float>(globalMouseX - (m_posX + m_style.paddingX) + m_scrollX);

    if (relativeMouseX <= 0) return 0; // Clicked before the first character

    // Iterate through display text (m_displayText) to find the character boundary
    int totalTextWidth = 0;
    for (int i = 0; i < static_cast<int>(m_displayText.length()); ++i) {
        // Measure width of the substring up to the current character + 1 for boundary calculation
        std::string charStr = m_displayText.substr(i, 1);
        int charW = 0, charH = 0;
        m_textRenderer.measureText(charStr, m_fontSize, charW, charH);

        // Check if the click is in the first half or second half of the current character
        if (relativeMouseX < (totalTextWidth + charW / 2.0f)) {
            // Click is closer to the start of this character: place cursor before it (index i)
            return i;
        }
        totalTextWidth += charW;
        if (relativeMouseX < totalTextWidth) {
             // Click is closer to the end of this character: place cursor after it (index i + 1)
             return i + 1;
        }
    }

    // Clicked beyond the end of the last character
    if (relativeMouseX >= totalTextWidth) {
        return static_cast<int>(m_displayText.length());
    }

    return static_cast<int>(m_displayText.length()); // Fallback: end of text
}

/**
 * @brief Calculates the X pixel position of the cursor when placed immediately *after* a specific character index.
 *
 * This position is relative to the start of the *entire* text string (i.e., offset 0 for index 0).
 * It does not include padding or scroll offset.
 *
 * @param charIndex The character index (0 to length) to measure up to.
 * @return The width in pixels of the substring from the start of the text up to `charIndex`.
 */
int InputBox::getTextXPosition(int charIndex) const {
    if (!m_textRenderer.isInitialized() || m_displayText.empty() || charIndex <= 0) {
        return 0;
    }
    // Clamp charIndex to be within the bounds of m_displayText
    charIndex = std::max(0, std::min(static_cast<int>(m_displayText.length()), charIndex));

    // Measure the width of the substring up to the desired index
    std::string sub = m_displayText.substr(0, charIndex);
    int subW = 0, subH = 0;
    m_textRenderer.measureText(sub, m_fontSize, subW, subH);
    return subW;
}

/**
 * @brief Deletes the currently selected text range.
 *
 * Updates the internal text string, moves the cursor to the selection start, and clears the selection range.
 */
void InputBox::deleteSelection() {
    if (m_selectionStart == -1 || m_selectionStart == m_cursorPos) return; // No actual selection

    // Determine the start (selS) and end (selE) of the selection in text indices
    int selS = std::min(m_selectionStart, m_cursorPos);
    int selE = std::max(m_selectionStart, m_cursorPos);
    int length = selE - selS;

    if (length > 0 && selS < static_cast<int>(m_text.length())) {
        // Erase the substring
        m_text.erase(selS, length);
        // Move cursor to the position where deletion started
        m_cursorPos = selS;
        m_selectionStart = -1; // Selection is now deleted, clear the range
        // Note: Caller must update m_displayText and notify via onTextChanged
    }
}

/**
 * @brief Copies the currently selected text to the system clipboard.
 *
 * If the input box is in password mode, no action is taken.
 */
void InputBox::copySelectionToClipboard() {
    if (m_isPassword) return; // Do not allow copying password text

    std::string selectedText;
    if (m_selectionStart != -1 && m_selectionStart != m_cursorPos) {
        int selS = std::min(m_selectionStart, m_cursorPos);
        int selE = std::max(m_selectionStart, m_cursorPos);
        selectedText = m_text.substr(selS, selE - selS);
    }
    // Only copy if a range selection exists

    if (!selectedText.empty()) {
        if (SDL_SetClipboardText(selectedText.c_str()) != 0) {
            // Handle error (e.g., logging) if clipboard operation fails
            // SDL_Log("SDL_SetClipboardText error: %s", SDL_GetError());
        }
    }
}

/**
 * @brief Pastes text from the system clipboard into the input box at the current cursor position.
 *
 * If there is an existing selection, it is deleted before insertion. Max length is enforced.
 */
void InputBox::pasteFromClipboard() {
    if (!SDL_HasClipboardText()) return;

    char* clipboardChars = SDL_GetClipboardText();
    if (!clipboardChars) {
        // Handle error (e.g., logging) if reading clipboard fails
        // SDL_Log("SDL_GetClipboardText error: %s", SDL_GetError());
        return;
    }
    std::string textToPaste(clipboardChars);
    SDL_free(clipboardChars); // Free the memory allocated by SDL_GetClipboardText

    if (textToPaste.empty()) return;

    // Delete existing selection, if any
    if (m_selectionStart != -1 && m_selectionStart != m_cursorPos) {
        deleteSelection(); // Moves m_cursorPos to selStart and sets m_selectionStart = -1
    }

    size_t CharsToInsertCount = textToPaste.length();
    // Enforce maximum length constraint
    if (m_maxLength > 0) {
        if (m_text.length() >= static_cast<size_t>(m_maxLength)) {
            return; // Max length reached
        }
        if (m_text.length() + CharsToInsertCount > static_cast<size_t>(m_maxLength)) {
            // Trim the pasted text if it exceeds the max length
            CharsToInsertCount = m_maxLength - m_text.length();
            textToPaste = textToPaste.substr(0, CharsToInsertCount);
        }
    }

    if (CharsToInsertCount > 0) {
        // Insert the pasted text
        m_text.insert(m_cursorPos, textToPaste);
        // Advance cursor
        m_cursorPos += CharsToInsertCount;
        m_selectionStart = -1; // Pasting clears selection range

        // Note: Text changed, caller (handleKeyDown) will handle display update and notification.
    }
}

/**
 * @brief Cuts (copies and deletes) the selected text to the system clipboard.
 *
 * If the input box is in password mode, the text is only deleted, not copied.
 */
void InputBox::cutSelectionToClipboard() {
    if (m_isPassword) {
        // If password, just delete the selection if one exists
        if (m_selectionStart != -1 && m_selectionStart != m_cursorPos) {
            deleteSelection();
            // Note: Text changed, caller will handle display update and notification.
        }
        return;
    }

    if (m_selectionStart != -1 && m_selectionStart != m_cursorPos) {
        copySelectionToClipboard(); // Copy the selection
        deleteSelection();          // Then delete the selection
        // Note: Text changed, caller will handle display update and notification.
    }
    // else: No selection to cut, do nothing.
}


// Getters/Setters
/**
 * @brief Gets the actual text content of the input box.
 * @return The current text string.
 */
std::string InputBox::getText() const {
    return m_text;
}

/**
 * @brief Sets the text content of the input box, enforcing max length.
 *
 * Resets cursor, selection, and scroll position. Triggers display update and change notification.
 *
 * @param text The new text string to set.
 */
void InputBox::setText(const std::string& text) {
    // Enforce max length on the new text string
    if (m_maxLength > 0 && text.length() > static_cast<size_t>(m_maxLength)) {
        m_text = text.substr(0, m_maxLength);
    } else {
        m_text = text;
    }

    // Reset state after setting new text
    m_cursorPos = static_cast<int>(m_text.length()); // Move cursor to end
    m_selectionStart = -1;                        // Reset selection
    m_scrollX = 0;                                // Reset scroll

    updateDisplayText(); // Essential to update m_displayText for rendering and calculations
    clampCursorAndScroll(); // Ensure cursor and scroll are valid for new text

    if (onTextChanged) {
        onTextChanged(m_text); // Notify about the change
    }

    // If the box is already focused, update SDL's IME state (position and cursor)
    if (m_hasFocus && m_window) {
    SDL_FPoint useOffset = m_lastViewOffset;
    SDL_Rect inputRect = {
        static_cast<int>(std::floor(useOffset.x + static_cast<float>(m_posX) + 0.5f)),
        static_cast<int>(std::floor(useOffset.y + static_cast<float>(m_posY) + 0.5f)),
        m_width,
        m_height
    };
    // Update IME position and cursor position
    SDL_SetTextInputArea(m_window, &inputRect, m_cursorPos);
}
}

/**
 * @brief Sets the maximum allowed length for the text content.
 *
 * If the current text exceeds the new limit, it is truncated.
 *
 * @param maxLength The new maximum length (0 means no limit).
 */
void InputBox::setMaxLength(int maxLength) {
    m_maxLength = std::max(0, maxLength);
    // If current text exceeds new max length, truncate it
    if (m_maxLength > 0 && m_text.length() > static_cast<size_t>(m_maxLength)) {
        m_text = m_text.substr(0, m_maxLength);
        updateDisplayText();
        clampCursorAndScroll(); // Cursor might now be out of bounds

        if (onTextChanged) {
            onTextChanged(m_text);
        }

        // If focused, update SDL's IME state after truncation
        if (m_hasFocus && m_window) {
    SDL_FPoint useOffset = m_lastViewOffset;
    SDL_Rect inputRect = {
        static_cast<int>(std::floor(useOffset.x + static_cast<float>(m_posX) + 0.5f)),
        static_cast<int>(std::floor(useOffset.y + static_cast<float>(m_posY) + 0.5f)),
        m_width,
        m_height
    };
    SDL_SetTextInputArea(m_window, &inputRect, m_cursorPos);
}
    }
}


} // namespace XenUI




