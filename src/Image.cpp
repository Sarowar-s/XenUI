// SPDX-License-Identifier: Apache-2.0
/*
 *  Copyright (C) 2025 MD S M Sarowar Hossain
 */
//
// Implements the Image resource class (SDL_Texture wrapper) and the Retained-mode
// ImageControl, along with Immediate-mode utility functions for drawing images.
//
#include "Image.h"
#include <SDL3/SDL_log.h>
#include <SDL_image.h> 
#include <unordered_map>
#include <utility>
#include <memory>
#include <stdexcept> 


Image::Image(SDL_Renderer* renderer, const std::string& filePath)
    : m_texture(nullptr), m_width(0), m_height(0)
{
    
    if (!renderer) {
        SDL_Log("Image: Renderer is null");
        return;
    }
   
    m_texture = loadTexture(renderer, filePath);
    if (m_texture) {
      
        if (m_width > 0 && m_height > 0) {
            SDL_Log("Image: \"%s\" → texture loaded, size = %d x %d",
                    filePath.c_str(), m_width, m_height);
        } else {
          
            SDL_Log("Image: \"%s\" → texture loaded but width/height are zero. (Check asset.)",
                    filePath.c_str());
        }
    } else {
       
        SDL_Log("Image: Failed to load texture from \"%s\"", filePath.c_str());
    }
}


Image::~Image() {
    
    if (m_texture) SDL_DestroyTexture(m_texture);
}


Image::Image(Image&& other) noexcept
    : m_texture(other.m_texture), m_width(other.m_width), m_height(other.m_height)
{
   
    other.m_texture = nullptr;
    other.m_width = 0;
    other.m_height = 0;
}


Image& Image::operator=(Image&& other) noexcept {
    if (this != &other) {
        
        if (m_texture) SDL_DestroyTexture(m_texture);
       
        // Transfer resources
        m_texture = other.m_texture;
        m_width = other.m_width;
        m_height = other.m_height;
       
        other.m_texture = nullptr;
        other.m_width = 0;
        other.m_height = 0;
    }
    return *this;
}


SDL_Texture* Image::loadTexture(SDL_Renderer* renderer, const std::string& filePath)
{
    // Open file using SDL_IO (necessary for cross-platform asset handling)
    SDL_IOStream* rw = SDL_IOFromFile(filePath.c_str(), "rb");
    if (!rw) {
        SDL_Log("Image: Cannot open file \"%s\": %s", filePath.c_str(), SDL_GetError());
        return nullptr;
    }

   
    SDL_Surface* surface = IMG_Load_IO(rw, /*auto-free=*/1);
    if (!surface) {
        SDL_Log("Image: IMG_Load_RW failed for \"%s\": %s", filePath.c_str(), SDL_GetError());
        return nullptr;
    }

  
    m_width = surface->w;
    m_height = surface->h;

   
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

    SDL_DestroySurface(surface);

    if (!texture) {
       
        SDL_Log("Image: SDL_CreateTextureFromSurface failed for \"%s\": %s", filePath.c_str(), SDL_GetError());
        m_width = m_height = 0;
    }

    return texture;
}


void Image::render(SDL_Renderer* renderer,
                   int x, int y,
                   float scaleX, float scaleY,
                   double angle,
                   SDL_Rect* clip,
                   SDL_FPoint* rotationCenter,
                   SDL_FlipMode flip) const
{
   
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

    
    if (clip) {
        dest.w = static_cast<float>(clip->w) * scaleX;
        dest.h = static_cast<float>(clip->h) * scaleY;
    } else {
        dest.w = static_cast<float>(m_width) * scaleX;
        dest.h = static_cast<float>(m_height) * scaleY;
    }

  
    if (dest.w <= 0.0f || dest.h <= 0.0f) {
        SDL_Log("Image::render aborted: w/h is zero (%.1f×%.1f)", dest.w, dest.h);
        return;
    }

  
    SDL_FRect srcFRect;
    const SDL_FRect* pSrcFRect = nullptr;
    if (clip) {
      
        srcFRect.x = static_cast<float>(clip->x);
        srcFRect.y = static_cast<float>(clip->y);
        srcFRect.w = static_cast<float>(clip->w);
        srcFRect.h = static_cast<float>(clip->h);
        pSrcFRect = &srcFRect;
    }

   
    if (angle == 0.0 && flip == SDL_FLIP_NONE) {
        if (SDL_RenderTexture(renderer, m_texture, pSrcFRect, &dest) != 0) {
            const char* err = SDL_GetError();
            if (err && *err) SDL_Log("Image::render failed (SDL_RenderTexture): %s", err);
        }
    } else {
       
        if (SDL_RenderTextureRotated(renderer, m_texture, pSrcFRect, &dest,
                                     angle, rotationCenter, flip) != 0) {
            const char* err = SDL_GetError();
            if (err && *err) SDL_Log("Image::render failed (SDL_RenderTextureRotated): %s", err);
        }
    }
}


