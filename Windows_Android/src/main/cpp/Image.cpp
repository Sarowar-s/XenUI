// SPDX-License-Identifier: Apache-2.0
/*
 *  Copyright (C) 2025 MD S M Sarowar Hossain
 *
 * 
 */
// Implements the Image resource class (SDL_Texture wrapper) and the Retained-mode
// ImageControl, along with Immediate-mode utility functions for drawing images.
//
#include "Image.h"
#include <SDL3/SDL_log.h>

#include <unordered_map>
#include <utility>
#include <memory>
#include <stdexcept> 

// ---------------- Image resource class implementation Starts ----------------

/**
 * @brief Constructs an Image object and attempts to load the texture resource.
 *
 * Initializes the image by loading an SDL_Texture from the specified file path.
 *
 * @param renderer The SDL_Renderer used to create the texture.
 * @param filePath The path to the image file to load.
 */
Image::Image(SDL_Renderer* renderer, const std::string& filePath)
    : m_texture(nullptr), m_width(0), m_height(0)
{
    // Log an error if the renderer context is invalid
    if (!renderer) {
        SDL_Log("Image: Renderer is null");
        return;
    }
    // Attempt to load the texture using the internal helper function
    m_texture = loadTexture(renderer, filePath);
    if (m_texture) {
        // Log success and image dimensions if loaded
        if (m_width > 0 && m_height > 0) {
            SDL_Log("Image: \"%s\" → texture loaded, size = %d x %d",
                    filePath.c_str(), m_width, m_height);
        } else {
            // Warn if the texture loaded but dimensions are invalid (shouldn't happen with valid image file)
            SDL_Log("Image: \"%s\" → texture loaded but width/height are zero. (Check asset.)",
                    filePath.c_str());
        }
    } else {
        // Log failure to load the texture
        SDL_Log("Image: Failed to load texture from \"%s\"", filePath.c_str());
    }
}

/**
 * @brief Destructor. Destroys the managed SDL_Texture resource.
 */
Image::~Image() {
    // Safely destroy the texture if it was loaded
    if (m_texture) SDL_DestroyTexture(m_texture);
}

/**
 * @brief Move constructor. Transfers ownership of the SDL_Texture from the source.
 *
 * @param other The Image object to move resources from.
 */
Image::Image(Image&& other) noexcept
    : m_texture(other.m_texture), m_width(other.m_width), m_height(other.m_height)
{
    // Nullify the state of the source object after move
    other.m_texture = nullptr;
    other.m_width = 0;
    other.m_height = 0;
}

/**
 * @brief Move assignment operator. Transfers ownership, destroying any existing resource.
 *
 * @param other The Image object to move resources from.
 * @return A reference to the current object.
 */
Image& Image::operator=(Image&& other) noexcept {
    if (this != &other) {
        // Destroy existing resource before transfer
        if (m_texture) SDL_DestroyTexture(m_texture);
        // Transfer resources
        m_texture = other.m_texture;
        m_width = other.m_width;
        m_height = other.m_height;
        // Nullify the state of the source object after move
        other.m_texture = nullptr;
        other.m_width = 0;
        other.m_height = 0;
    }
    return *this;
}

/**
 * @brief Internal utility function to load an SDL_Texture from a file path.
 *
 * Handles file I/O, surface creation using SDL_image, and texture creation.
 * It also updates the internal m_width and m_height fields upon success.
 *
 * @param renderer The SDL_Renderer context.
 * @param filePath The path to the image file.
 * @return The created SDL_Texture pointer on success, or nullptr on failure.
 */
