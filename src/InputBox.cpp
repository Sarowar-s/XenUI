


// SPDX-License-Identifier: Apache-2.0
/*
 *  Copyright (C) 2025 MD S M Sarowar Hossain
 */
//
// Implementation of the InputBox retained-mode UI control, managing text input,
// cursor blinking, scrolling, and rendering.
//
#include "InputBox.h"
#include <algorithm> 
// #include <iostream>


namespace XenUI {


static bool isPrimaryModifier(SDL_Keymod mod) {
    return (mod & (SDL_KMOD_LCTRL | SDL_KMOD_RCTRL | SDL_KMOD_LGUI | SDL_KMOD_RGUI)) != 0;

}


InputBox::InputBox(const PositionParams& posParams, const std::string& initialText,
                   int width, int fontSize, InputBoxStyle style, bool isPassword)
    : m_posParams(posParams),
      m_text(""), 
      m_style(style),
      m_fontSize(fontSize > 0 ? fontSize : DEFAULT_INPUT_FONT_SIZE),
      m_isPassword(isPassword),
      m_width(width > 0 ? width : 200),
      m_textRenderer(TextRenderer::getInstance()),
      m_window(nullptr)
{
    calculateHeight();    
    setText(initialText);  
    recalculatePosition(); 
                         
}



bool InputBox::handleEvent(const SDL_Event& event) {
    SDL_Window* win = m_forwardedWindow ? m_forwardedWindow : m_window;
    return handleEvent(event, win, m_lastViewOffset);
}

void InputBox::draw(SDL_Renderer* renderer) {
    draw(renderer, SDL_FPoint{0.0f, 0.0f});
}

// // Backwards-compat recalc name
// void InputBox::recalculatePosition() {
// //     recalculateLayout();
// // }


void InputBox::updateDisplayText() {
    if (m_isPassword) {
        // Use '*' for every character if password mode is active
        m_displayText = std::string(m_text.length(), '*');
    } else {
        // Otherwise, the display text is the same as the actual text
        m_displayText = m_text;
    }
}


void InputBox::calculateHeight() {
  
    if (!m_textRenderer.isInitialized()) {
        m_height = (m_fontSize > 0 ? m_fontSize : 16) + 2 * m_style.paddingY;
        return;
    }
    int text_w, text_h;
   
    m_textRenderer.measureText("M", m_fontSize, text_w, text_h);
   
    m_height = text_h + 2 * m_style.paddingY;
   
    if (m_height < (m_fontSize + 2 * m_style.paddingY)) {
        m_height = m_fontSize + 2 * m_style.paddingY;
    }
}


void InputBox::recalculatePosition() {
   
    SDL_Point finalPos = XenUI::CalculateFinalPosition(m_posParams, m_width, m_height);
    m_posX = finalPos.x;
    m_posY = finalPos.y;
}


bool XenUI::InputBox::update(float deltaTime) {
    bool changed = false;

    
    if (IControl::hasFocus()) {
        m_cursorBlinkTimer += deltaTime;
        if (m_cursorBlinkTimer >= CURSOR_BLINK_RATE) {
          
            m_isCursorVisible = !m_isCursorVisible;
           
            m_cursorBlinkTimer -= CURSOR_BLINK_RATE
                              * static_cast<int>(m_cursorBlinkTimer / CURSOR_BLINK_RATE);
            changed = true;
        }
    } else {
       
        if (m_isCursorVisible) {
            m_isCursorVisible = false;
            changed = true;
        }
    }

    return changed;
}

bool XenUI::InputBox::isAnimating() const {
        
        return IControl::hasFocus();
    }


void InputBox::recalculateLayout(int parentWidth, int parentHeight) {
   
    calculateHeight();

    
    SDL_Point finalPos = XenUI::CalculateFinalPosition(m_posParams, m_width, m_height, parentWidth, parentHeight);
    m_posX = finalPos.x;
    m_posY = finalPos.y;

  
    clampCursorAndScroll();
}

SDL_FRect InputBox::getBounds() const {
    return SDL_FRect{ float(m_posX), float(m_posY), float(m_width), float(m_height) };
}

void InputBox::draw(SDL_Renderer* renderer, const SDL_FPoint& viewOffset) {
    if (!renderer) return;

   
    float absX = viewOffset.x + float(m_posX);
    float absY = viewOffset.y + float(m_posY);

    SDL_FRect boxRect = {
        absX,
        absY,
        (float)m_width,
        (float)m_height
    };

    //  Draw Background
    if (m_style.drawBackground) {
        SDL_SetRenderDrawColor(renderer,
            m_style.bgColor.r, m_style.bgColor.g, m_style.bgColor.b, m_style.bgColor.a);
        SDL_RenderFillRect(renderer, &boxRect);
    }

    //  Draw Border
    if (m_style.drawBorder) {
        SDL_Color borderColor = m_hasFocus ? m_style.focusedBorderColor : m_style.borderColor;
        SDL_SetRenderDrawColor(renderer, borderColor.r, borderColor.g, borderColor.b, borderColor.a);
        SDL_RenderRect(renderer, &boxRect);
    }

   
    SDL_FRect innerRect = {
        absX + (float)m_style.paddingX,
        absY + (float)m_style.paddingY,
        (float)(m_width - 2 * m_style.paddingX),
        (float)(m_height - 2 * m_style.paddingY)
    };
    if (innerRect.w < 1.0f) innerRect.w = 1.0f;
    if (innerRect.h < 1.0f) innerRect.h = 1.0f;

    

    
    SDL_Rect oldClip;
    bool hadOldClip = SDL_RenderClipEnabled(renderer);
    if (hadOldClip) SDL_GetRenderClipRect(renderer, &oldClip);

    
    SDL_Rect innerClip = {
        static_cast<int>(std::floor(innerRect.x + 0.5f)),
        static_cast<int>(std::floor(innerRect.y + 0.5f)),
        static_cast<int>(std::floor(innerRect.w + 0.5f)),
        static_cast<int>(std::floor(innerRect.h + 0.5f))
    };

    SDL_Rect finalClip = innerClip;
    if (hadOldClip) {
       
        int x1 = std::max(oldClip.x, innerClip.x);
        int y1 = std::max(oldClip.y, innerClip.y);
        int x2 = std::min(oldClip.x + oldClip.w, innerClip.x + innerClip.w);
        int y2 = std::min(oldClip.y + oldClip.h, innerClip.y + innerClip.h);
        if (x2 <= x1 || y2 <= y1) {
           
            finalClip = {0,0,0,0};
        } else {
           
            finalClip = { x1, y1, x2 - x1, y2 - y1 };
        }
    }

   
    SDL_SetRenderClipRect(renderer, &finalClip);

    
    float textRenderBaseX = innerRect.x - (float)m_scrollX;

    
    if (m_selectionStart != -1 && m_selectionStart != m_cursorPos) {
        int selS = std::min(m_selectionStart, m_cursorPos);
        int selE = std::max(m_selectionStart, m_cursorPos);

       
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

    //  Draw text (using TextRenderer caching)
    if (m_textRenderer.isInitialized() && !m_displayText.empty()) {
        int textTextureW = 0, textTextureH = 0;
        // Request the text texture from the cache or generate it
        SDL_Texture* textTexture = m_textRenderer.getInstance().renderTextToTexture(
            m_displayText, m_style.textColor, m_fontSize, textTextureW, textTextureH);

        if (textTexture) {
          
            float textRenderH = static_cast<float>(textTextureH);
            float textRenderY = innerRect.y + (innerRect.h - textRenderH) / 2.0f;
            if (textTextureH > static_cast<int>(innerRect.h)) {
               
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

           
#ifdef INPUTBOX_DEBUG_RECT
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 128); 
            SDL_RenderRect(renderer, &textDestRect);
#endif

           
        }
    }

  
    if (m_hasFocus && m_isCursorVisible) {
        
        float cursorScreenX = textRenderBaseX + static_cast<float>(getTextXPosition(m_cursorPos));
        SDL_SetRenderDrawColor(renderer,
            m_style.cursorColor.r, m_style.cursorColor.g, m_style.cursorColor.b, m_style.cursorColor.a);

        
        SDL_RenderLine(renderer, cursorScreenX, innerRect.y, cursorScreenX, innerRect.y + innerRect.h - 1.0f);
    }

   
    
    if (hadOldClip) SDL_SetRenderClipRect(renderer, &oldClip);
    else SDL_SetRenderClipRect(renderer, nullptr);


#ifdef INPUTBOX_DEBUG_RECT
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 96); // translucent green
    SDL_RenderRect(renderer, &textDestRect);
#endif

}

// ScrollView::handleEvent(event, window, viewOffset)


bool InputBox::handleEvent(const SDL_Event& event, SDL_Window* window, const SDL_FPoint& viewOffset) {
    bool changed = false;

    
    if (window) m_window = window;

        float mx = 0.f, my = 0.f;
    bool haveMouseCoords = false;
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

       auto pointInside = [&](float x, float y) {
        return (x >= left && x <= right && y >= top && y <= bottom);
    };

       if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button == SDL_BUTTON_LEFT && haveMouseCoords) {
        if (pointInside(mx, my)) {
                focus(window, viewOffset);

            m_cursorPos = getIndexFromXCoord(static_cast<int>(mx));
            m_selectionStart = m_cursorPos;
            m_isDragging = true;
            changed = true;

          
            if (m_window) {
                SDL_FPoint useOffset = viewOffset; 

               
                SDL_Rect inputRect = {
                    static_cast<int>(std::floor(useOffset.x + static_cast<float>(m_posX) + 0.5f)),
                    static_cast<int>(std::floor(useOffset.y + static_cast<float>(m_posY) + 0.5f)),
                    m_width,
                    m_height
                };
              
                SDL_SetTextInputArea(m_window, &inputRect, m_cursorPos);
                SDL_Log("InputBox::focus rect=(%d,%d,%d,%d) cursor=%d useOffset=(%.1f,%.1f) window=%p",
                        inputRect.x, inputRect.y, inputRect.w, inputRect.h, m_cursorPos,
                        useOffset.x, useOffset.y, (void*)m_window);
            }

        }
//          else {
//     
//     if (m_hasFocus) {
//     
//         m_hasFocus = false;
//         m_isDragging = false;
//         m_selectionStart = -1;

//        
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
           
            changed = true;

           
            if (m_window) {
               
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

    // Keyboard & text input when focused
    if (m_hasFocus) {
        if (event.type == SDL_EVENT_TEXT_INPUT) {
           
            handleTextInput(event.text.text);
            changed = true;
        } else if (event.type == SDL_EVENT_KEY_DOWN) {
          
            handleKeyDown(event.key.key, static_cast<SDL_Keymod>(event.key.mod));
            changed = true;
        }else if (event.type == SDL_EVENT_TEXT_EDITING) {
           
            SDL_Log(
                "App: SDL_EVENT_TEXT_EDITING --- text: '%s', start: %d, length: %d",
                event.edit.text ? event.edit.text : "NULL",
                event.edit.start,
                event.edit.length
            );
        }
       
    }

   
    clampCursorAndScroll();

    return changed;
}




void InputBox::setWindow(SDL_Window* window) {
    m_forwardedWindow = window;
   
    if (IControl::hasFocus() && window) m_window = window;
}


void InputBox::setViewOffset(const SDL_FPoint& viewOffset) {
    m_lastViewOffset = viewOffset;
   
    if (IControl::hasFocus() && m_window) {
      
        SDL_Rect inputRect = {
            static_cast<int>(std::floor(m_lastViewOffset.x + static_cast<float>(m_posX) + 0.5f)),
            static_cast<int>(std::floor(m_lastViewOffset.y + static_cast<float>(m_posY) + 0.5f)),
            m_width,
            m_height
        };
     
        SDL_SetTextInputArea(m_window, &inputRect, m_cursorPos);
    }
}



void InputBox::focus(SDL_Window* window) {
   
    focus(window, m_lastViewOffset);
}


void InputBox::focus(SDL_Window* window, const SDL_FPoint& viewOffset) {
   
    if (IControl::hasFocus()) {
        if (window) m_window = window;
        m_lastViewOffset = viewOffset;
        if (m_window) {
          
            SDL_Rect inputRect = {
                static_cast<int>(std::floor(m_lastViewOffset.x + static_cast<float>(m_posX) + 0.5f)),
                static_cast<int>(std::floor(m_lastViewOffset.y + static_cast<float>(m_posY) + 0.5f)),
                m_width,
                m_height
            };
          
            SDL_SetTextInputArea(m_window, &inputRect, m_cursorPos);
			SDL_StopTextInput(m_window);
            SDL_StartTextInput(m_window);
        }
        return;
    }

   
    IControl::focus(window);

  
    if (window) {
        m_forwardedWindow = window;
        m_window = window;
    }
    m_lastViewOffset = viewOffset;

  
    m_isCursorVisible = true;
    m_cursorBlinkTimer = 0.0f;

   
    if (m_window) {
        SDL_StartTextInput(m_window);
    } else {
       
        SDL_StartTextInput(nullptr);
    }

   
    clampCursorAndScroll();

    
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

   
    SDL_Log("InputBox::focus rect=(%d,%d,%d,%d) cursor=%d offset=(%.1f,%.1f) window=%p",
            inputRect.x, inputRect.y, inputRect.w, inputRect.h, m_cursorPos,
            m_lastViewOffset.x, m_lastViewOffset.y, (void*)m_window);
}




void InputBox::unfocus(SDL_Window* window) {
    if (!IControl::hasFocus()) return;

    
    m_selectionStart = -1;
    
    SDL_Window* w = window ? window : m_window;
    if (w) SDL_StopTextInput(w);
    else SDL_StopTextInput(nullptr); 

   
    IControl::unfocus(window);
   
    m_window = nullptr;
}




void InputBox::handleKeyDown(SDL_Keycode key, SDL_Keymod mod) {
   
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
               
                if (m_cursorPos > 0) m_cursorPos--;
                // Collapse selection if shift was not pressed
                if (!shiftPressed && m_selectionStart != -1) m_selectionStart = m_cursorPos;
                selectionChangedByKey = true;
                break;

            case SDLK_RIGHT:
                // Handle selection anchoring if Shift is pressed
                if (shiftPressed) {
                    if (m_selectionStart == -1 || oldSelectionStart == oldCursorPos) {
                       
                        m_selectionStart = oldCursorPos;
                    }
                } else {
                    m_selectionStart = -1;
                }
                // Move cursor right
                if (m_cursorPos < static_cast<int>(m_text.length())) m_cursorPos++;
               
                if (!shiftPressed && m_selectionStart != -1) m_selectionStart = m_cursorPos;
                selectionChangedByKey = true;
                break;

            case SDLK_HOME:
               
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
               
                break;
        }
    }

   
    if (textChanged) {
        updateDisplayText();
        if (onTextChanged) onTextChanged(m_text);
    }

   
    if (selectionChangedByKey && !shiftPressed && (key == SDLK_LEFT || key == SDLK_RIGHT || key == SDLK_HOME || key == SDLK_END)) {
        m_selectionStart = -1;
    }

   
    clampCursorAndScroll();

   
    if (m_window) {
        SDL_FPoint useOffset = m_lastViewOffset;
        SDL_Rect inputRect = {
            static_cast<int>(std::floor(useOffset.x + static_cast<float>(m_posX) + 0.5f)),
            static_cast<int>(std::floor(useOffset.y + static_cast<float>(m_posY) + 0.5f)),
            m_width,
            m_height
        };
      
        SDL_SetTextInputArea(m_window, &inputRect, m_cursorPos);
        SDL_Log("InputBox::update IME rect=(%d,%d) cursor=%d", inputRect.x, inputRect.y, m_cursorPos);
    }

