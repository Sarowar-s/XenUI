#ifndef DROPDOWN_H
#define DROPDOWN_H


// SPDX-License-Identifier: Apache-2.0
/*
 *  Copyright (C) 2025 MD S M Sarowar Hossain
 */
// Defines the structure and interface for the Dropdown control, supporting both
// retained mode (Dropdown class) and immediate mode (XenUI::Dropdown function).
//
#include <SDL3/SDL.h>
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include <iostream>
#include <algorithm>

#include "TextRenderer.h"
#include "Position.h"
#include "UIElement.h" // IControl interface definition

// Style -----------------------------
/**
 * @brief Defines the visual style and sizing parameters for a Dropdown control.
 *
 * Contains all colors, font sizes, padding, and layout constants.
 */
struct DropdownStyle {
    // Color definitions
    SDL_Color mainButtonBgColor        = {80, 80, 80, 255};      ///< Background color of the main button.
    SDL_Color mainButtonTextColor      = {255, 255, 255, 255};   ///< Text color for the selected item in the main button.
    SDL_Color mainButtonBorderColor    = {120, 120, 120, 255};   ///< Border color for the main button.
    SDL_Color listBgColor              = {60, 60, 60, 255};      ///< Background color of the dropdown list when open.
    SDL_Color listItemTextColor        = {255, 255, 255, 255};   ///< Text color for items in the list.
    SDL_Color listItemHoverBgColor     = {100, 100, 100, 255};   ///< Background color for a list item when hovered.
    SDL_Color listItemBorderColor      = {90, 90, 90, 255};      ///< Border color for list items.

    // Sizing parameters
    int paddingX                       = 10;                     ///< Horizontal padding inside the button/list items.
    int paddingY                       = 8;                      ///< Vertical padding inside the button/list items.
    int mainButtonFontSize             = 18;                     ///< Font size for the text in the main button.
    int listItemFontSize               = 16;                     ///< Font size for list item text.
    int listMaxHeight                  = 200;                    ///< Maximum height of the dropdown list before a scrollbar might be needed.
    bool drawBorder                    = true;                   ///< Flag to enable/disable drawing borders.
    int scrollbarWidth                 = 12;                     ///< Width reserved for a future scrollbar implementation.
};

const int DEFAULT_DROPDOWN_FONT_SIZE = 18;

// ---------- Retained mode Dropdown Starts ----------
/**
 * @brief Represents a Dropdown UI control implemented in Retained Mode.
 *
 * This class maintains its state (open/closed, selection), handles events, and
 * manages its own layout relative to its parent container.
 */
class Dropdown : public IControl {
public:
    /**
     * @brief Constructor for the retained mode Dropdown control.
     *
     * @param id A unique identifier for the control instance.
     * @param posParams Parameters for resolving the control's position.
     * @param width The fixed width of the dropdown control.
     * @param options A list of selectable string options.
     * @param initialSelectedIndex The index of the option initially selected (default 0).
     * @param style The visual style parameters (defaults to default DropdownStyle).
     * @param onSelectionChanged A callback function triggered when the selection changes.
     */
    Dropdown(const std::string& id,
             const XenUI::PositionParams& posParams,
             float width,
             const std::vector<std::string>& options,
             int initialSelectedIndex = 0,
             DropdownStyle style = {},
             std::function<void(int)> onSelectionChanged = nullptr);

    // IControl interface implementation:

    /**
     * @brief Processes an SDL event to update the dropdown's state.
     *
     * Event coordinates are expected to be in content-space if the event was forwarded
     * and translated by a parent container (like a ScrollView).
     *
     * @param event The SDL_Event to handle.
     * @return true if the control's state (open/closed, selection, hover) changed, false otherwise.
     */
    bool handleEvent(const SDL_Event& event) override;

    /**
     * @brief Renders the dropdown (main button and open list) to the screen.
     *
     * Applies the 'viewOffset' to convert content-space coordinates to absolute screen-space.
     *
     * @param renderer The SDL_Renderer context.
     * @param viewOffset The offset applied by a parent (e.g., scroll position).
     */
    void draw(SDL_Renderer* renderer, const SDL_FPoint& viewOffset) override;

    /**
     * @brief Recalculates the position and internal element heights.
     *
     * Uses parent dimensions to resolve anchor-based positioning.
     *
     * @param parentWidth The width of the parent container/content area.
     * @param parentHeight The height of the parent container/content area.
     */
    void recalculateLayout(int parentWidth, int parentHeight) override;

    /**
     * @brief Overload to recalculate layout using the current window size as parent dimensions.
     */
    void recalculateLayout(){
        SDL_Point s = XenUI::GetWindowSize();
        recalculateLayout(s.x, s.y);
    }

    /**
     * @brief Retrieves the bounding rectangle of the control in content-space.
     *
     * The bounds include the list area when the dropdown is open to ensure proper
     * clipping and input routing from parent containers.
     *
     * @return The SDL_FRect defining the control's total occupied area.
     */
    SDL_FRect getBounds() const override;

    // Direct API for external control
    /**
     * @brief Sets the selected option by index.
     * @param newIndex The index of the option to select.
     */
    void setSelectedIndex(int newIndex);