SDL_Texture* Image::loadTexture(SDL_Renderer* renderer, const std::string& filePath)
{
    // Open file using SDL_IO (necessary for cross-platform asset handling)
    SDL_IOStream* rw = SDL_IOFromFile(filePath.c_str(), "rb");
    if (!rw) {
        SDL_Log("Image: Cannot open file \"%s\": %s", filePath.c_str(), SDL_GetError());
        return nullptr;
    }

    // Load surface from file I/O using SDL_image; auto-free flag is set
    SDL_Surface* surface = IMG_Load_IO(rw, /*auto-free=*/1);
    if (!surface) {
        SDL_Log("Image: IMG_Load_RW failed for \"%s\": %s", filePath.c_str(), SDL_GetError());
        return nullptr;
    }

    // Store dimensions from the loaded surface
    m_width = surface->w;
    m_height = surface->h;

    // Create hardware-accelerated texture from the surface
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    // The surface is no longer needed
    SDL_DestroySurface(surface);

    if (!texture) {
        // Log failure if texture creation fails
        SDL_Log("Image: SDL_CreateTextureFromSurface failed for \"%s\": %s", filePath.c_str(), SDL_GetError());
        m_width = m_height = 0;
    }

    return texture;
}

/**
 * @brief Renders the image texture to the renderer with various transformations.
 *
 * @param renderer The SDL_Renderer context to draw to.
 * @param x The X coordinate of the top-left corner (in integer screen-space coordinates).
 * @param y The Y coordinate of the top-left corner (in integer screen-space coordinates).
 * @param scaleX Horizontal scaling factor.
 * @param scaleY Vertical scaling factor.
 * @param angle Rotation angle in degrees (clockwise).
 * @param clip A pointer to the source SDL_Rect to clip the texture (nullptr for full texture).
 * @param rotationCenter A pointer to the SDL_FPoint for the rotation center (relative to destination rectangle).
 * @param flip The SDL_FlipMode (horizontal, vertical, or none).
 */
void Image::render(SDL_Renderer* renderer,
                   int x, int y,
                   float scaleX, float scaleY,
                   double angle,
                   SDL_Rect* clip,
                   SDL_FPoint* rotationCenter,
                   SDL_FlipMode flip) const
{
    // Pre-check for null pointers
    if (!m_texture) {
        SDL_Log("Image::render failed: texture is null");
        return;
    }
    if (!renderer) {
        SDL_Log("Image::render failed: renderer is null");
        return;
    }

    SDL_FRect dest;
    dest.x = static_cast<float>(x);
    dest.y = static_cast<float>(y);

    // Calculate destination width and height based on clip or full texture size and scale
    if (clip) {
        dest.w = static_cast<float>(clip->w) * scaleX;
        dest.h = static_cast<float>(clip->h) * scaleY;
    } else {
        dest.w = static_cast<float>(m_width) * scaleX;
        dest.h = static_cast<float>(m_height) * scaleY;
    }

    // Skip rendering if dimensions are non-positive
    if (dest.w <= 0.0f || dest.h <= 0.0f) {
        SDL_Log("Image::render aborted: w/h is zero (%.1f×%.1f)", dest.w, dest.h);
        return;
    }

    // Prepare source rectangle for texture clipping
    SDL_FRect srcFRect;
    const SDL_FRect* pSrcFRect = nullptr;
    if (clip) {
        // Use a floating-point source rect if clipping is required
        srcFRect.x = static_cast<float>(clip->x);
        srcFRect.y = static_cast<float>(clip->y);
        srcFRect.w = static_cast<float>(clip->w);
        srcFRect.h = static_cast<float>(clip->h);
        pSrcFRect = &srcFRect;
    }

    // Use appropriate SDL rendering function (optimized path for no rotation/flip)
    if (angle == 0.0 && flip == SDL_FLIP_NONE) {
        if (SDL_RenderTexture(renderer, m_texture, pSrcFRect, &dest) != 0) {
            const char* err = SDL_GetError();
            if (err && *err) SDL_Log("Image::render failed (SDL_RenderTexture): %s", err);
        }
    } else {
        // Use rotated function for rotation or flipping
        if (SDL_RenderTextureRotated(renderer, m_texture, pSrcFRect, &dest,
                                     angle, rotationCenter, flip) != 0) {
            const char* err = SDL_GetError();
            if (err && *err) SDL_Log("Image::render failed (SDL_RenderTextureRotated): %s", err);
        }
    }
}

/**
 * @brief Sets the alpha modulation value for the image texture.
 *
 * @param alpha The alpha value (0 to 255).
 * @return true on success, false if the texture is null or setting failed.
 */
