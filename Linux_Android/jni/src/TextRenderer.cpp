// SPDX-License-Identifier: Apache-2.0
/*
 *  Copyright (C) 2025 MD S M Sarowar Hossain
 */

#include "TextRenderer.h"
#include <SDL3/SDL_log.h>     
#include <fstream> 
#include <SDL_ttf.h>

TextRenderer& TextRenderer::getInstance() {
    static TextRenderer instance;
    return instance;
}

TextRenderer::TextRenderer() : m_renderer(nullptr), m_initialized(false) {}

TextRenderer::~TextRenderer() {
    clearCache();
    if (TTF_WasInit()) {
        TTF_Quit();
    }
}

bool TextRenderer::isInitialized() const {
    return m_initialized;
}

std::string TextRenderer::findBundledFallbackFont() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "TextRenderer: Searching for bundled fallback font...");
    
    const char* commonFontNames[] = {"DejaVuSans.ttf", "Roboto-Regular.ttf", "NotoSans-Regular.ttf", "Arial.ttf"};
    std::string foundFontName = "";

#ifdef __ANDROID__
    for (const char* assetName : commonFontNames) {
        std::string assetPath = "fonts/"; 
        assetPath += assetName;
        SDL_IOStream* rwops = SDL_IOFromFile(assetPath.c_str(), "rb");
        if (rwops) {
            SDL_CloseIO(rwops);
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "TextRenderer: Found bundled font in assets: %s", assetPath.c_str());
            foundFontName = assetPath;
            break;
        }
    }
    if (foundFontName.empty()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "TextRenderer: No common bundled font found in assets.");
    }
#else
    #ifdef XENUI_FALLBACK_FONT_PATH
        std::string embeddedPath = XENUI_FALLBACK_FONT_PATH;
        std::ifstream f(embeddedPath.c_str());
        if (f.good()) {
            foundFontName = embeddedPath;
        }
    #endif
    if (foundFontName.empty()) {
        const char* desktopFallback = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";
        std::ifstream f(desktopFallback);
        if (f.good()) {
            foundFontName = desktopFallback;
        }
    }
#endif
    return foundFontName;
}

void TextRenderer::init(SDL_Renderer* renderer, const std::vector<std::string>& preferredFamilies) {
    if (m_initialized) return;
    if (!renderer) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "TextRenderer init failed: Renderer is null.");
        return;
    }
    m_renderer = renderer;

    if (!TTF_Init()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_ttf could not initialize! SDL_ttf Error: %s", SDL_GetError());
        return;
    }

    m_fontPath = findBundledFallbackFont(); 

    if (m_fontPath.empty()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "ERROR: TextRenderer init failed. Could not find any suitable bundled font!");
        TTF_Quit(); 
        m_initialized = false;
        return; 
    }

    m_initialized = true;
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "TextRenderer initialized successfully. Using font: %s", m_fontPath.c_str());
}

TTF_Font* TextRenderer::getFont(int fontSize) {
    if (!m_initialized || m_fontPath.empty() || fontSize <= 0) return nullptr;

    auto it = m_fontsBySize.find(fontSize);
    if (it != m_fontsBySize.end()) {
        return it->second;
    }

    SDL_IOStream* rwops = SDL_IOFromFile(m_fontPath.c_str(), "rb");
    if (!rwops) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "getFont: Failed to open font '%s': %s", m_fontPath.c_str(), SDL_GetError());
        return nullptr;
    }
    
    TTF_Font* font = TTF_OpenFontIO(rwops, true, fontSize); 
    if (!font) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "getFont: Failed to open font size %d. Error: %s", fontSize, SDL_GetError());
        return nullptr;
    }

    m_fontsBySize[fontSize] = font;
    return font;
}

std::string TextRenderer::createCacheKey(const std::string& text, int fontSize, int wrapWidth) {
    std::stringstream ss;
    ss << text << '|' << fontSize << '|' << wrapWidth;
    return ss.str();
}

