#ifndef XENUI_INPUTBOX_H
#define XENUI_INPUTBOX_H
// SPDX-License-Identifier: Apache-2.0
/*
 *  Copyright (C) 2025 MD S M Sarowar Hossain
 */
//
//

// --- Standard Library and SDL Includes ---
#include "Position.h"     // Defines PositionParams and helper functions like CalculateFinalPosition
#include "TextRenderer.h" // Interface for rendering and measuring text
#include <string>         // For std::string manipulation
#include <functional>     // For std::function (event callbacks)
#include <algorithm>      // For std::min/max
#include <vector>         // For potential future use (e.g., undo/redo history)
#include <SDL3/SDL.h>
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_clipboard.h> // For clipboard functions
#include "UIElement.h"    // Base interface for UI controls (IControl)
#include <cmath>          // For floating point math in layout calculations

namespace XenUI {

/**
 * @brief Defines the visual style and dimensions for an InputBox.
 *
 * This structure holds color schemes, padding, and border drawing flags.
 */
struct InputBoxStyle {
    SDL_Color bgColor = { 40, 40, 40, 255 };             ///< Background color of the input box.
    SDL_Color textColor = { 230, 230, 230, 255 };         ///< Color of the displayed text.
    SDL_Color borderColor = { 80, 200, 80, 255 };         ///< Color of the border when unfocused.
    SDL_Color cursorColor = { 240, 240, 240, 255 };       ///< Color of the text insertion cursor (caret).
    SDL_Color selectionBgColor = { 70, 100, 130, 150 };   ///< Background color for selected text.
    SDL_Color focusedBorderColor = { 100, 150, 255, 255 }; ///< Color of the border when focused.
    int paddingX = 5;                                   ///< Horizontal padding (left/right) between border and text area.
    int paddingY = 3;                                   ///< Vertical padding (top/bottom) between border and text area.
    bool drawBackground = true;                         ///< Flag to enable/disable background drawing.
    bool drawBorder = true;                             ///< Flag to enable/disable border drawing.
};

const int DEFAULT_INPUT_FONT_SIZE = 16; ///< Default point size for the input box font.

/**
 * @brief A retained-mode UI control for standard text input.
 *
 * This class handles text editing, cursor movement, selection, scrolling,
 * password masking, and SDL event integration (mouse/keyboard/IME).
 *
 * NOTE: This is a **Retained Mode** control, inheriting from IControl.
 */
class InputBox : public IControl {
public:
    /**
     * @brief Constructor for the InputBox control.
     *
     * @param posParams Parameters for resolving the control's position relative to its parent/window.
     * @param initialText The text string to initially populate the box with.
     * @param width The fixed width of the input box in pixels.
     * @param fontSize The font size for the text.
     * @param style The visual style to apply to the box.
     * @param isPassword If true, the input text is masked (e.g., replaced by '*').
     */
    InputBox(const PositionParams& posParams,
             const std::string& initialText = "",
             int width = 200,
             int fontSize = DEFAULT_INPUT_FONT_SIZE,
             InputBoxStyle style = InputBoxStyle(),
             bool isPassword = false);

    // IControl virtual function implementations (declared in UIElement.h)
    // The InputBox is a retained mode control.
    // The following methods implement the IControl interface for update and query.

    /**
     * @brief Updates the control's state over time. Primarily handles cursor blinking.
     *
     * @param deltaTime The time elapsed since the last update call (in seconds).
     * @return true if the control's visual state changed (needs redraw), false otherwise.
     */
    bool update(float deltaTime);

    /**
     * @brief Indicates if the control requires continuous updates/redrawing.
     *
     * @return true if the control has focus (due to cursor blinking), false otherwise.
     */
    bool isAnimating() const;

    // --- Public API for Text Access/Modification ---

    /**
     * @brief Retrieves the actual text content of the input box.
     * @return The current text string.
     */
    std::string getText() const;

