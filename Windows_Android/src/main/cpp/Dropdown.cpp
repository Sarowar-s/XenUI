// SPDX-License-Identifier: Apache-2.0
/*
 *  Copyright (C) 2025 MD S M Sarowar Hossain
 *
 * 
 */
// Implementation of the Dropdown control, providing both retained mode (Dropdown class)
// and immediate mode (XenUI::Dropdown function) functionalities.
//
#include "Dropdown.h"
#include "WindowUtil.h" // Includes utility for coordinate calculations (CalculateFinalPosition)
#include <algorithm>    // For std::clamp and std::min/max

// Get the global TextRenderer instance (commented out as it's typically accessed via getInstance())
//auto& tr = TextRenderer::getInstance();

using namespace XenUI;

// ---------------- Retained implementation Starts ----------------

/**
 * @brief Constructs a Dropdown object for Retained Mode UI.
 *
 * Initializes the dropdown with its configuration, options, and style.
 *
 * @param id A unique identifier string for the control.
 * @param posParams Parameters determining the dropdown's position relative to its parent.
 * @param width The fixed width of the dropdown main button and list.
 * @param options A list of strings representing the selectable items.
 * @param initialSelectedIndex The index of the initially selected option.
 * @param style Visual style parameters for the dropdown.
 * @param onSelectionChanged A function to call when a new item is selected.
 */
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
    // Ensure the initial index is valid if options are provided
    if (!m_options.empty()) {
        m_selectedIndex = std::clamp(m_selectedIndex, 0, int(m_options.size()) - 1);
    } else {
        // If there are no options, selection index should be invalid
        m_selectedIndex = -1;
    }

    // Measure required heights and perform initial layout calculation
    recalculateLayout();
}

/**
 * @brief Recalculates the internal layout and positioning of the dropdown.
 *
 * Measures the necessary heights for the main button and list items based on font size,
 * then calculates the final position of the control within the parent/content area.
 *
 * @param parentWidth The width of the parent container/content area.
 * @param parentHeight The height of the parent container/content area.
 */
void Dropdown::recalculateLayout(int parentWidth, int parentHeight) {
    // Measure height required for main button text (e.g., using "M" as an average glyph size)
    int tw, th;
    if (TextRenderer::getInstance().isInitialized()) {
        TextRenderer::getInstance().measureText("M", m_style.mainButtonFontSize, tw, th);
        // Main button height is text height plus vertical padding
        m_mainButtonHeight = th + 2 * m_style.paddingY;
        // Measure height required for list item text
        TextRenderer::getInstance().measureText("M", m_style.listItemFontSize, tw, th);
        // List item height is text height plus vertical padding
        m_listItemHeight = th + 2 * m_style.paddingY;
    } else {
        // Fallback dimensions if text renderer is not initialized
        m_mainButtonHeight = 24;
        m_listItemHeight = 20;
    }

    // Compute the final position (top-left) in content-space using position parameters
    SDL_Point finalPos = XenUI::CalculateFinalPosition(m_posParams, m_width, m_mainButtonHeight, parentWidth, parentHeight);
    m_posX = finalPos.x;
    m_posY = finalPos.y;
}

/**
 * @brief Gets the bounding rectangle of the main dropdown button in content-space coordinates.
 * @return The SDL_FRect for the main button.
 */
SDL_FRect Dropdown::getMainButtonRectContent() const {
    return { float(m_posX), float(m_posY), float(m_width), float(m_mainButtonHeight) };
}

/**
 * @brief Gets the bounding rectangle of a specific list item in content-space coordinates.
 * @param index The index of the list item.
 * @return The SDL_FRect for the list item, or {0,0,0,0} if the index is out of bounds.
 */
SDL_FRect Dropdown::getListItemRectContent(int index) const {
    if (index < 0 || index >= (int)m_options.size()) return {0,0,0,0};
    // Calculate the Y position: main button Y + main button height + (index * item height)
    float itemY = float(m_posY + m_mainButtonHeight + index * m_listItemHeight);
    return { float(m_posX), itemY, float(m_width), float(m_listItemHeight) };
}

/**
 * @brief Checks if a given point is within the bounds of the main dropdown button (content-space).
 * @param x The X coordinate (content-space).
 * @param y The Y coordinate (content-space).
 * @return true if the point is inside the main button rectangle.
 */
bool Dropdown::isPointInMainButtonContent(float x, float y) const {
    SDL_FRect r = getMainButtonRectContent();
    return x >= r.x && x <= (r.x + r.w) && y >= r.y && y <= (r.y + r.h);
}

/**
 * @brief Checks if a given point is within the bounds of the open list area (content-space).
 * @param x The X coordinate (content-space).
 * @param y The Y coordinate (content-space).
 * @return true if the point is inside the list area and the dropdown is open.
 */
