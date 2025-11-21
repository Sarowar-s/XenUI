// SPDX-License-Identifier: Apache-2.0
/*
 *  Copyright (C) 2025 MD S M Sarowar Hossain
 */
//
// Defines the Image class (a wrapper for SDL_Texture) and the Retained-mode
// ImageControl class, along with the Immediate-mode drawing API.
//
#ifndef IMAGE_H
#define IMAGE_H

#include <SDL3/SDL.h>
#include <SDL_image.h> // SDL_image function declarations (needed for IMG_Load_IO)
#include <string>
#include <memory>

#include "UIElement.h"   // IControl interface
#include "Position.h"    // PositionParams structure
#include "WindowUtil.h"  // For XenUI::GetWindowSize (and likely CalculateFinalPosition implementation)

/**
 * @brief Manages an SDL_Texture resource, including loading, destruction, and rendering parameters.
 *
 * This class handles the low-level graphics resource management for images.
 */
class Image {
public:
    /**
     * @brief Loads an image texture from a file path.
     *
     * @param renderer The SDL_Renderer context used to create the texture.
     * @param filePath The path to the image file.
     */
    Image(SDL_Renderer* renderer, const std::string& filePath);

    /**
     * @brief Destructor. Destroys the managed SDL_Texture resource.
     */
    ~Image();

    // The Image class manages a raw SDL_Texture pointer and should not be copied.
    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;

    // Move operations are enabled for efficient resource transfer.
    /**
     * @brief Move constructor. Transfers ownership of the texture resource.
     * @param other The source Image object.
     */
    Image(Image&& other) noexcept;

    /**
     * @brief Move assignment operator. Destroys current texture and transfers ownership from source.
     * @param other The source Image object.
     * @return Reference to the target object.
     */
    Image& operator=(Image&& other) noexcept;

    /**
     * @brief Renders the image texture to the screen with transformations.
     *
     * @param renderer The SDL_Renderer context to draw to.
     * @param x The X position of the destination rectangle (screen-space, integer).
     * @param y The Y position of the destination rectangle (screen-space, integer).
     * @param scaleX Horizontal scaling factor (default 1.0f).
     * @param scaleY Vertical scaling factor (default 1.0f).
     * @param angle Rotation angle in degrees clockwise (default 0.0).
     * @param clip A pointer to the source SDL_Rect for clipping (nullptr for full texture, default nullptr).
     * @param rotationCenter A pointer to the SDL_FPoint for the rotation center (relative to the destination rectangle, default nullptr).
     * @param flip The SDL_FlipMode (horizontal, vertical, or none, default SDL_FLIP_NONE).
     */
    void render(SDL_Renderer* renderer,
                int x, int y,
                float scaleX = 1.0f, float scaleY = 1.0f,
                double angle = 0.0,
                SDL_Rect* clip = nullptr,
                SDL_FPoint* rotationCenter = nullptr,
                SDL_FlipMode flip = SDL_FLIP_NONE) const;

    /**
     * @brief Sets the alpha transparency modulation for the texture.
     * @param alpha The alpha value (0 to 255).
     * @return true on success, false otherwise.
     */
    bool setAlpha(Uint8 alpha);

    /**
     * @brief Sets the color modulation for the texture.
     * @param r Red component (0 to 255).
     * @param g Green component (0 to 255).
     * @param b Blue component (0 to 255).
     * @return true on success, false otherwise.
     */
    bool setColorMod(Uint8 r, Uint8 g, Uint8 b);

    /**
     * @brief Sets the blend mode for the texture.
     * @param blendMode The SDL_BlendMode to use.
     * @return true on success, false otherwise.
     */
    bool setBlendMode(SDL_BlendMode blendMode);

    /**
     * @brief Checks if the texture resource was successfully loaded.
     * @return true if m_texture is valid, false otherwise.
     */
    bool isLoaded() const { return m_texture != nullptr; }

    /**
     * @brief Gets the native pixel width of the image.
     * @return The width in pixels.
     */
    int  getWidth()  const { return m_width; }

    /**
     * @brief Gets the native pixel height of the image.
     * @return The height in pixels.
     */
    int  getHeight() const { return m_height; }

