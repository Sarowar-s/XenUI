// SPDX-License-Identifier: Apache-2.0
/*
 *  Copyright (C) 2025 MD S M Sarowar Hossain
 *
 * 
 */
// Defines the core interface (IControl) for all graphical user interface elements
// within the XenonUI framework, ensuring polymorphic control management.
//

#pragma once

#include "SDL3/SDL.h"

/**
 * @brief An interface for all interactable UI controls (Retained Mode).
 *
 * This pure abstract class defines the essential functions that every control
 * in the XenonUI library must implement. This design allows container elements like
 * ScrollView or Window to manage a heterogeneous collection of controls
 * polymorphically, forming the foundation of the Retained Mode UI structure.
 */
class IControl {
protected:
    /**
     * @brief Tracks whether the control currently has input focus.
     *
     * Protected so base classes can manage it internally via the focus()/unfocus() methods.
     */
    bool m_hasFocus = false;

public:
    /**
     * @brief Virtual destructor.
     *
     * Ensures that derived-class destructors are called correctly when
     * objects are deleted through a base-class pointer (e.g., when a container is cleared).
     */
    virtual ~IControl() = default;

    /**
     * @brief Handles incoming SDL events for the control.
     *
     * Event coordinates are expected to be in the **control's content-space** (relative to the parent's content area).
     *
     * @param e The SDL_Event to process.
     * @return true if the event was handled and resulted in a state change
     * (e.g., hover, press, value change) that requires a redraw, false otherwise.
     */
    virtual bool handleEvent(const SDL_Event& e) = 0;

    /**
     * @brief Renders the control to the screen.
     *
     * The implementation must render the control's geometry and appearance, taking the
     * `viewOffset` into account to translate content-space coordinates to screen-space.
     *
     * @param renderer The SDL_Renderer to draw with.
     * @param viewOffset The screen-space offset inherited from parent containers (e.g., ScrollView).
     */
    virtual void draw(SDL_Renderer* renderer, const SDL_FPoint& viewOffset) = 0;

    /**
     * @brief Recalculates the control's position and dimensions based on parent constraints.
     *
     * This method resolves the control's layout parameters (PositionParams) into final
     * content-space coordinates (x, y, width, height).
     *
     * @param parentWidth The width of the parent's content area.
     * @param parentHeight The height of the parent's view area (used for anchoring).
     */
    virtual void recalculateLayout(int parentWidth, int parentHeight) = 0;

    /**
     * @brief Gets the content-space bounding box of the control.
     *
     * This rectangle defines the final, resolved position (x, y) and size (w, h)
     * of the control relative to its parent's content origin.
     *
     * @return An SDL_FRect representing the control's position and size in content-space.
     */
    virtual SDL_FRect getBounds() const = 0;

    // --- Interaction and State Management Methods ---

    /**
     * @brief Checks if a given point is within the control's content-space bounds.
     *
     * Used primarily for hit testing mouse clicks or touch events.
     *
     * @param point The point to check, expected to be in the same coordinate space as getBounds().
     * @return true if the point is inside the control's bounding box.
     */
    virtual bool isInside(const SDL_FPoint& point) const {
        const SDL_FRect bounds = getBounds();
        return (point.x >= bounds.x && point.x < (bounds.x + bounds.w) &&
                point.y >= bounds.y && point.y < (bounds.y + bounds.h));
    }

    /**
     * @brief Gives input focus to the control (single-argument form).
     *
     * Derived classes (e.g., InputBox) should override this to handle focus events,
     * such as activating the SDL software keyboard or starting text input.
     *
     * @param window The parent SDL_Window context, potentially used for SDL_StartTextInput.
     */
    virtual void focus(SDL_Window* /*window*/) {
        // Default implementation simply updates the focus flag
        m_hasFocus = true;
    }

    /**
     * @brief Gives input focus to the control (two-argument form).
     *
     * This version is preferred as it passes necessary context to the control.
     * The default implementation sets the window/viewOffset hooks and then calls
     * the single-argument `focus(window)` for compatibility with older overrides.
     *
     * @param window The parent SDL_Window context.
     * @param viewOffset The current screen-space offset of the control's container.
     */
    virtual void focus(SDL_Window* window, const SDL_FPoint& viewOffset) {
        // Set context hooks first
        setWindow(window);
        setViewOffset(viewOffset);

        // Forward to the single-arg overload so existing derived-class overrides continue to work.
        focus(window);
    }

    /**
     * @brief Removes input focus from the control (single-argument form).
     *
     * Derived classes (e.g., InputBox) should override this to handle unfocus events,
     * such as stopping text input or finalizing data entry.
     *
     * @param window The parent SDL_Window context, potentially used for SDL_StopTextInput.
     */
    virtual void unfocus(SDL_Window* /*window*/) {
        // Default implementation simply updates the focus flag
        m_hasFocus = false;
    }

    /**
     * @brief Removes input focus from the control (two-argument form).
     *
     * Default implementation updates the context hooks and calls the single-argument `unfocus(window)`.
     *
     * @param window The parent SDL_Window context.
     * @param viewOffset The current screen-space offset of the control's container.
     */
    virtual void unfocus(SDL_Window* window, const SDL_FPoint& viewOffset) {
        // Update stored offset if needed by derived classes
        setViewOffset(viewOffset);
        // Forward to the single-arg overload
        unfocus(window);
    }


    /**
     * @brief Returns the current input focus state of the control.
     * @return true if the control has focus, false otherwise.
     */
    virtual bool hasFocus() const {
        return m_hasFocus;
    }

    // --- Optional Context Hooks for Containers ---

    /**
     * @brief Optional hook for a container to provide the current SDL_Window.
     *
     * Derived classes can override this to store the window pointer if required
     * for input handling (e.g., SDL_StartTextInput, clipboard access).
     *
     * @param window The parent window context.
     */
    virtual void setWindow(SDL_Window* /*window*/) { /* optional override */ }

    /**
     * @brief Optional hook for a container to provide the current view offset.
     *
     * Derived classes can override this to store the offset, especially if they
     * need to translate coordinates for input (e.g., managing a cursor position
     * relative to a scrolling view).
     *
     * @param viewOffset The current screen-space offset of the container.
     */
    virtual void setViewOffset(const SDL_FPoint& /*viewOffset*/) { /* optional override */ }

    /**
     * @brief Handles incoming SDL events with additional context (window and offset).
     *
     * This provides a complete context for event handling within container hierarchies.
     * The default implementation ignores the context and forwards the event to the
     * simpler, pure virtual `handleEvent(const SDL_Event& e)` to keep derived classes simple.
     *
     * @param e The SDL_Event to process.
     * @param window The parent SDL_Window context.
     * @param viewOffset The screen-space offset of the control's parent container.
     * @return true if the event was handled and resulted in a state change, false otherwise.
     */
    virtual bool handleEvent(const SDL_Event& e, SDL_Window* window, const SDL_FPoint& viewOffset){
        // Suppress unused parameter warnings explicitly for the default implementation
        (void)window;
        (void)viewOffset;
        // By default forward to the single-arg handler to keep derived classes simple
        return handleEvent(e);
    }
};