// SPDX-License-Identifier: Apache-2.0
/*
 *  Copyright (C) 2025 MD S M Sarowar Hossain
 */
//
// Implementation of the Dropdown control, providing both retained mode (Dropdown class)
// and immediate mode (XenUI::Dropdown function) functionalities.
//
#include "Dropdown.h"
#include "WindowUtil.h" 
#include <algorithm>    


using namespace XenUI;


Dropdown::Dropdown(const std::string& id,
                   const XenUI::PositionParams& posParams,
                   float width,
                   const std::vector<std::string>& options,
                   int initialSelectedIndex,
                   DropdownStyle style,
                   std::function<void(int)> onSelectionChanged)
    : m_id(id),
      m_posParams(posParams),
      m_options(options),
      m_selectedIndex(initialSelectedIndex),
      m_style(style),
      m_onSelectionChanged(std::move(onSelectionChanged)),
      m_width(static_cast<int>(width))
{

    if (!m_options.empty()) {
        m_selectedIndex = std::clamp(m_selectedIndex, 0, int(m_options.size()) - 1);
    } else {
        // If there are no options, selection index should be invalid
        m_selectedIndex = -1;
    }

    
    recalculateLayout();
}


void Dropdown::recalculateLayout(int parentWidth, int parentHeight) {
    
    int tw, th;
    if (TextRenderer::getInstance().isInitialized()) {
        TextRenderer::getInstance().measureText("M", m_style.mainButtonFontSize, tw, th);
        
        m_mainButtonHeight = th + 2 * m_style.paddingY;
      
        TextRenderer::getInstance().measureText("M", m_style.listItemFontSize, tw, th);
      
        m_listItemHeight = th + 2 * m_style.paddingY;
    } else {
       
        m_mainButtonHeight = 24;
        m_listItemHeight = 20;
    }

   
    SDL_Point finalPos = XenUI::CalculateFinalPosition(m_posParams, m_width, m_mainButtonHeight, parentWidth, parentHeight);
    m_posX = finalPos.x;
    m_posY = finalPos.y;
}


SDL_FRect Dropdown::getMainButtonRectContent() const {
    return { float(m_posX), float(m_posY), float(m_width), float(m_mainButtonHeight) };
}


SDL_FRect Dropdown::getListItemRectContent(int index) const {
    if (index < 0 || index >= (int)m_options.size()) return {0,0,0,0};
    // Calculate the Y position: main button Y + main button height + (index * item height)
    float itemY = float(m_posY + m_mainButtonHeight + index * m_listItemHeight);
    return { float(m_posX), itemY, float(m_width), float(m_listItemHeight) };
}


bool Dropdown::isPointInMainButtonContent(float x, float y) const {
    SDL_FRect r = getMainButtonRectContent();
    return x >= r.x && x <= (r.x + r.w) && y >= r.y && y <= (r.y + r.h);
}


bool Dropdown::isPointInListAreaContent(float x, float y) const {
    if (!m_isOpen || m_options.empty()) return false;
   
    float top = float(m_posY + m_mainButtonHeight);
    float bottom = top + float(m_options.size() * m_listItemHeight);
   
    return x >= float(m_posX) && x <= float(m_posX + m_width) && y >= top && y <= bottom;
}