    /**
     * @brief Provides direct access to the underlying SDL_Texture pointer.
     * @return The raw SDL_Texture pointer.
     */
    SDL_Texture* getSdlTexture() const { return m_texture; }

private:
    /**
     * @brief Internal helper to perform file loading and texture creation.
     * @param renderer The SDL_Renderer context.
     * @param filePath The path to the image file.
     * @return The created SDL_Texture pointer.
     */
    SDL_Texture* loadTexture(SDL_Renderer* renderer, const std::string& filePath);

    SDL_Texture* m_texture; ///< The actual hardware texture resource.
    int          m_width;   ///< Native width of the texture.
    int          m_height;  ///< Native height of the texture.
};

// ---------- Retained-mode wrapper control Starts ----------
/**
 * @brief A Retained Mode UI control that wraps the Image resource class.
 *
 * Handles positioning, scaling logic, and rendering within the XenonUI IControl hierarchy.
 */
class ImageControl : public IControl {
public:
    /**
     * @brief Constructs an ImageControl, loading the image resource internally.
     *
     * The final size is determined by native size, desired dimensions, and scale factors.
     * If 'desiredWidth' or 'desiredHeight' is greater than 0.0f, it will be used to
     * calculate the scale factors, potentially overriding 'scaleX/Y' or preserving aspect ratio.
     *
     * @param renderer The SDL_Renderer context.
     * @param filePath The path to the image file.
     * @param posParams Positioning parameters for layout.
     * @param desiredWidth Target width for the control (0.0f means auto-size/scale-based).
     * @param desiredHeight Target height for the control (0.0f means auto-size/scale-based).
     * @param scaleX Horizontal scaling factor (default 1.0f).
     * @param scaleY Vertical scaling factor (default 1.0f).
     * @param angle Rotation angle (default 0.0).
     * @param clip Source rectangle for clipping (optional, default nullptr).
     * @param rotationCenter Rotation center (optional, default nullptr).
     * @param flip Flip mode (optional, default SDL_FLIP_NONE).
     */
    ImageControl(SDL_Renderer* renderer,
                 const std::string& filePath,
                 const XenUI::PositionParams& posParams,
                 float desiredWidth = 0.0f,
                 float desiredHeight = 0.0f,
                 float scaleX = 1.0f,
                 float scaleY = 1.0f,
                 double angle = 0.0,
                 SDL_Rect* clip = nullptr,
                 SDL_FPoint* rotationCenter = nullptr,
                 SDL_FlipMode flip = SDL_FLIP_NONE);

    /**
     * @brief Default virtual destructor for IControl inheritance.
     */
    ~ImageControl() override = default;

    // IControl interface implementation:
    /**
     * @brief Event handling for the control.
     * @param e The SDL_Event to process.
     * @return false, as images typically do not respond to events by default.
     */
    bool handleEvent(const SDL_Event& e) override;

    /**
     * @brief Draws the image control on the screen.
     * @param renderer The SDL_Renderer context.
     * @param viewOffset The offset to apply to local coordinates (content -> screen conversion).
     */
    void draw(SDL_Renderer* renderer, const SDL_FPoint& viewOffset) override;

    /**
     * @brief Computes the control's position (m_posX/m_posY) and final display size (m_finalW/m_finalH).
     * @param parentWidth The width of the parent container.
     * @param parentHeight The height of the parent container.
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
     * @brief Returns the bounding rectangle of the control in content-space coordinates.
     * @return The SDL_FRect defining the bounds.
     */
    SDL_FRect getBounds() const override;

    // Access underlying image (if needed)
    /**
     * @brief Provides access to the underlying Image resource object.
     * @return Pointer to the Image resource.
     */
    Image* getImage() const { return m_image.get(); }

private:
    std::unique_ptr<Image> m_image;               ///< The managed image resource.
    XenUI::PositionParams  m_posParams;           ///< Parameters for position calculation.
    std::string            m_path;                ///< The original file path.

    // Layout/Drawing parameters (stored for recalculation and drawing)
    int m_posX = 0;
    int m_posY = 0;
    float m_desiredW = 0.0f;
    float m_desiredH = 0.0f;
    float m_scaleX = 1.0f;
    float m_scaleY = 1.0f;
    double m_angle = 0.0;
    SDL_Rect* m_clip = nullptr;
    SDL_FPoint* m_rotationCenter = nullptr;
    SDL_FlipMode m_flip = SDL_FLIP_NONE;