    if (key == SDLK_BACKSPACE) { // Log state after processing for Backspace debug
        SDL_Log("InputBox: Backspace KEYDOWN processed. Final m_text: '%s', cursor: %d, scrollX: %d", m_text.c_str(), m_cursorPos, m_scrollX);
    }
}



void InputBox::handleTextInput(const char* inputText) {
    SDL_Log("InputBox: TEXT_INPUT event. inputText: '%s', m_text_before: '%s', cursor_before: %d", inputText ? inputText : "NULL", m_text.c_str(), m_cursorPos);

    if (!m_hasFocus || !inputText || inputText[0] == '\0') return;

    m_isCursorVisible = true;
    m_cursorBlinkTimer = 0.0f;

    
    if (m_selectionStart != -1 && m_selectionStart != m_cursorPos) {
        deleteSelection();
    }

    std::string textToInsert(inputText);
    size_t CharsToInsertCount = textToInsert.length(); 

   
    if (m_maxLength > 0) {
        if (m_text.length() >= static_cast<size_t>(m_maxLength)) {
            return; 
        }
        if (m_text.length() + CharsToInsertCount > static_cast<size_t>(m_maxLength)) {
            
            CharsToInsertCount = m_maxLength - m_text.length();
            textToInsert = textToInsert.substr(0, CharsToInsertCount);
        }
    }

    if (CharsToInsertCount > 0) {
        
        m_text.insert(m_cursorPos, textToInsert);
       
        m_cursorPos += CharsToInsertCount;
        m_selectionStart = -1; 

        updateDisplayText();
       
        if (onTextChanged) {
            onTextChanged(m_text);
        }
    }

    
    clampCursorAndScroll();

    
    if (m_window) {
    SDL_FPoint useOffset = m_lastViewOffset;
    SDL_Rect inputRect = {
        static_cast<int>(std::floor(useOffset.x + static_cast<float>(m_posX) + 0.5f)),
        static_cast<int>(std::floor(useOffset.y + static_cast<float>(m_posY) + 0.5f)),
        m_width,
        m_height
    };
    
    SDL_SetTextInputArea(m_window, &inputRect, m_cursorPos);
    SDL_Log("InputBox::update IME rect=(%d,%d) cursor=%d", inputRect.x, inputRect.y, m_cursorPos);
}

    SDL_Log("InputBox: TEXT_INPUT processed. Final m_text: '%s', cursor_after: %d, scrollX: %d", m_text.c_str(), m_cursorPos, m_scrollX);

}