bool Dropdown::handleEvent(const SDL_Event& event) {
   
    float mouseX = 0.0f, mouseY = 0.0f;
    bool haveMouseCoords = false;
    if (event.type == SDL_EVENT_MOUSE_MOTION) {
        mouseX = float(event.motion.x);
        mouseY = float(event.motion.y);
        haveMouseCoords = true;
    } else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN || event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
        mouseX = float(event.button.x);
        mouseY = float(event.button.y);
        haveMouseCoords = true;
    } else if (event.type == SDL_EVENT_MOUSE_WHEEL) {

        SDL_FPoint mp; SDL_GetMouseState(&mp.x, &mp.y);
        mouseX = mp.x; mouseY = mp.y;
        haveMouseCoords = true;
    } else {

        SDL_FPoint mp; SDL_GetMouseState(&mp.x, &mp.y);
        mouseX = mp.x; mouseY = mp.y;
        haveMouseCoords = true;
    }

    bool changed = false;

    if (m_isOpen) {

        if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button == SDL_BUTTON_LEFT) {
            bool clickedItem = false;
            // Check if any list item was clicked
            for (size_t i = 0; i < m_options.size(); ++i) {
                SDL_FRect itemRect = getListItemRectContent(int(i));
                if (mouseX >= itemRect.x && mouseX <= (itemRect.x + itemRect.w)
                    && mouseY >= itemRect.y && mouseY <= (itemRect.y + itemRect.h)) {
                    // Item clicked: update selection, close list, trigger callback
                    if ((int)i != m_selectedIndex) {
                        m_selectedIndex = int(i);
                        if (m_onSelectionChanged) m_onSelectionChanged(m_selectedIndex);
                    }
                    m_isOpen = false;
                    m_hoveredListItemIndex = -1;
                    changed = true;
                    clickedItem = true;
                    break;
                }
            }
            
            if (!clickedItem && !isPointInMainButtonContent(mouseX, mouseY)) {
                m_isOpen = false;
                m_hoveredListItemIndex = -1;
                changed = true;
            }
        } else if (event.type == SDL_EVENT_MOUSE_MOTION) {
          
            int old = m_hoveredListItemIndex;
            m_hoveredListItemIndex = -1;
            for (size_t i = 0; i < m_options.size(); ++i) {
                SDL_FRect r = getListItemRectContent(int(i));
                if (mouseX >= r.x && mouseX <= (r.x + r.w) && mouseY >= r.y && mouseY <= (r.y + r.h)) {
                    m_hoveredListItemIndex = int(i);
                    break;
                }
            }
            if (old != m_hoveredListItemIndex) changed = true;
        }
    } else {
       
        bool wasHovered = m_isHoveredMainButton;
        m_isHoveredMainButton = isPointInMainButtonContent(mouseX, mouseY);
        if (wasHovered != m_isHoveredMainButton) changed = true;

        if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button == SDL_BUTTON_LEFT) {
         
            if (m_isHoveredMainButton) {
                m_isOpen = true;
                changed = true;
            }
        }
    }

    return changed;
}


void Dropdown::draw(SDL_Renderer* renderer, const SDL_FPoint& viewOffset) {
    if (!renderer) return;
    if (!TextRenderer::getInstance().isInitialized()) {
       
        return;
    }

    
    SDL_FRect mainBtn = getMainButtonRectContent();
    mainBtn.x += viewOffset.x;
    mainBtn.y += viewOffset.y;

    // Draw main button background
    SDL_SetRenderDrawColor(renderer, m_style.mainButtonBgColor.r, m_style.mainButtonBgColor.g,
                           m_style.mainButtonBgColor.b, m_style.mainButtonBgColor.a);
    SDL_RenderFillRect(renderer, &mainBtn);

    // Draw border
    if (m_style.drawBorder) {
        SDL_SetRenderDrawColor(renderer, m_style.mainButtonBorderColor.r, m_style.mainButtonBorderColor.g,
                               m_style.mainButtonBorderColor.b, m_style.mainButtonBorderColor.a);
        SDL_RenderRect(renderer, &mainBtn);
    }

    // Draw selected text
    if (m_selectedIndex != -1 && !m_options.empty()) {
        std::string display = m_options[m_selectedIndex];
        int tw, th;
        TextRenderer::getInstance().measureText(display, m_style.mainButtonFontSize, tw, th);
        // Calculate centered text position
        int tx = int(mainBtn.x + m_style.paddingX);
        int ty = int(mainBtn.y + (mainBtn.h - th) / 2.0f);
        TextRenderer::getInstance().getInstance().renderText(display, tx, ty, m_style.mainButtonTextColor, m_style.mainButtonFontSize);
    }

    // Draw list if open (in screen-space)
    if (m_isOpen) {
        // List area's screen coordinates
        float listX = float(m_posX) + viewOffset.x;
        float listY = float(m_posY + m_mainButtonHeight) + viewOffset.y;
        float listW = float(m_width);
        float listH = float(m_options.size() * m_listItemHeight);

        // Draw list background
        SDL_FRect listBg = { listX, listY, listW, listH };
        SDL_SetRenderDrawColor(renderer, m_style.listBgColor.r, m_style.listBgColor.g, m_style.listBgColor.b, m_style.listBgColor.a);
        SDL_RenderFillRect(renderer, &listBg);

        // Draw individual list items
        for (size_t i = 0; i < m_options.size(); ++i) {
           
            SDL_FRect item = getListItemRectContent(int(i));
            item.x += viewOffset.x;
            item.y += viewOffset.y;

            // Highlight hovered item
            if ((int)i == m_hoveredListItemIndex) {
                SDL_SetRenderDrawColor(renderer, m_style.listItemHoverBgColor.r, m_style.listItemHoverBgColor.g,
                                       m_style.listItemHoverBgColor.b, m_style.listItemHoverBgColor.a);
                SDL_RenderFillRect(renderer, &item);
            }

            // Draw item text
            int tw, th;
            TextRenderer::getInstance().measureText(m_options[i], m_style.listItemFontSize, tw, th);
            int tx = int(item.x + m_style.paddingX);
            int ty = int(item.y + (item.h - th) / 2.0f);
            TextRenderer::getInstance().getInstance().renderText(m_options[i], tx, ty, m_style.listItemTextColor, m_style.listItemFontSize);

            // Draw item border
            if (m_style.drawBorder) {
                SDL_SetRenderDrawColor(renderer, m_style.listItemBorderColor.r, m_style.listItemBorderColor.g,
                                       m_style.listItemBorderColor.b, m_style.listItemBorderColor.a);
                SDL_RenderRect(renderer, &item);
            }
        }
    }
}


