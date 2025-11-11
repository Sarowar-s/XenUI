// Checkbox.h
//
// Copyright (c) 2025 MD S M Sarowar Hossain
//  
//
// Defines the Checkbox control structure, styling, and the interface for both
// retained mode (Checkbox class) and immediate mode (XenUI::Checkbox function).
//
#ifndef CHECKBOX_H
#define CHECKBOX_H

#include <SDL3/SDL.h>
#include <string>
#include <functional>
#include "Position.h"     // For XenUI::PositionParams, used for layout calculation.
#include "TextRenderer.h" // For TextRenderer class (extern declaration).
#include "WindowUtil.h"   // For XenUI::CalculateFinalPosition and XenUI::GetWindowSize.
#include "UIElement.h"    // IControl interface: provides common UI element methods.

// Ensure the global textRenderer instance is declared
// auto& tr = TextRenderer::getInstance();

// Default font size used if none is specified in the constructor or immediate mode call.
#ifndef DEFAULT_CHECKBOX_FONT_SIZE
#define DEFAULT_CHECKBOX_FONT_SIZE 20
#endif

// --- Checkbox Style ---
/**
 * @brief Defines the visual style properties for a Checkbox control.
 *
 * This structure holds color definitions and size parameters necessary for rendering
 * the checkbox in its various states (default, hover, pressed).
 */
struct CheckboxStyle {
    // Box fill colors for different states
    SDL_Color boxBgColor         = {50, 50, 50, 255};      ///< Default background color of the box.
    SDL_Color boxBorderColor     = {150, 150, 150, 255};   ///< Color of the box border.
    SDL_Color boxHoverColor      = {70, 70, 70, 255};      ///< Background color when the mouse hovers over the control.
    SDL_Color boxPressedColor    = {30, 30, 30, 255};      ///< Background color when the control is pressed.
    // Checkmark and label colors
    SDL_Color checkmarkColor     = {10, 200, 100, 255};    ///< Color of the checkmark symbol.
    SDL_Color labelColor         = {255, 255, 255, 255};   ///< Color of the text label.

    // Sizing and spacing parameters
    int boxSize          = 20;   ///< Width and height of the square checkbox box (in pixels).
    int checkmarkThickness = 2;  ///< Thickness of the checkmark lines (in pixels).
    int paddingX         = 8;    ///< Horizontal padding: between box/label and around the entire component.
    int paddingY         = 4;    ///< Vertical padding around the entire component.
};

// --------------------- Retained Mode Checkbox ---------------------
/**
 * @brief Represents a Checkbox UI control implemented using the Retained Mode paradigm.
 *
 * This class inherits from IControl and maintains its own state, handles events,
 * and manages its layout calculation.
 */
class Checkbox : public IControl {
public:
    /**
     * @brief Constructor for the Checkbox control.
     *
     * @param label The text displayed next to the checkbox.
     * @param posParams Parameters for resolving the control's position relative to its parent.
     * @param initialState The initial checked state (default is false).
     * @param style The visual style parameters (default is an empty, default-initialized style).
     * @param fontSize Font size for the label (default uses DEFAULT_CHECKBOX_FONT_SIZE).
     * @param onToggle A callback function executed when the checkbox state changes.
     */
    Checkbox(const std::string& label,
             const XenUI::PositionParams& posParams,
             bool initialState = false,
             CheckboxStyle style = {},
             int fontSize = DEFAULT_CHECKBOX_FONT_SIZE,
             std::function<void(bool)> onToggle = nullptr);

    // IControl interface implementation:

    /**
     * @brief Processes an SDL event to update the checkbox state (hover, press, checked).
     * @param e The SDL_Event to be handled.
     * @return true if the control's state was modified, false otherwise.
     */
    bool handleEvent(const SDL_Event& e) override;

    /**
     * @brief Renders the checkbox and its label to the screen.
     * @param renderer The SDL_Renderer context.
     * @param viewOffset The offset applied to local coordinates (e.g., from a ScrollView).
     */
    void draw(SDL_Renderer* renderer, const SDL_FPoint& viewOffset) override;