void InputBox::handleMouseInput(const SDL_Event& event, SDL_Window* window, const SDL_FPoint& viewOffset) {
    int mouseX = 0;
    int mouseY = 0;


    if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        
        mouseX = static_cast<int>(event.button.x);
        mouseY = static_cast<int>(event.button.y);
       
        bool inside = (mouseX >= m_posX && mouseX <= (m_posX + m_width) &&
                       mouseY >= m_posY && mouseY <= (m_posY + m_height));

       
        SDL_Window* currentWindow = window ? window : SDL_GetWindowFromID(event.button.windowID);

        if (event.button.button == SDL_BUTTON_LEFT) {
            
            if (!hasFocus()) {
                focus(currentWindow, viewOffset); 
            }

            if (inside) {
                if (!m_hasFocus) {
                   
                    focus(currentWindow, viewOffset);
                } else if (!m_window) { 
                    m_window = currentWindow;
                }

               
                int clickedIndex = getIndexFromXCoord(mouseX);
                bool shiftPressed = (SDL_GetModState() & SDL_KMOD_SHIFT) != 0;

                
                if (shiftPressed) {
                    if (m_selectionStart == -1) { 
                        m_selectionStart = m_cursorPos;
                    }
                } else {
                    
                    m_selectionStart = clickedIndex;
                }
                m_cursorPos = clickedIndex;
                m_isDragging = true;

                
                m_isCursorVisible = true;
                m_cursorBlinkTimer = 0.0f;
                
                clampCursorAndScroll();

                
                if (m_window) {
    SDL_FPoint useOffset = m_lastViewOffset;
    SDL_Rect inputRect = {
        static_cast<int>(std::floor(useOffset.x + static_cast<float>(m_posX) + 0.5f)),
        static_cast<int>(std::floor(useOffset.y + static_cast<float>(m_posY) + 0.5f)),
        m_width,
        m_height
    };
   
    SDL_SetTextInputArea(m_window, &inputRect, m_cursorPos);
    SDL_Log("InputBox::update IME rect=(%d,%d) cursor=%d", inputRect.x, inputRect.y, m_cursorPos);
}

            } else {
                if (m_hasFocus) {
                    
                    SDL_Window* winToUse = window ? window : SDL_GetWindowFromID(event.button.windowID);
                    unfocus(winToUse);
                }
            }
        }
    } else if (event.type == SDL_EVENT_MOUSE_MOTION) {
       
        if (m_isDragging && (event.motion.state & SDL_BUTTON_LMASK)) {
           
            mouseX = static_cast<int>(event.motion.x);

          
            m_cursorPos = getIndexFromXCoord(mouseX);

           
            m_isCursorVisible = true;
            m_cursorBlinkTimer = 0.0f;
           
            clampCursorAndScroll(); 

            if (m_window) {
                
                SDL_Rect inputRect = {
    static_cast<int>(std::floor(m_lastViewOffset.x + static_cast<float>(m_posX) + 0.5f)),
    static_cast<int>(std::floor(m_lastViewOffset.y + static_cast<float>(m_posY) + 0.5f)),
    m_width,
    m_height
};
               
SDL_SetTextInputArea(m_forwardedWindow ? m_forwardedWindow : SDL_GetWindowFromID(event.button.windowID), &inputRect, m_cursorPos);

            }
        }
    } else if (event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
        if (event.button.button == SDL_BUTTON_LEFT) {
           
            m_isDragging = false;
            // Selection range (m_selectionStart and m_cursorPos) is retained for rendering
        }
    }
}