SDL_FRect Dropdown::getBounds() const {
    
    SDL_FRect mainBtn = getMainButtonRectContent();

    if (!m_isOpen || m_options.empty()) {
        return mainBtn;
    }

   
    float listTop = float(m_posY + m_mainButtonHeight);
    float listHeight = float(m_options.size() * m_listItemHeight);
    float totalTop = std::min(mainBtn.y, listTop);
    float totalBottom = std::max(mainBtn.y + mainBtn.h, listTop + listHeight);
    SDL_FRect full = { mainBtn.x, totalTop, mainBtn.w, totalBottom - totalTop };
    return full;
}



void Dropdown::setSelectedIndex(int newIndex) {
    if (m_options.empty()) return;
    newIndex = std::clamp(newIndex, 0, int(m_options.size()) - 1);
    if (newIndex != m_selectedIndex) {
        m_selectedIndex = newIndex;
        if (m_onSelectionChanged) m_onSelectionChanged(m_selectedIndex);
    }
}


std::string Dropdown::getSelectedText() const {
    if (m_selectedIndex >= 0 && m_selectedIndex < (int)m_options.size()) return m_options[m_selectedIndex];
    return {};
}

// ---------------- Retained implementation Ends ----------------


// ---------------- Immediate Mode Dropdown Starts ----------------
namespace XenUI {


bool Dropdown(const std::string& id,
              const PositionParams& posParams,
              float width,
              const std::vector<std::string>& options,
              int* selectedIndex,
              DropdownStyle style,

              const SDL_FPoint& viewOffset,
              int parentWidth,
              int parentHeight)
{
    SDL_Renderer* renderer = TextRenderer::getInstance().getRenderer();
    if (!renderer || !TextRenderer::getInstance().isInitialized() || !selectedIndex || options.empty())
        return false;

   
    Detail::DropdownState& st = Detail::dropdownStates[id];
    bool changed = false;

   
    if (st.selectedIndex != *selectedIndex) st.selectedIndex = *selectedIndex;
    st.selectedIndex = std::clamp(st.selectedIndex, 0, int(options.size()) - 1);

   
    int tw, th;
    TextRenderer::getInstance().measureText("M", style.mainButtonFontSize, tw, th);
    st.mainButtonHeight = th + 2 * style.paddingY;
    TextRenderer::getInstance().measureText("M", style.listItemFontSize, tw, th);
    st.listItemHeight = th + 2 * style.paddingY;

    
    int pW = parentWidth;
    int pH = parentHeight;
    if (pW <= 0 || pH <= 0) {
        SDL_Point win = XenUI::GetWindowSize();
        pW = win.x;
        pH = win.y;
    }


    SDL_Point localPos = CalculateFinalPosition(posParams, int(width), st.mainButtonHeight, pW, pH);

    // Convert to absolute screen-space position using viewOffset
    float finalX = float(localPos.x) + viewOffset.x;
    float finalY = float(localPos.y) + viewOffset.y;


    st.mainButtonRect = { finalX, finalY, width, float(st.mainButtonHeight) };

    // Input handling
    SDL_FPoint mp; SDL_GetMouseState(&mp.x, &mp.y);
    bool mouseLeftDown = (SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON_MASK(SDL_BUTTON_LEFT)) != 0;


    bool isOverMain = (mp.x >= st.mainButtonRect.x && mp.x <= (st.mainButtonRect.x + st.mainButtonRect.w)
                    && mp.y >= st.mainButtonRect.y && mp.y <= (st.mainButtonRect.y + st.mainButtonRect.h));

    st.isHoveredMainButton = isOverMain;

    if (st.isOpen) {
       
        st.hoveredListItemIndex = -1;
        bool clickedItem = false;

        for (size_t i = 0; i < options.size(); ++i) {

            SDL_FRect item = { finalX, finalY + st.mainButtonHeight + i * st.listItemHeight,
                               width, float(st.listItemHeight) };
            bool overItem = (mp.x >= item.x && mp.x <= item.x + item.w &&
                             mp.y >= item.y && mp.y <= item.y + item.h);
            if (overItem) st.hoveredListItemIndex = int(i);

           
            if (mouseLeftDown && overItem) {
                if (st.selectedIndex != int(i)) {
                    st.selectedIndex = int(i);
                    *selectedIndex = st.selectedIndex; 
                    changed = true;
                }
                st.isOpen = false;
                clickedItem = true;
                break;
            }
        }

      
        if (mouseLeftDown && !clickedItem && !isOverMain) {
            st.isOpen = false;
        }
    } else {
     
        if (mouseLeftDown && isOverMain) {
            st.isOpen = true;
        }
    }

    // --- Drawing ---
    // Draw main button background
    SDL_SetRenderDrawColor(renderer, style.mainButtonBgColor.r, style.mainButtonBgColor.g,
                           style.mainButtonBgColor.b, style.mainButtonBgColor.a);
    SDL_RenderFillRect(renderer, &st.mainButtonRect);

    // Draw main button border
    if (style.drawBorder) {
        SDL_SetRenderDrawColor(renderer, style.mainButtonBorderColor.r, style.mainButtonBorderColor.g,
                               style.mainButtonBorderColor.b, style.mainButtonBorderColor.a);
        SDL_RenderRect(renderer, &st.mainButtonRect);
    }

    // Draw selected text in main button
    if (st.selectedIndex >= 0 && st.selectedIndex < (int)options.size()) {
        std::string disp = options[st.selectedIndex];
        int textW, textH;
        TextRenderer::getInstance().measureText(disp, style.mainButtonFontSize, textW, textH);
       
        int tx = int(st.mainButtonRect.x + style.paddingX);
        int ty = int(st.mainButtonRect.y + (st.mainButtonRect.h - textH) / 2.0f);
        TextRenderer::getInstance().getInstance().renderText(disp, tx, ty, style.mainButtonTextColor, style.mainButtonFontSize);
    }

    
    if (st.isOpen) {
       
        SDL_FRect listBg = { finalX, finalY + st.mainButtonHeight, width,
                             float(options.size() * st.listItemHeight) };
        SDL_SetRenderDrawColor(renderer, style.listBgColor.r, style.listBgColor.g,
                               style.listBgColor.b, style.listBgColor.a);
        SDL_RenderFillRect(renderer, &listBg);

       
        for (size_t i = 0; i < options.size(); ++i) {
           
            SDL_FRect item = { finalX, finalY + st.mainButtonHeight + float(i * st.listItemHeight),
                               width, float(st.listItemHeight) };

            // Highlight if hovered
            if (int(i) == st.hoveredListItemIndex) {
                SDL_SetRenderDrawColor(renderer, style.listItemHoverBgColor.r, style.listItemHoverBgColor.g,
                                       style.listItemHoverBgColor.b, style.listItemHoverBgColor.a);
                SDL_RenderFillRect(renderer, &item);
            }

            // Draw item text
            int itw, ith;
            TextRenderer::getInstance().measureText(options[i], style.listItemFontSize, itw, ith);
            int tx = int(item.x + style.paddingX);
            int ty = int(item.y + (item.h - ith) / 2.0f);
            TextRenderer::getInstance().getInstance().renderText(options[i], tx, ty, style.listItemTextColor, style.listItemFontSize);

            // Draw item border
            if (style.drawBorder) {
                SDL_SetRenderDrawColor(renderer, style.listItemBorderColor.r, style.listItemBorderColor.g,
                                       style.listItemBorderColor.b, style.listItemBorderColor.a);
                SDL_RenderRect(renderer, &item);
            }
        }
    }

    return changed;
}

} // namespace XenUI
// ---------------- Immediate Mode Dropdown Ends ----------------