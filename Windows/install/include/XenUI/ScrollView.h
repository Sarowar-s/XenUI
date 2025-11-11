//
// File: ScrollView.h
//
// Copyright (C) 2025 MD S M Sarowar Hossain
//
// Defines the retained-mode ScrollView class (IControl) and the immediate-mode
// ScrollView API (BeginScrollView/EndScrollView).
//

#pragma once

#include <vector>
#include <memory>
#include <functional>
#include <string>
#include "SDL3/SDL.h"

#include "UIElement.h"   // Defines IControl base interface
#include "Position.h"    // Defines XenUI::PositionParams
#include "TextRenderer.h"// Included for potential text rendering context (e.g., debug)

/**
 * @brief Defines the visual style properties for both retained and immediate mode ScrollViews.
 */
struct ScrollViewStyle {
    SDL_Color bgColor = { 0, 0, 0, 255 };                ///< Color of the background area within the ScrollView bounds.
    SDL_Color scrollbarBgColor = { 20, 20, 20, 255 };    ///< Color of the scrollbar track background.
    SDL_Color scrollbarThumbColor = { 80, 80, 80, 255 }; ///< Color of the scrollbar thumb (default state).
    SDL_Color scrollbarThumbHoverColor = { 110, 110, 110, 255 }; ///< Color of the scrollbar thumb when hovered.
    SDL_Color scrollbarThumbGrabbedColor = { 140, 140, 140, 255 }; ///< Color of the scrollbar thumb when dragged.
    int scrollbarWidth = 12;                            ///< Width of the scrollbar in pixels.
    bool drawBackground = true;                         ///< Flag to enable/disable background drawing.
    bool drawBorder = true;                             ///< Flag to enable/disable border drawing.
    SDL_Color borderColor = { 60, 60, 60, 255 };        ///< Color of the outer border.
};


// -----------------------------------------------------------------------------
// --- Retained Mode ScrollView Implementation Starts Here (Header) ---
// -----------------------------------------------------------------------------

/**
 * @brief A retained-mode container control that provides a scrollable viewport for child controls.
 *
 * The ScrollView manages child layout relative to its content area, performs clipping,
 * and handles scroll input via mouse wheel, scrollbar dragging, and touch input.
 */
class ScrollView : public IControl {
public:
    /**
     * @brief Constructs a retained-mode ScrollView.
     * @param posParams The positional constraints for the ScrollView's viewport.
     * @param style The visual style parameters.
     */
    explicit ScrollView(const XenUI::PositionParams& posParams, const ScrollViewStyle& style = {});

    /**
     * @brief Default virtual destructor.
     */
    ~ScrollView() override = default;

    /**
     * @brief Adds a child control to the scroll view's content area.
     *
     * The added control's internal position parameters should be relative to the
     * top-left of the scrollable content area (which starts at 0,0).
     *
     * @param control A unique_ptr owning the IControl instance to add.
     */
    void addControl(std::unique_ptr<IControl> control);

    // --- IControl Interface Overrides ---

    /**
     * @brief Handles events, including scrolling and forwarding input to children.
     *
     * This is the primary event handling entry point, providing window context and offset.
     *
     * @param evt The SDL_Event to process.
     * @param window The SDL_Window the event occurred in (or null for best-effort derivation).
     * @param parentViewOffset The screen-space offset of this ScrollView from its parent or the screen origin.
     * @return true if the event was handled and consumed, false otherwise.
     */
    bool handleEvent(const SDL_Event& evt, SDL_Window* window, const SDL_FPoint& parentViewOffset);

    /**
     * @brief Convenience overload for IControl base compatibility (assumes zero parent offset).
     * @param e The SDL_Event to process.
     * @return true if the event was handled.
     */
    bool handleEvent(const SDL_Event& e) override;

    /**
     * @brief Renders the ScrollView, setting up clipping before drawing its children.
     * @param renderer The SDL_Renderer context.
     * @param viewOffset The screen-space offset inherited from parent containers.
     */
    void draw(SDL_Renderer* renderer, const SDL_FPoint& viewOffset) override;

    /**
     * @brief Recalculates the ScrollView's own size and the layout of its children.
     * @param parentWidth The width of the ScrollView's parent container.
     * @param parentHeight The height of the ScrollView's parent container.
     */
    void recalculateLayout(int parentWidth, int parentHeight) override;

    /**
     * @brief Returns the absolute bounds of the ScrollView viewport on screen.
     * @return The SDL_FRect defining the outer bounds.
     */
    SDL_FRect getBounds() const override { return m_bounds; }

private:
    /**
     * @brief Recomputes the total required height of the scrollable content area.
     */
    void updateContentHeight();

    /**
     * @brief Renders the vertical scrollbar track and thumb.
     * @param renderer The SDL_Renderer context.
     * @param viewOffset The screen-space offset of the ScrollView.
     */
    void drawScrollbar(SDL_Renderer* renderer, const SDL_FPoint& viewOffset);

    /**
     * @brief Calculates the size and relative position of the scrollbar thumb.
     * @param viewOffset Unused parameter (retained for consistency with the IControl draw pattern).
     * @return The SDL_FRect defining the thumb's size and position relative to the viewRect.
     */
    SDL_FRect getScrollbarThumbRect(const SDL_FPoint& viewOffset) const;