    /**
     * @brief Sets the text content, enforcing max length, and resetting cursor/selection.
     * @param text The new text string to set.
     */
    void setText(const std::string& text);

    // --- Focus Management ---

    /**
     * @brief Sets focus to this control, initiating SDL text input and setting the IME area.
     *
     * @param window The SDL_Window context.
     * @param viewOffset The parent's content-to-screen translation offset.
     */
    void focus(SDL_Window* window, const SDL_FPoint& viewOffset);

    /**
     * @brief Sets focus using the last known view offset. (Overrides IControl::focus).
     * @param window The SDL_Window context.
     */
    void focus(SDL_Window* window) override;

    /**
     * @brief Removes focus and stops SDL text input. (Overrides IControl::unfocus).
     * @param window The SDL_Window context.
     */
    void unfocus(SDL_Window* window) override;

    /**
     * @brief Checks if the control currently holds focus.
     * @return true if focused, false otherwise.
     */
    bool hasFocus() const { return m_hasFocus; }

    // --- Constraints ---

    /**
     * @brief Sets the maximum character length for the input text.
     *
     * @param maxLength The maximum length (0 means no limit).
     */
    void setMaxLength(int maxLength);

    /**
     * @brief Gets the maximum character length constraint.
     * @return The maximum length (0 if unlimited).
     */
    int getMaxLength() const { return m_maxLength; }

    // --- Callbacks ---

    std::function<void(const std::string&)> onTextChanged;   ///< Callback invoked when the text content changes.
    std::function<void(const std::string&)> onEnterPressed;  ///< Callback invoked when the Enter key is pressed.

    // --- IControl Interface Implementations ---

    /**
     * @brief Handles incoming SDL events, routing them to specialized handlers. (Overrides IControl::handleEvent).
     * @param event The SDL_Event to process.
     * @return true if the event resulted in a state change, false otherwise.
     */
    bool handleEvent(const SDL_Event& event) override;

    /**
     * @brief Renders the input box, including background, border, text, selection, and cursor. (Overrides IControl::draw).
     * @param renderer The SDL_Renderer context.
     * @param viewOffset The offset applied by a parent container (e.g., ScrollView) to content-space coordinates.
     */
    void draw(SDL_Renderer* renderer, const SDL_FPoint& viewOffset) override;

    /**
     * @brief Recalculates the control's absolute position and height based on parent dimensions. (Overrides IControl::recalculateLayout).
     * @param parentWidth The width of the parent container.
     * @param parentHeight The height of the parent container.
     */
    void recalculateLayout(int parentWidth, int parentHeight) override; // computes m_posX/m_posY and final w/h in content-space

    /**
     * @brief Convenience overload for recalculateLayout when parent dimensions are the window size.
     *
     * Retrieves window size and calls the two-argument recalculateLayout.
     */
    void recalculateLayout(){
        SDL_Point s = XenUI::GetWindowSize();
        recalculateLayout(s.x, s.y);
    }

    /**
     * @brief Returns the bounding box of the control in content-space. (Overrides IControl::getBounds).
     * @return The SDL_FRect representing position and size.
     */
    virtual SDL_FRect getBounds() const;

    // --- Convenience Overloads and Context Setting ---

    /**
     * @brief Full event handler including context window and view offset. (Not an override).
     * @param event The SDL_Event to process.
     * @param window The window context.
     * @param viewOffset The content-to-screen offset.
     * @return true if the event was handled.
     */
    bool handleEvent(const SDL_Event& event, SDL_Window* window, const SDL_FPoint& viewOffset);

    /**
     * @brief Backwards-compatible draw without view offset (assumes {0,0}). (Not an override).
     * @param renderer The SDL_Renderer context.
     */
    void draw(SDL_Renderer* renderer);

    /**
     * @brief Recalculates position based on m_posParams and window size. (Not an override).
     */
    virtual void recalculatePosition();

    /**
     * @brief Sets the stored SDL_Window context. (Overrides IControl::setWindow).
     * @param window The window pointer.
     */
    void setWindow(SDL_Window* window) override;