bool Image::setAlpha(Uint8 alpha) {
    if (!m_texture) return false;
    return SDL_SetTextureAlphaMod(m_texture, alpha) == 0;
}

/**
 * @brief Sets the color modulation value for the image texture.
 *
 * @param r Red color component (0 to 255).
 * @param g Green color component (0 to 255).
 * @param b Blue color component (0 to 255).
 * @return true on success, false if the texture is null or setting failed.
 */
bool Image::setColorMod(Uint8 r, Uint8 g, Uint8 b) {
    if (!m_texture) return false;
    return SDL_SetTextureColorMod(m_texture, r, g, b) == 0;
}

/**
 * @brief Sets the blending mode for the image texture.
 *
 * @param blendMode The SDL_BlendMode to use.
 * @return true on success, false if the texture is null or setting failed.
 */
bool Image::setBlendMode(SDL_BlendMode blendMode) {
    if (!m_texture) return false;
    return SDL_SetTextureBlendMode(m_texture, blendMode) == 0;
}

// ---------------- Image resource class implementation Ends ----------------


// ---------------- Retained-mode ImageControl Starts ----------------

/**
 * @brief Constructs an ImageControl, which manages an Image resource in the Retained Mode UI hierarchy.
 *
 * Loads the image and performs an initial layout calculation.
 *
 * @param renderer The SDL_Renderer context used to load the image.
 * @param filePath The path to the image file.
 * @param posParams Positioning parameters for the control.
 * @param desiredWidth The target width for the image. If >0, overrides scaleX.
 * @param desiredHeight The target height for the image. If >0, overrides scaleY.
 * @param scaleX Initial horizontal scaling factor (used if desiredWidth/H are 0).
 * @param scaleY Initial vertical scaling factor (used if desiredWidth/H are 0).
 * @param angle Rotation angle.
 * @param clip Source rectangle for clipping (optional).
 * @param rotationCenter Rotation center (optional).
 * @param flip Flip mode (optional).
 */
ImageControl::ImageControl(SDL_Renderer* renderer,
                           const std::string& filePath,
                           const XenUI::PositionParams& posParams,
                           float desiredWidth,
                           float desiredHeight,
                           float scaleX,
                           float scaleY,
                           double angle,
                           SDL_Rect* clip,
                           SDL_FPoint* rotationCenter,
                           SDL_FlipMode flip)
    // Initialize the unique_ptr with the newly loaded Image object
    : m_image(std::make_unique<Image>(renderer, filePath)),
      m_posParams(posParams),
      m_path(filePath),
      m_desiredW(desiredWidth),
      m_desiredH(desiredHeight),
      m_scaleX(scaleX),
      m_scaleY(scaleY),
      m_angle(angle),
      m_clip(clip),
      m_rotationCenter(rotationCenter),
      m_flip(flip)
{
    // Perform initial layout calculation to set position and size
    recalculateLayout();
}

/**
 * @brief Handles incoming SDL events.
 *
 * @param e The SDL_Event to process.
 * @return false as images typically do not change state based on events.
 */
bool ImageControl::handleEvent(const SDL_Event& /*e*/) {
    // By default, a simple ImageControl does not consume or change state based on events.
    return false;
}

/**
 * @brief Recalculates the control's final display size and its position in content-space.
 *
 * The final size is determined by native image size, desired dimensions, and scale factors,
 * prioritizing desired dimensions to enforce size or maintain aspect ratio.
 *
 * @param parentWidth The width of the control's parent/content area.
 * @param parentHeight The height of the control's parent/content area.
 */