    XenUI::PositionParams m_posParams;              ///< Positional constraints for the ScrollView itself.
    ScrollViewStyle m_style;                        ///< Visual style configuration.
    std::vector<std::unique_ptr<IControl>> m_controls; ///< Collection of child controls.

    SDL_FRect m_bounds{};                           ///< The resolved absolute bounds of the entire control (including scrollbar area).
    SDL_FRect m_viewRect{};                         ///< The inner area reserved for content (m_bounds minus scrollbar space).
    float m_contentHeight = 0.0f;                   ///< The maximum vertical extent required by all children.
    float m_scrollY = 0.0f;                         ///< Current vertical scroll offset (0.0 = top).

    // State for scrollbar interaction
    bool m_isScrollbarHovered = false;              ///< True if the scrollbar thumb is currently being hovered by the mouse.
    bool m_isScrollbarGrabbed = false;              ///< True if the scrollbar thumb is currently being dragged.
    float m_scrollbarGrabOffsetY = 0.0f;            ///< Mouse offset from the top of the thumb when dragging started.
    IControl* m_focusedChild = nullptr;             ///< Pointer to the child control that currently has keyboard focus.

    // touch state
    bool m_touchActive = false;                     ///< True if a touch sequence is active within the ScrollView bounds.
    float m_lastTouchY = 0.0f;                      ///< Last vertical touch position for delta calculation during touch-scrolling.
    uint64_t m_activeTouchId = 0;                   ///< The SDL touch ID of the finger actively performing the scroll/drag.

};
// -----------------------------------------------------------------------------
// --- Retained Mode ScrollView Implementation Ends Here (Header) ---
// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------
// --- Immediate Mode ScrollView API Starts Here ---
// -----------------------------------------------------------------------------

namespace XenUI {

    /**
     * @brief Immediate Mode: Starts a scrollable region by specifying the view rectangle directly.
     *
     * This function should be called first. It processes input (scroll, drag), applies clipping,
     * and returns the rendering offset.
     *
     * @param id Unique identifier (persistent state per-view, e.g., "my_scroll_area").
     * @param viewRect On‐screen clipping rectangle of the viewport.
     * @param contentSize Total size of the scrollable content.
     * @param renderer SDL_Renderer*.
     * @param event Current SDL_Event& (for wheel + drag).
     * @param style The visual style parameters.
     * @return The view offset {screen_x, screen_y - scroll_y} to apply when drawing children.
     */
    SDL_FPoint BeginScrollView(
        const std::string&          id,
        const SDL_FRect&            viewRect,
        const SDL_FPoint&           contentSize,
        SDL_Renderer* renderer,
        const SDL_Event&            event,
        const ScrollViewStyle&      style = {}
    );

    /**
     * @brief Immediate Mode: Starts a scrollable region by resolving PositionParams.
     *
     * Computes the final viewRect using the global context (window size).
     *
     * @param id Unique identifier.
     * @param posParams Positional constraints (anchors / offsets).
     * @param viewWidth The explicit width of the viewport.
     * @param viewHeight The explicit height of the viewport.
     * @param contentSize Total size of the scrollable content.
     * @param renderer SDL_Renderer*.
     * @param event Current SDL_Event&.
     * @param style The visual style parameters.
     * @return The view offset to apply when drawing children.
     */
    SDL_FPoint BeginScrollView(
        const std::string&            id,
        const XenUI::PositionParams&  posParams,     // anchors / offsets
        int                           viewWidth,
        int                           viewHeight,
        const SDL_FPoint&             contentSize,
        SDL_Renderer* renderer,
        const SDL_Event&              event,
        const ScrollViewStyle&        style = {}
    );

    /**
     * @brief Immediate Mode: Starts a scrollable region, allowing explicit parent dimensions.
     *
     * Used when the ScrollView is nested or needs to resolve its anchors relative to a specific parent area.
     *
     * @param id Unique identifier.
     * @param posParams Positional constraints.
     * @param viewWidth The explicit width of the viewport.
     * @param viewHeight The explicit height of the viewport.
     * @param parentWidth The width of the parent container used for layout resolution.
     * @param parentHeight The height of the parent container used for layout resolution.
     * @param contentSize Total size of the scrollable content.
     * @param renderer SDL_Renderer*.
     * @param event Current SDL_Event&.
     * @param style The visual style parameters.
     * @return The view offset to apply when drawing children.
     */
    SDL_FPoint BeginScrollView(
        const std::string&            id,
        const XenUI::PositionParams&  posParams,
        int                           viewWidth,
        int                           viewHeight,
        int                           parentWidth,
        int                           parentHeight,
        const SDL_FPoint&             contentSize,
        SDL_Renderer* renderer,
        const SDL_Event&              event,
        const ScrollViewStyle&        style = {}
    );


    /**
     * @brief Immediate Mode: Ends the most-recent BeginScrollView call.
     *
     * This function restores the renderer's clip rectangle, draws the border, and draws the scrollbar.
     *
     * @param renderer The SDL_Renderer context.
     */
    void EndScrollView(SDL_Renderer* renderer);
}

// -----------------------------------------------------------------------------
// --- Immediate Mode ScrollView API Ends Here ---
// -----------------------------------------------------------------------------