SDL_Texture* TextRenderer::renderTextToTexture(const std::string& text, SDL_Color color, int fontSize, int& outW, int& outH, int wrapWidth) {
    if (!m_initialized || text.empty()) {
        outW = 0; outH = 0;
        return nullptr;
    }

    std::string key = createCacheKey(text, fontSize, wrapWidth);
    auto cacheIt = m_textureCache.find(key);
    if (cacheIt != m_textureCache.end()) {
        outW = cacheIt->second.width; 
        outH = cacheIt->second.height;
        return cacheIt->second.texture;
    }

    TTF_Font* font = getFont(fontSize);
    if (!font) {
        outW = 0; outH = 0;
        return nullptr;
    }

    SDL_Surface* surface = nullptr;
    if (wrapWidth > 0) {
        surface = TTF_RenderText_Blended_Wrapped(font, text.c_str(), 0, color, wrapWidth);
    } else {
        surface = TTF_RenderText_Blended(font, text.c_str(), 0, color);
    }

    if (!surface) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "renderTextToTexture: Failed to create surface: %s", SDL_GetError());
        outW = 0; outH = 0;
        return nullptr;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(m_renderer, surface);
    if (!texture) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "renderTextToTexture: Failed to create texture: %s", SDL_GetError());
        outW = 0; outH = 0;
    } else {
        outW = surface->w; 
        outH = surface->h;
        m_textureCache[key] = {texture, outW, outH};
    }
    
    SDL_DestroySurface(surface); 
    return texture;
}

void TextRenderer::renderText(const std::string& text, int x, int y, SDL_Color color, int fontSize, int wrapWidth) {
    if (!m_initialized) return;
    int texW = 0, texH = 0;
    SDL_Texture* texture = renderTextToTexture(text, color, fontSize, texW, texH, wrapWidth);
    if (!texture) return;
    
    SDL_FRect dstRect = { (float)x, (float)y, (float)texW, (float)texH };
    SDL_RenderTexture(m_renderer, texture, nullptr, &dstRect); 
}

SDL_Texture* TextRenderer::renderTextImmediateToTexture(const std::string& text, SDL_Color color, int fontSize, int& outW, int& outH, int wrapWidth) {
    outW = 0; outH = 0;
    if (!m_initialized || text.empty() || fontSize <= 0) return nullptr;

    TTF_Font* font = getFont(fontSize);
    if (!font) return nullptr;

    SDL_Surface* surface = nullptr;
    if (wrapWidth > 0) {
        surface = TTF_RenderText_Blended_Wrapped(font, text.c_str(), 0, color, wrapWidth);
    } else {
        surface = TTF_RenderText_Blended(font, text.c_str(), 0, color);
    }

    if (!surface) return nullptr;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(m_renderer, surface);
    if (texture) {
        outW = surface->w;
        outH = surface->h;
    }

    SDL_DestroySurface(surface); 
    return texture; 
}

void TextRenderer::measureText(const std::string& text, int fontSize, int& w, int& h, int wrapWidth) {
    w = 0; h = 0;
    if (!m_initialized || text.empty()) return;
    
    TTF_Font* font = getFont(fontSize);
    if (!font) return;

    bool success = false;
    if (wrapWidth > 0) {
        success = TTF_GetStringSizeWrapped(font, text.c_str(), 0, wrapWidth, &w, &h);
    } else {
        success = TTF_GetStringSize(font, text.c_str(), 0, &w, &h);
    }

    if (!success) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "measureText failed for text '%s': %s", text.c_str(), SDL_GetError());
        w = 0; h = 0;
    }
}

SDL_Point TextRenderer::getTextSize(const std::string& text, int fontSize, int wrapWidth) {
    SDL_Point size = {0, 0};
    measureText(text, fontSize, size.x, size.y, wrapWidth);
    return size;
}

void TextRenderer::getFontMetrics(int fontSize, int &outAscent, int &outDescent) {
    outAscent = outDescent = 0;
    if (!m_initialized) return;
    TTF_Font* font = getFont(fontSize);
    if (!font) return;
    outAscent  = TTF_GetFontAscent(font);
    outDescent = -TTF_GetFontDescent(font);
}

void TextRenderer::clearCache() {
    for (auto const& [key, val] : m_textureCache) {
        if (val.texture) SDL_DestroyTexture(val.texture);
    }
    m_textureCache.clear();

    for (auto const& [size, font] : m_fontsBySize) {
        if (font) TTF_CloseFont(font);
    }
    m_fontsBySize.clear();
}

SDL_Renderer* TextRenderer::getRenderer() const {
    return m_renderer;
}