    // Computed final size in content-space
    float m_finalW = 0.0f;
    float m_finalH = 0.0f;
};
// ---------- Retained-mode wrapper control Ends ----------


// ---------- Immediate helper (cached) Starts ----------
namespace XenUI {

/**
 * @brief Immediate-mode function to draw an image, caching the resource internally.
 *
 * This overload automatically determines the parent size using the current window size
 * for position calculation (a fallback for non-nested elements).
 *
 * @param cacheKey A unique ID used to cache the loaded SDL_Texture (usually the file path).
 * @param renderer The SDL_Renderer context.
 * @param filePath The path to the image file.
 * @param posParams Positioning parameters (content-space).
 * @param viewOffset Scroll offset (converts content -> screen, default {0.0f, 0.0f}).
 * @param desiredWidth Target width for the image (0.0f means auto-size, default 0.0f).
 * @param desiredHeight Target height for the image (0.0f means auto-size, default 0.0f).
 * @param scaleX Horizontal scaling factor (default 1.0f).
 * @param scaleY Vertical scaling factor (default 1.0f).
 * @param angle Rotation angle (default 0.0).
 * @param clip Source rectangle for clipping (optional, default nullptr).
 * @param rotationCenter Rotation center (optional, default nullptr).
 * @param flip Flip mode (optional, default SDL_FLIP_NONE).
 * @return true if the image was successfully loaded and rendered, false otherwise.
 */
bool DrawImage(const std::string& cacheKey,
               SDL_Renderer* renderer,
               const std::string& filePath,
               const PositionParams& posParams,
               const SDL_FPoint& viewOffset = {0.0f, 0.0f},
               float desiredWidth = 0.0f,
               float desiredHeight = 0.0f,
               float scaleX = 1.0f,
               float scaleY = 1.0f,
               double angle = 0.0,
               SDL_Rect* clip = nullptr,
               SDL_FPoint* rotationCenter = nullptr,
               SDL_FlipMode flip = SDL_FLIP_NONE);

/**
 * @brief Immediate-mode function to draw an image, explicitly specifying parent dimensions.
 *
 * This overload is used for proper layout when drawing inside containers like scrollviews
 * or nested layouts where the parent size is known and differs from the window size.
 *
 * @param cacheKey A unique ID used to cache the loaded SDL_Texture.
 * @param renderer The SDL_Renderer context.
 * @param filePath The path to the image file.
 * @param posParams Positioning parameters (content-space).
 * @param parentWidth The explicit width of the containing parent element.
 * @param parentHeight The explicit height of the containing parent element.
 * @param viewOffset Scroll offset (converts content -> screen, default {0.0f, 0.0f}).
 * @param desiredWidth Target width for the image (default 0.0f).
 * @param desiredHeight Target height for the image (default 0.0f).
 * @param scaleX Horizontal scaling factor (default 1.0f).
 * @param scaleY Vertical scaling factor (default 1.0f).
 * @param angle Rotation angle (default 0.0).
 * @param clip Source rectangle for clipping (optional, default nullptr).
 * @param rotationCenter Rotation center (optional, default nullptr).
 * @param flip Flip mode (optional, default SDL_FLIP_NONE).
 * @return true if the image was successfully loaded and rendered, false otherwise.
 */
bool DrawImage(const std::string& cacheKey,
               SDL_Renderer* renderer,
               const std::string& filePath,
               const PositionParams& posParams,
               int parentWidth,
               int parentHeight,
               const SDL_FPoint& viewOffset = {0.0f, 0.0f},
               float desiredWidth = 0.0f,
               float desiredHeight = 0.0f,
               float scaleX = 1.0f,
               float scaleY = 1.0f,
               double angle = 0.0,
               SDL_Rect* clip = nullptr,
               SDL_FPoint* rotationCenter = nullptr,
               SDL_FlipMode flip = SDL_FLIP_NONE);

}
// ---------- Immediate helper (cached) Ends ----------


#endif // IMAGE_H