    /**
     * @brief Sets the stored view offset (used for IME area calculation). (Overrides IControl::setViewOffset).
     * @param viewOffset The content-to-screen offset.
     */
    void setViewOffset(const SDL_FPoint& viewOffset) override;


private:
    // --- Configuration and Text State ---
    PositionParams m_posParams;          ///< Parameters defining layout position (anchors, offsets).
    std::string m_text;                  ///< The actual, unmasked text content.
    std::string m_displayText;           ///< The text used for rendering (m_text or '*' for password mode).
    InputBoxStyle m_style;               ///< Visual style of the input box.
    int m_fontSize;                      ///< Font size in points.
    bool m_isPassword;                   ///< Flag indicating if content should be masked.
    int m_maxLength = 0;                 ///< Maximum allowed length of m_text (0 for unlimited).

    // --- Layout State (Content-Space) ---
    int m_posX = 0, m_posY = 0;          ///< Top-left position in content-space (calculated by recalculateLayout).
    int m_width = 0;                     ///< Width of the control.
    int m_height = 0;                    ///< Height of the control (calculated by calculateHeight).

    // --- Editing State ---
    //bool m_hasFocus = false;           // Inherited from IControl
    int m_cursorPos = 0;                 ///< Character index where the cursor is placed (0 to length).
    int m_selectionStart = -1;           ///< Anchor for selection (-1 if no selection range).
    bool m_isDragging = false;           ///< Flag indicating if the mouse is currently dragging to select.

    // --- Context and Context Forwarding ---
    SDL_Window* m_window = nullptr;      ///< Store the window for SDL API calls (e.g., SDL_StartTextInput).
    SDL_Window* m_forwardedWindow = nullptr; ///< Stored window context from the last setWindow call.
    SDL_FPoint  m_lastViewOffset = {0.0f, 0.0f}; ///< Stored view offset from the last setViewOffset call.

    // --- Rendering & Scrolling State ---
    int m_scrollX = 0;                   ///< Horizontal pixel offset of the text, used for scrolling long text.
    bool m_isCursorVisible = true;       ///< Current visibility state of the cursor (blinking).
    float m_cursorBlinkTimer = 0.0f;     ///< Timer for tracking cursor blink cycle.
    static constexpr float CURSOR_BLINK_RATE = 0.53f; ///< Time in seconds for one half of the blink cycle.

    TextRenderer& m_textRenderer;        ///< Reference to the singleton text rendering utility.


    // --- Internal Logic Helpers ---
    void updateDisplayText();            ///< Updates m_displayText based on m_text and m_isPassword.
    void calculateHeight();              ///< Calculates m_height based on m_fontSize and padding.
    void clampCursorAndScroll();         ///< Ensures cursor is in bounds and scrolls the view to keep it visible.
    int getIndexFromXCoord(int globalMouseX); ///< Maps a content-space X pixel coordinate to a character index.
    int getTextXPosition(int charIndex) const; ///< Returns the pixel width of the text up to charIndex.
    void deleteSelection();              ///< Deletes the text between m_selectionStart and m_cursorPos.

    // --- Event Handler Components ---
    void handleKeyDown(SDL_Keycode key, SDL_Keymod mod); ///< Processes special keys (arrows, backspace, shortcuts).
    void handleTextInput(const char* inputText);         ///< Inserts composed text from keyboard/IME.
    void handleMouseInput(const SDL_Event& event, SDL_Window* window, const SDL_FPoint& viewOffset); ///< Processes mouse down/up/motion events.

    // --- Clipboard Helpers ---
    void copySelectionToClipboard();     ///< Copies selected text to clipboard.
    void pasteFromClipboard();           ///< Pastes text from clipboard.
    void cutSelectionToClipboard();      ///< Cuts selected text to clipboard.
};

} // namespace XenUI
#endif // XENUI_INPUTBOX_H