    /**
     * @brief Returns the index of the currently selected option.
     * @return The selected index.
     */
    int  getSelectedIndex() const { return m_selectedIndex; }

    /**
     * @brief Returns the text of the currently selected option.
     * @return The selected option string, or an empty string if none is selected.
     */
    std::string getSelectedText() const;

    /**
     * @brief Returns the unique identifier for this control instance.
     * @return The control's ID string.
     */
    const std::string& getId() const { return m_id; }

private:
    // Helper functions to compute geometry in content-space (without viewOffset)
    /**
     * @brief Gets the bounding rectangle of the main dropdown button (content-space).
     * @return The main button's SDL_FRect.
     */
    SDL_FRect getMainButtonRectContent() const;

    /**
     * @brief Gets the bounding rectangle of a specific list item (content-space).
     * @param index The index of the list item.
     * @return The list item's SDL_FRect.
     */
    SDL_FRect getListItemRectContent(int index) const;

    /**
     * @brief Checks if a content-space point is inside the main button.
     * @param x The X coordinate (content-space).
     * @param y The Y coordinate (content-space).
     * @return true if the point is within the main button.
     */
    bool isPointInMainButtonContent(float x, float y) const;

    /**
     * @brief Checks if a content-space point is inside the open list area.
     * @param x The X coordinate (content-space).
     * @param y The Y coordinate (content-space).
     * @return true if the point is within the list area and the list is open.
     */
    bool isPointInListAreaContent(float x, float y) const;

private:
    // Configuration data
    std::string m_id;                                     ///< Unique identifier.
    XenUI::PositionParams m_posParams;                    ///< Positioning parameters.
    std::vector<std::string> m_options;                  ///< List of selectable items.
    int m_selectedIndex;                                  ///< Index of the currently selected option.
    DropdownStyle m_style;                                ///< Visual style properties.
    std::function<void(int)> m_onSelectionChanged;        ///< Callback on selection change.

    // Layout/Geometry data (content-space coordinates)
    int m_posX = 0;                                       ///< X position in content-space.
    int m_posY = 0;                                       ///< Y position in content-space.
    int m_width = 0;                                      ///< Fixed width of the control.
    int m_mainButtonHeight = 0;                           ///< Calculated height of the main button.
    int m_listItemHeight   = 0;                           ///< Calculated height of a single list item.

    // Runtime state
    bool m_isOpen = false;                                ///< True if the list is currently displayed.
    bool m_isHoveredMainButton = false;                   ///< True if the mouse is hovering over the main button.
    int  m_hoveredListItemIndex = -1;                     ///< Index of the list item currently hovered, or -1.
};
// ---------- Retained mode Dropdown Ends ----------


// ---------- Immediate mode Dropdown Starts ----------
namespace XenUI {
    namespace Detail {
        /**
         * @brief Internal structure to hold the persistent state required for the immediate-mode dropdown.
         *
         * Since immediate mode functions are stateless, this structure maintains the necessary
         * state variables (like open status and selection) across multiple frames.
         */
        struct DropdownState {
            int selectedIndex = 0;
            bool isOpen = false;
            bool isHoveredMainButton = false;
            int hoveredListItemIndex = -1;
            SDL_FRect mainButtonRect = {0,0,0,0}; // Stores the main button's SCREEN-SPACE rect for hit-testing & drawing
            int listItemHeight = 0;
            int mainButtonHeight = 0;
        };
        // Static map to store the state for each immediate-mode dropdown instance, keyed by its unique ID.
        static std::unordered_map<std::string, DropdownState> dropdownStates;
    }

    // IMPORTANT: posParams are CONTENT-SPACE. viewOffset converts content->screen.
    // Pass viewOffset (BeginScrollView returns it). For non-scroll usage pass {0,0}.
    /**
     * @brief Immediate-mode function to draw and handle a Dropdown control.
     *
     * This function is responsible for layout calculation, event handling (mouse input),
     * updating the external selection index, and rendering the entire control and list.
     *
     * @param id A unique string identifier used to retrieve persistent state (DropdownState).
     * @param posParams Positioning parameters in content-space.
     * @param width The fixed width of the dropdown.
     * @param options The list of selectable option strings.
     * @param selectedIndex Pointer to the external integer variable holding the selected index.
     * @param style Visual style parameters.
     * @param viewOffset The scroll offset applied to translate content-space to screen-space (default {0.0f, 0.0f}).
     * @param parentWidth The width of the parent container for layout calculation (default -1, uses window width).
     * @param parentHeight The height of the parent container for layout calculation (default -1, uses window height).
     * @return true if the selected index was changed during this call, false otherwise.
     */
bool Dropdown(const std::string& id,
              const PositionParams& posParams,
              float width,
              const std::vector<std::string>& options,
              int* selectedIndex,
              DropdownStyle style = {},

              const SDL_FPoint& viewOffset = {0.0f, 0.0f},
             int parentWidth = -1,
              int parentHeight = -1);

}
// ---------- Immediate mode Dropdown Ends ----------

#endif // DROPDOWN_H