bool Dropdown::isPointInListAreaContent(float x, float y) const {
    if (!m_isOpen || m_options.empty()) return false;
    // Calculate the list's top and bottom Y coordinates
    float top = float(m_posY + m_mainButtonHeight);
    float bottom = top + float(m_options.size() * m_listItemHeight);
    // Check if point is within the X range and the list's Y range
    return x >= float(m_posX) && x <= float(m_posX + m_width) && y >= top && y <= bottom;
}

/**
 * @brief Processes an SDL event to update the dropdown's state (open/closed, selection, hover).
 *
 * This function assumes event coordinates are already translated to the control's
 * content-space (e.g., by a parent ScrollView).
 *
 * @param event The SDL_Event to process.
 * @return true if the internal state changed, false otherwise.
 */
bool Dropdown::handleEvent(const SDL_Event& event) {
    // Extract mouse coordinates from event (assumed to be content-space coordinates)
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
        // Fallback to getting current mouse state (may be screen-space if not handled by parent)
        SDL_FPoint mp; SDL_GetMouseState(&mp.x, &mp.y);
        mouseX = mp.x; mouseY = mp.y;
        haveMouseCoords = true;
    } else {
        // General fallback to current mouse state (similarly, may be screen-space)
        SDL_FPoint mp; SDL_GetMouseState(&mp.x, &mp.y);
        mouseX = mp.x; mouseY = mp.y;
        haveMouseCoords = true;
    }

    bool changed = false;

    if (m_isOpen) {
        // Handling input when the list is open
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
            // If the click was not on an item AND not on the main button, close the list
            if (!clickedItem && !isPointInMainButtonContent(mouseX, mouseY)) {
                m_isOpen = false;
                m_hoveredListItemIndex = -1;
                changed = true;
            }
        } else if (event.type == SDL_EVENT_MOUSE_MOTION) {
            // Update hovered list item index
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
        // Handling input when the list is closed
        // Update main button hover state
        bool wasHovered = m_isHoveredMainButton;
        m_isHoveredMainButton = isPointInMainButtonContent(mouseX, mouseY);
        if (wasHovered != m_isHoveredMainButton) changed = true;

        if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button == SDL_BUTTON_LEFT) {
            // Click on the main button opens the list
            if (m_isHoveredMainButton) {
                m_isOpen = true;
                changed = true;
            }
        }
    }

    return changed;
}

/**
 * @brief Renders the dropdown control (main button and open list) to the screen.
 *
 * All local content-space coordinates are translated to absolute screen-space using viewOffset.
 *
 * @param renderer The SDL_Renderer context.
 * @param viewOffset The coordinate space offset (e.g., from a scroll view).
 */