void InputBox::clampCursorAndScroll() {
   
    m_cursorPos = std::max(0, std::min(static_cast<int>(m_text.length()), m_cursorPos));

  
    int visibleTextWidth = m_width - 2 * m_style.paddingX;
    if (visibleTextWidth < 0) visibleTextWidth = 0;

 
    int cursorPixelX = getTextXPosition(m_cursorPos);

   
    if (cursorPixelX < m_scrollX) {
      
        m_scrollX = cursorPixelX;
    }
    
    else if (cursorPixelX > m_scrollX + visibleTextWidth) {
       
        m_scrollX = cursorPixelX - visibleTextWidth;
    }

   
    int totalTextWidth = getTextXPosition(static_cast<int>(m_text.length()));
    if (totalTextWidth <= visibleTextWidth) { // Text fits entirely
        m_scrollX = 0;
    } else {
       
        m_scrollX = std::max(0, std::min(m_scrollX, totalTextWidth - visibleTextWidth));
    }
}


int InputBox::getIndexFromXCoord(int globalMouseX) {
    if (!m_textRenderer.isInitialized()) return 0;

    float relativeMouseX = static_cast<float>(globalMouseX - (m_posX + m_style.paddingX) + m_scrollX);

    if (relativeMouseX <= 0) return 0;

   
    int totalTextWidth = 0;
    for (int i = 0; i < static_cast<int>(m_displayText.length()); ++i) {
        
        std::string charStr = m_displayText.substr(i, 1);
        int charW = 0, charH = 0;
        m_textRenderer.measureText(charStr, m_fontSize, charW, charH);

       
        if (relativeMouseX < (totalTextWidth + charW / 2.0f)) {
            
            return i;
        }
        totalTextWidth += charW;
        if (relativeMouseX < totalTextWidth) {
           
             return i + 1;
        }
    }

   
    if (relativeMouseX >= totalTextWidth) {
        return static_cast<int>(m_displayText.length());
    }

    return static_cast<int>(m_displayText.length());
}