    /**
     * @brief Forces a recalculation of the control's internal layout and dimensions.
     *
     * @param parentWidth The width of the control's parent/content area.
     * @param parentHeight The height of the control's parent/content area.
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
     * @brief Retrieves the control's bounding rectangle in local (content) coordinates.
     * @return The SDL_FRect defining the control's bounds.
     */
    SDL_FRect getBounds() const override { return m_bounds; }

    /**
     * @brief Backwards-compatible drawing call assuming no scroll offset.
     * @param renderer The SDL_Renderer context.
     */
    void draw(SDL_Renderer* renderer) { draw(renderer, {0.0f, 0.0f}); }

    // State accessors and mutators
    /**
     * @brief Returns the current checked state of the checkbox.
     * @return true if checked, false otherwise.
     */
    bool isChecked() const { return m_isChecked; }

    /**
     * @brief Manually sets the checked state of the checkbox.
     * @param checked The new state.
     */
    void setChecked(bool checked) { m_isChecked = checked; }

private:
    /**
     * @brief Internal helper function to draw the checkmark symbol.
     * @param renderer The SDL_Renderer context.
     * @param boxRect The absolute screen rectangle of the checkbox box.
     */
    void drawCheckmark(SDL_Renderer* renderer, const SDL_FRect& boxRect) const;

    // Control configuration data
    std::string m_label;                          ///< Text displayed next to the box.
    XenUI::PositionParams m_posParams;            ///< Parameters for position calculation.
    CheckboxStyle m_style;                        ///< Visual style attributes.
    int m_fontSize;                               ///< Font size for the label.
    std::function<void(bool)> m_onToggleCallback; ///< Function called upon state change.

    // Internal state variables
    bool m_isChecked;   ///< Current checked state.
    bool m_isHovered;   ///< Mouse is currently over the control.
    bool m_isPressed;   ///< Mouse button is currently pressed over the control.

    // Layout/Geometry data (local coordinates)
    SDL_FRect m_bounds;     ///< Total bounding box of the control (box + label + padding).
    SDL_FRect m_boxRect;    ///< Bounding box of just the square box.
    SDL_FPoint m_labelPos;  ///< Draw position for the label text (baseline y).

    // Measured label text dimensions
    float m_textWidth;
    float m_textHeight;
};

// --------------------- Retained Mode Checkbox Ends ---------------------
// -----------------------------------------------------------------------


// -----------Immediate mode Checkbox starts-----
namespace XenUI {
    /**
     * @brief Immediate-mode function to draw and handle a Checkbox.
     *
     * This function is stateless in the framework but uses a static map internally
     * to track press events across frames for proper click detection. It computes
     * layout, handles input, updates the provided 'isChecked' pointer, and draws
     * the control in a single call.
     *
     * @param id A unique string identifier required for transient state tracking (e.g., press state).
     * @param label The text displayed next to the checkbox.
     * @param isChecked Pointer to the boolean variable holding the check state that will be toggled on click.
     * @param posParams Parameters for resolving the control's position.
     * @param style The visual style parameters (defaults to default CheckboxStyle).
     * @param fontSize Font size for the label (defaults to DEFAULT_CHECKBOX_FONT_SIZE).
     * @param viewOffset The coordinate space offset to apply (default is {0.0f, 0.0f}).
     * @param parentWidth The width of the parent container for layout (default is -1, uses window width).
     * @param parentHeight The height of the parent container for layout (default is -1, uses window height).
     * @return true if the check state (*isChecked) was changed during this call, false otherwise.
     */
    bool Checkbox(const char* id,
              const std::string& label,
              bool* isChecked,
              const PositionParams& posParams,
              const CheckboxStyle& style = {},
              int fontSize = DEFAULT_CHECKBOX_FONT_SIZE,

              const SDL_FPoint& viewOffset = {0.0f, 0.0f},
              int parentWidth = -1,
              int parentHeight = -1);

}
// -----------Immediate mode Checkbox ends-----


#endif // CHECKBOX_H