void ImageControl::recalculateLayout(int parentWidth, int parentHeight) {
    // Compute final size based on image native size and desired dims
    if (!m_image || !m_image->isLoaded()) {
        // If image failed to load, size is zero
        m_finalW = m_finalH = 0.0f;
    } else {
        float nativeW = float(m_image->getWidth());
        float nativeH = float(m_image->getHeight());

        // Start with the initial scale factors
        float sx = m_scaleX;
        float sy = m_scaleY;

        // Apply desired width/height, which overrides explicit scale factors
        if (m_desiredW > 0.0f && m_desiredH > 0.0f) {
            // Both desired, calculate scales independently
            sx = m_desiredW / nativeW;
            sy = m_desiredH / nativeH;
        } else if (m_desiredW > 0.0f) {
            // Desired width only: calculate scaleX and apply to scaleY to preserve aspect ratio
            sx = m_desiredW / nativeW;
            sy = sx;
        } else if (m_desiredH > 0.0f) {
            // Desired height only: calculate scaleY and apply to scaleX to preserve aspect ratio
            sy = m_desiredH / nativeH;
            sx = sy;
        }

        // Calculate final dimensions
        m_finalW = nativeW * sx;
        m_finalH = nativeH * sy;
    }

    // Compute content-space position using layout parameters and the final size
    // Note: Cast size to int for position calculation utilities that rely on integer bounds
    SDL_Point final = XenUI::CalculateFinalPosition(m_posParams, int(m_finalW > 0 ? m_finalW : 0), int(m_finalH > 0 ? m_finalH : 0), parentWidth, parentHeight);
    m_posX = final.x;
    m_posY = final.y;
}

/**
 * @brief Renders the image control to the screen.
 *
 * @param renderer The SDL_Renderer context.
 * @param viewOffset The offset applied by a parent (e.g., scroll view).
 */
void ImageControl::draw(SDL_Renderer* renderer, const SDL_FPoint& viewOffset) {
    if (!m_image || !m_image->isLoaded() || !renderer) return;

    // Re-calculate effective scale factors just before drawing, in case layout hasn't been called this frame
    float nativeW = float(m_image->getWidth());
    float nativeH = float(m_image->getHeight());
    float sx = m_scaleX, sy = m_scaleY;
    if (m_desiredW > 0.0f && m_desiredH > 0.0f) {
        sx = m_desiredW / nativeW;
        sy = m_desiredH / nativeH;
    } else if (m_desiredW > 0.0f) {
        sx = m_desiredW / nativeW;
        sy = sx;
    } else if (m_desiredH > 0.0f) {
        sy = m_desiredH / nativeH;
        sx = sy;
    }

    // Convert content-space position (m_posX/Y) to screen-space position
    int drawX = int(float(m_posX) + viewOffset.x);
    int drawY = int(float(m_posY) + viewOffset.y);
    // Render the underlying Image resource
    m_image->render(renderer, drawX, drawY, sx, sy, m_angle, m_clip, m_rotationCenter, m_flip);
}

/**
 * @brief Retrieves the bounding rectangle of the control in content-space.
 *
 * @return The SDL_FRect defining the control's bounds (position and final size).
 */
SDL_FRect ImageControl::getBounds() const {
    return { float(m_posX), float(m_posY), m_finalW, m_finalH };
}

// ---------------- Retained-mode ImageControl Ends ----------------


// ---------------- Immediate-mode cached Image helper Starts ----------------

namespace {
    // Static storage for cached Image resources used by the immediate-mode DrawImage functions.
    // The key is typically the file path or a unique ID.
    static std::unordered_map<std::string, std::unique_ptr<Image>> g_imageCache;
}

namespace XenUI{
    /**
     * @brief Immediate-mode function to draw an image, automatically handling window size for layout.
     *
     * This variant uses the current window dimensions as the parent dimensions for position calculation.
     *
     * @param cacheKey A unique ID used to cache the loaded SDL_Texture (often the file path).
     * @param renderer The SDL_Renderer context.
     * @param filePath The path to the image file.
     * @param posParams Positioning parameters.
     * @param viewOffset The scroll offset to apply to the final draw position.
     * @param desiredWidth The target width for the image (overrides scaleX).
     * @param desiredHeight The target height for the image (overrides scaleY).
     * @param scaleX Initial horizontal scaling factor.
     * @param scaleY Initial vertical scaling factor.
     * @param angle Rotation angle.
     * @param clip Source rectangle for clipping (optional).
     * @param rotationCenter Rotation center (optional).
     * @param flip Flip mode (optional).
     * @return true if the image was successfully loaded and rendered, false otherwise.
     */
    bool DrawImage(const std::string& cacheKey,
               SDL_Renderer* renderer,
               const std::string& filePath,
               const PositionParams& posParams,
               const SDL_FPoint& viewOffset,
               float desiredWidth,
               float desiredHeight,
               float scaleX,
               float scaleY,
               double angle,
               SDL_Rect* clip,
               SDL_FPoint* rotationCenter,
               SDL_FlipMode flip)
    {
        int w, h;
        // Get the current render output size (typically the window size) to use as parent bounds
        SDL_GetCurrentRenderOutputSize(renderer, &w, &h);
        // Delegate to the parent-aware variant
        return DrawImage(cacheKey, renderer, filePath, posParams, w, h,
                         viewOffset, desiredWidth, desiredHeight, scaleX, scaleY,
                         angle, clip, rotationCenter, flip);
    }