int InputBox::getTextXPosition(int charIndex) const {
    if (!m_textRenderer.isInitialized() || m_displayText.empty() || charIndex <= 0) {
        return 0;
    }
    
    charIndex = std::max(0, std::min(static_cast<int>(m_displayText.length()), charIndex));

   
    std::string sub = m_displayText.substr(0, charIndex);
    int subW = 0, subH = 0;
    m_textRenderer.measureText(sub, m_fontSize, subW, subH);
    return subW;
}

void InputBox::deleteSelection() {
    if (m_selectionStart == -1 || m_selectionStart == m_cursorPos) return; // No actual selection

    
    int selS = std::min(m_selectionStart, m_cursorPos);
    int selE = std::max(m_selectionStart, m_cursorPos);
    int length = selE - selS;

    if (length > 0 && selS < static_cast<int>(m_text.length())) {
       
        m_text.erase(selS, length);
       
        m_cursorPos = selS;
        m_selectionStart = -1; 
       
    }
}


void InputBox::copySelectionToClipboard() {
    if (m_isPassword) return; 

    std::string selectedText;
    if (m_selectionStart != -1 && m_selectionStart != m_cursorPos) {
        int selS = std::min(m_selectionStart, m_cursorPos);
        int selE = std::max(m_selectionStart, m_cursorPos);
        selectedText = m_text.substr(selS, selE - selS);
    }
   

    if (!selectedText.empty()) {
        if (SDL_SetClipboardText(selectedText.c_str()) != 0) {
           
           
        }
    }
}