void Dropdown::draw(SDL_Renderer* renderer, const SDL_FPoint& viewOffset) {
    if (!renderer) return;
    if (!TextRenderer::getInstance().isInitialized()) {
        // Cannot draw text label/options without an initialized TextRenderer
        return;
    }

    // Compute screen-space rect for the main button
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
            // Item rect in screen space
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

/**
 * @brief Retrieves the bounding rectangle of the control in content-space.
 *
 * The bounds reflect only the main button when closed, but include the entire
 * dropdown list when open to ensure proper hit testing by parent containers
 * (e.g., ScrollView needs the total occupied space).
 *
 * @return The SDL_FRect representing the control's total occupied area.
 */
SDL_FRect Dropdown::getBounds() const {
    // Return bounding box of main button (content-space) when closed,
    // or the main button + list area when open.
    SDL_FRect mainBtn = getMainButtonRectContent();

    if (!m_isOpen || m_options.empty()) {
        return mainBtn;
    }

    // Calculate the total extent including the list area below
    float listTop = float(m_posY + m_mainButtonHeight);
    float listHeight = float(m_options.size() * m_listItemHeight);
    float totalTop = std::min(mainBtn.y, listTop);
    float totalBottom = std::max(mainBtn.y + mainBtn.h, listTop + listHeight);
    SDL_FRect full = { mainBtn.x, totalTop, mainBtn.w, totalBottom - totalTop };
    return full;
}


/**
 * @brief Programmatically sets the selected index and triggers the selection change callback.
 * @param newIndex The index of the option to select.
 */
void Dropdown::setSelectedIndex(int newIndex) {
    if (m_options.empty()) return;
    newIndex = std::clamp(newIndex, 0, int(m_options.size()) - 1);
    if (newIndex != m_selectedIndex) {
        m_selectedIndex = newIndex;
        if (m_onSelectionChanged) m_onSelectionChanged(m_selectedIndex);
    }
}

/**
 * @brief Returns the text of the currently selected option.
 * @return The selected option string, or an empty string if no option is selected.
 */
std::string Dropdown::getSelectedText() const {
    if (m_selectedIndex >= 0 && m_selectedIndex < (int)m_options.size()) return m_options[m_selectedIndex];
    return {};
}

// ---------------- Retained implementation Ends ----------------


// ---------------- Immediate Mode Dropdown Starts ----------------
namespace XenUI {

/**
 * @brief Immediate-mode function to draw and handle a Dropdown control.
 *
 * The function manages the dropdown's state (open/closed, selection, hover) internally
 * using a static map keyed by 'id'. It performs layout, processes input, updates the
 * external 'selectedIndex' pointer, and draws the control in one call.
 *
 * @param id A unique string identifier for tracking internal state across frames.
 * @param posParams Parameters for positioning the control.
 * @param width The fixed width of the dropdown.
 * @param options The list of selectable option strings.
 * @param selectedIndex Pointer to the external integer variable holding the selected index.
 * @param style Visual style parameters.
 * @param viewOffset The scroll offset to translate content-space to screen-space (default {0.0f, 0.0f}).
 * @param parentWidth The width of the parent container for layout calculation (default -1, uses window width).
 * @param parentHeight The height of the parent container for layout calculation (default -1, uses window height).
 * @return true if the selected index was changed during this call, false otherwise.
 */
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

    // Access the persistent state for this unique control ID
    Detail::DropdownState& st = Detail::dropdownStates[id];
    bool changed = false;

    // Sync internal state with external state pointer and clamp to valid range
    if (st.selectedIndex != *selectedIndex) st.selectedIndex = *selectedIndex;
    st.selectedIndex = std::clamp(st.selectedIndex, 0, int(options.size()) - 1);

    // Measure sizes based on font settings
    int tw, th;
    TextRenderer::getInstance().measureText("M", style.mainButtonFontSize, tw, th);
    st.mainButtonHeight = th + 2 * style.paddingY;
    TextRenderer::getInstance().measureText("M", style.listItemFontSize, tw, th);
    st.listItemHeight = th + 2 * style.paddingY;

    // Normalize parent dimensions for position calculation
    int pW = parentWidth;
    int pH = parentHeight;
    if (pW <= 0 || pH <= 0) {
        SDL_Point win = XenUI::GetWindowSize();
        pW = win.x;
        pH = win.y;
    }

    // Resolve content-space position
    SDL_Point localPos = CalculateFinalPosition(posParams, int(width), st.mainButtonHeight, pW, pH);

    // Convert to absolute screen-space position using viewOffset
    float finalX = float(localPos.x) + viewOffset.x;
    float finalY = float(localPos.y) + viewOffset.y;

    // Store the main button's screen-space rectangle in state
    st.mainButtonRect = { finalX, finalY, width, float(st.mainButtonHeight) };

    // Input handling
    SDL_FPoint mp; SDL_GetMouseState(&mp.x, &mp.y);
    bool mouseLeftDown = (SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON_MASK(SDL_BUTTON_LEFT)) != 0;

    // Check if mouse is over the main button (screen-space check)
    bool isOverMain = (mp.x >= st.mainButtonRect.x && mp.x <= (st.mainButtonRect.x + st.mainButtonRect.w)
                    && mp.y >= st.mainButtonRect.y && mp.y <= (st.mainButtonRect.y + st.mainButtonRect.h));

    st.isHoveredMainButton = isOverMain;

    if (st.isOpen) {
        // Dropdown is open: handle list item interaction
        st.hoveredListItemIndex = -1;
        bool clickedItem = false;

        for (size_t i = 0; i < options.size(); ++i) {
            // Calculate item's screen-space rectangle
            SDL_FRect item = { finalX, finalY + st.mainButtonHeight + i * st.listItemHeight,
                               width, float(st.listItemHeight) };
            bool overItem = (mp.x >= item.x && mp.x <= item.x + item.w &&
                             mp.y >= item.y && mp.y <= item.y + item.h);
            if (overItem) st.hoveredListItemIndex = int(i);

            // Check for click on a list item
            if (mouseLeftDown && overItem) {
                if (st.selectedIndex != int(i)) {
                    st.selectedIndex = int(i);
                    *selectedIndex = st.selectedIndex; // Update external state
                    changed = true;
                }
                st.isOpen = false; // Close on selection
                clickedItem = true;
                break;
            }
        }

        // If the click (mouse down) was not on an item AND not on the main button, close the list
        if (mouseLeftDown && !clickedItem && !isOverMain) {
            st.isOpen = false;
        }
    } else {
        // Dropdown is closed: check for click on main button to open
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
        // Calculate text position (centered vertically, padded horizontally)
        int tx = int(st.mainButtonRect.x + style.paddingX);
        int ty = int(st.mainButtonRect.y + (st.mainButtonRect.h - textH) / 2.0f);
        TextRenderer::getInstance().getInstance().renderText(disp, tx, ty, style.mainButtonTextColor, style.mainButtonFontSize);
    }

    // Draw the dropdown list if open
    if (st.isOpen) {
        // Calculate list background screen rectangle
        SDL_FRect listBg = { finalX, finalY + st.mainButtonHeight, width,
                             float(options.size() * st.listItemHeight) };
        SDL_SetRenderDrawColor(renderer, style.listBgColor.r, style.listBgColor.g,
                               style.listBgColor.b, style.listBgColor.a);
        SDL_RenderFillRect(renderer, &listBg);

        // Draw individual list items
        for (size_t i = 0; i < options.size(); ++i) {
            // Item screen rectangle
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