bool Image::setAlpha(Uint8 alpha) {
    if (!m_texture) return false;
    return SDL_SetTextureAlphaMod(m_texture, alpha) == 0;
}


bool Image::setColorMod(Uint8 r, Uint8 g, Uint8 b) {
    if (!m_texture) return false;
    return SDL_SetTextureColorMod(m_texture, r, g, b) == 0;
}


bool Image::setBlendMode(SDL_BlendMode blendMode) {
    if (!m_texture) return false;
    return SDL_SetTextureBlendMode(m_texture, blendMode) == 0;
}

// ---------------- Image resource class implementation Ends ----------------


// ---------------- Retained-mode ImageControl Starts ----------------


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
   
}


bool ImageControl::handleEvent(const SDL_Event& /*e*/) {
   
    return false;
}


void ImageControl::recalculateLayout(int parentWidth, int parentHeight) {
   
    if (!m_image || !m_image->isLoaded()) {
      
        m_finalW = m_finalH = 0.0f;
    } else {
        float nativeW = float(m_image->getWidth());
        float nativeH = float(m_image->getHeight());

       
        float sx = m_scaleX;
        float sy = m_scaleY;

     
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

       
        m_finalW = nativeW * sx;
        m_finalH = nativeH * sy;
    }

   
    SDL_Point final = XenUI::CalculateFinalPosition(m_posParams, int(m_finalW > 0 ? m_finalW : 0), int(m_finalH > 0 ? m_finalH : 0), parentWidth, parentHeight);
    m_posX = final.x;
    m_posY = final.y;
}


void ImageControl::draw(SDL_Renderer* renderer, const SDL_FPoint& viewOffset) {
    if (!m_image || !m_image->isLoaded() || !renderer) return;

  
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

   
    int drawX = int(float(m_posX) + viewOffset.x);
    int drawY = int(float(m_posY) + viewOffset.y);

    m_image->render(renderer, drawX, drawY, sx, sy, m_angle, m_clip, m_rotationCenter, m_flip);
}


SDL_FRect ImageControl::getBounds() const {
    return { float(m_posX), float(m_posY), m_finalW, m_finalH };
}

// ---------------- Retained-mode ImageControl Ends ----------------


// ---------------- Immediate-mode cached Image helper Starts ----------------

namespace {
   
    static std::unordered_map<std::string, std::unique_ptr<Image>> g_imageCache;
}

namespace XenUI{
   
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
        
        SDL_GetCurrentRenderOutputSize(renderer, &w, &h);
       
        return DrawImage(cacheKey, renderer, filePath, posParams, w, h,
                         viewOffset, desiredWidth, desiredHeight, scaleX, scaleY,
                         angle, clip, rotationCenter, flip);
    }


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
     
        auto it = g_imageCache.find(cacheKey);
        if (it == g_imageCache.end()) {
          
            auto img = std::make_unique<Image>(renderer, filePath);
            if (!img->isLoaded()) return false;
           
            it = g_imageCache.insert({cacheKey, std::move(img)}).first;
        }

        Image* img = it->second.get();
        if (!img || !img->isLoaded()) return false;

        
        float nativeW = float(img->getWidth());
        float nativeH = float(img->getHeight());
        float sx = scaleX, sy = scaleY;
        if (desiredWidth > 0.0f && desiredHeight > 0.0f) {
            sx = desiredWidth / nativeW;
            sy = desiredHeight / nativeH;
        } else if (desiredWidth > 0.0f) {
            sx = desiredWidth / nativeW; sy = sx; 
        } else if (desiredHeight > 0.0f) {
            sy = desiredHeight / nativeH; sx = sy; 
        }

       
        SDL_Point finalPos = XenUI::CalculateFinalPosition(posParams,
            int(nativeW * sx + 0.5f), 
            int(nativeH * sy + 0.5f), 
            parentWidth,
            parentHeight);

      
        float screenX = float(finalPos.x) + viewOffset.x;
        float screenY = float(finalPos.y) + viewOffset.y;

       
        img->render(renderer, int(screenX + 0.5f), int(screenY + 0.5f), sx, sy, angle, clip, rotationCenter, flip);
        return true;
    }

} // namespace XenUI
// ---------------- Immediate-mode cached Image helper Ends ----------------