void InputBox::pasteFromClipboard() {
    if (!SDL_HasClipboardText()) return;

    char* clipboardChars = SDL_GetClipboardText();
    if (!clipboardChars) {
     
        return;
    }
    std::string textToPaste(clipboardChars);
    SDL_free(clipboardChars);

    if (textToPaste.empty()) return;

   
    if (m_selectionStart != -1 && m_selectionStart != m_cursorPos) {
        deleteSelection(); 
    }

    size_t CharsToInsertCount = textToPaste.length();
   
    if (m_maxLength > 0) {
        if (m_text.length() >= static_cast<size_t>(m_maxLength)) {
            return; 
        }
        if (m_text.length() + CharsToInsertCount > static_cast<size_t>(m_maxLength)) {
           
            CharsToInsertCount = m_maxLength - m_text.length();
            textToPaste = textToPaste.substr(0, CharsToInsertCount);
        }
    }

    if (CharsToInsertCount > 0) {
       
        m_text.insert(m_cursorPos, textToPaste);
        
        m_cursorPos += CharsToInsertCount;
        m_selectionStart = -1;

       
    }
}


void InputBox::cutSelectionToClipboard() {
    if (m_isPassword) {
        // If password, just delete the selection if one exists
        if (m_selectionStart != -1 && m_selectionStart != m_cursorPos) {
            deleteSelection();
            
        }
        return;
    }

    if (m_selectionStart != -1 && m_selectionStart != m_cursorPos) {
        copySelectionToClipboard();
        deleteSelection();         
     
    }
  
}


std::string InputBox::getText() const {
    return m_text;
}


void InputBox::setText(const std::string& text) {
   
    if (m_maxLength > 0 && text.length() > static_cast<size_t>(m_maxLength)) {
        m_text = text.substr(0, m_maxLength);
    } else {
        m_text = text;
    }

   
    m_cursorPos = static_cast<int>(m_text.length());
    m_selectionStart = -1;                       
    m_scrollX = 0;                              

    updateDisplayText(); 
    clampCursorAndScroll(); 

    if (onTextChanged) {
        onTextChanged(m_text); 
    }

   
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


void InputBox::setMaxLength(int maxLength) {
    m_maxLength = std::max(0, maxLength);
    if (m_maxLength > 0 && m_text.length() > static_cast<size_t>(m_maxLength)) {
        m_text = m_text.substr(0, m_maxLength);
        updateDisplayText();
        clampCursorAndScroll();

        if (onTextChanged) {
            onTextChanged(m_text);
        }

       
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