    /**
     * @brief Immediate-mode function to draw an image, explicitly taking parent dimensions for layout.
     *
     * @param cacheKey A unique ID used to cache the loaded SDL_Texture.
     * @param renderer The SDL_Renderer context.
     * @param filePath The path to the image file.
     * @param posParams Positioning parameters.
     * @param parentWidth The width of the containing parent element/area.
     * @param parentHeight The height of the containing parent element/area.
     * @param viewOffset The scroll offset.
     * @param desiredWidth The target width for the image (overrides scaleX).
     * @param desiredHeight The target height for the image (overrides scaleY).
     * @param scaleX Initial horizontal scaling factor.
     * @param scaleY Initial vertical scaling factor.
     * @param angle Rotation angle.
     * @param clip Source rectangle for clipping (optional).
     * @param rotationCenter Rotation center (optional).
     * @param flip Flip mode (optional).
     * @return true if the image was successfully loaded and rendered, false otherwise.
     */
    bool DrawImage(const std::string& cacheKey,
               SDL_Renderer* renderer,
               const std::string& filePath,
               const XenUI::PositionParams& posParams,
               int parentWidth,
               int parentHeight,
               const SDL_FPoint& viewOffset,
               float desiredWidth,
               float desiredHeight,
               float scaleX,
               float scaleY,
               double angle,
               SDL_Rect* clip,
               SDL_FPoint* rotationCenter,
               SDL_FlipMode flip)
    {
        // 1. Check/load image from cache
        auto it = g_imageCache.find(cacheKey);
        if (it == g_imageCache.end()) {
            // Image not in cache, load it now
            auto img = std::make_unique<Image>(renderer, filePath);
            if (!img->isLoaded()) return false;
            // Insert into cache and get the iterator to the new element
            it = g_imageCache.insert({cacheKey, std::move(img)}).first;
        }

        Image* img = it->second.get();
        if (!img || !img->isLoaded()) return false;

        // 2. Calculate final scale factors
        float nativeW = float(img->getWidth());
        float nativeH = float(img->getHeight());
        float sx = scaleX, sy = scaleY;
        if (desiredWidth > 0.0f && desiredHeight > 0.0f) {
            sx = desiredWidth / nativeW;
            sy = desiredHeight / nativeH;
        } else if (desiredWidth > 0.0f) {
            sx = desiredWidth / nativeW; sy = sx; // Preserve aspect ratio
        } else if (desiredHeight > 0.0f) {
            sy = desiredHeight / nativeH; sx = sy; // Preserve aspect ratio
        }

        // 3. Calculate position (content-space)
        // Pass the *resulting* scaled size to the position calculation utility
        SDL_Point finalPos = XenUI::CalculateFinalPosition(posParams,
            int(nativeW * sx + 0.5f), // Final width (rounded to nearest int)
            int(nativeH * sy + 0.5f), // Final height (rounded to nearest int)
            parentWidth,
            parentHeight);

        // 4. Convert to screen-space for drawing
        float screenX = float(finalPos.x) + viewOffset.x;
        float screenY = float(finalPos.y) + viewOffset.y;

        // 5. Render
        // Convert screen position back to integer for the render call
        img->render(renderer, int(screenX + 0.5f), int(screenY + 0.5f), sx, sy, angle, clip, rotationCenter, flip);
        return true;
    }

} // namespace XenUI
// ---------------- Immediate-mode cached Image helper Ends ----------------