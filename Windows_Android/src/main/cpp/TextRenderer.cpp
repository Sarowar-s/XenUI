// SPDX-License-Identifier: Apache-2.0
/*
 *  Copyright (C) 2025 MD S M Sarowar Hossain
 *
 * 
 */


#include "TextRenderer.h"
#include <SDL3/SDL_log.h>     
#include <fstream> 


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
        } else {
            SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "TextRenderer: Bundled font '%s' not found in assets via SDL_IOFromFile: %s", assetPath.c_str(), SDL_GetError());
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
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "TextRenderer: Found fallback font at embedded path (non-Android): %s", embeddedPath.c_str());
            foundFontName = embeddedPath;
        } else {
             SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "TextRenderer: Embedded fallback font path not found or not readable (non-Android): %s", embeddedPath.c_str());
        }
    #endif
    if (foundFontName.empty()) {
       
        const char* desktopFallback = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"; // Example Linux path
        std::ifstream f(desktopFallback);
        if (f.good()) {
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "TextRenderer: Found fallback font at system path (non-Android): %s", desktopFallback);
            foundFontName = desktopFallback;
        } else {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "TextRenderer: Bundled fallback font could not be located (non-Android), and system fallback failed.");
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

    // Initialize SDL_ttf
    if (TTF_Init() == -1) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_ttf could not initialize! SDL_ttf Error: %s", SDL_GetError());
        return; 
    }

    m_fontPath = findBundledFallbackFont(); 

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
        "Using font path: %s", m_fontPath.empty() ? "<none>" : m_fontPath.c_str());


    if (m_fontPath.empty()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "ERROR: TextRenderer init failed. Could not find any suitable bundled font!");
        TTF_Quit(); 
        m_initialized=false;
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

    SDL_IOStream* rwops = nullptr;
#ifdef __ANDROID__

    rwops = SDL_IOFromFile(m_fontPath.c_str(), "rb"); 
#else

    rwops = SDL_IOFromFile(m_fontPath.c_str(), "rb");
#endif

    if (!rwops) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "getFont: Failed to open font '%s' as RWops: %s", m_fontPath.c_str(), SDL_GetError());
        return nullptr;
    }
    

    TTF_Font* font = TTF_OpenFontIO(rwops, true, fontSize); 

    if (!font) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "getFont: Failed to open font from RWops for '%s' size %d. Error: %s", m_fontPath.c_str(), fontSize, SDL_GetError());

        return nullptr;
    }

    m_fontsBySize[fontSize] = font;
    return font;
}

std::string TextRenderer::createCacheKey(const std::string& text, int fontSize) {
    std::stringstream ss;
    ss << text << '|' << fontSize;
    return ss.str();
}

SDL_Texture* TextRenderer::renderTextToTexture(const std::string& text, SDL_Color color, int fontSize, int& outW, int& outH) {
    if (!m_initialized || text.empty()) {
        outW = 0; outH = 0;
        return nullptr;
    }

    std::string key = createCacheKey(text, fontSize);
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

    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), 0, color);
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

void TextRenderer::renderText(const std::string& text, int x, int y, SDL_Color color, int fontSize) {
    if (!m_initialized) return;
    int texW = 0, texH = 0;
    SDL_Texture* texture = renderTextToTexture(text, color, fontSize, texW, texH);
    if (!texture) return;
    
    SDL_FRect dstRect = { (float)x, (float)y, (float)texW, (float)texH }; 
    SDL_RenderTexture(m_renderer, texture, nullptr, &dstRect); 
}

SDL_Texture* TextRenderer::renderTextImmediateToTexture(const std::string& text, SDL_Color color, int fontSize, int& outW, int& outH) {
    outW = 0; outH = 0;
    if (!m_initialized || text.empty() || fontSize <= 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error: TextRenderer not ready or invalid input for immediate render.");
        return nullptr;
    }

    TTF_Font* font = getFont(fontSize);
    if (!font) return nullptr;

    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), 0, color);
    if (!surface) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error creating immediate surface: %s", SDL_GetError());
        return nullptr;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(m_renderer, surface);
    if (texture) {
        outW = surface->w;
        outH = surface->h;
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error creating immediate texture: %s", SDL_GetError());
    }

    SDL_DestroySurface(surface); 
    return texture; 
}

void TextRenderer::measureText(const std::string& text, int fontSize, int& w, int& h) {
    w = 0; h = 0;
    if (!m_initialized || text.empty()) return;
    
    TTF_Font* font = getFont(fontSize);
    if (!font) return;

    if (!TTF_GetStringSize(font, text.c_str(), 0, &w, &h)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "measureText: TTF_GetStringSize failed for font '%s', text '%s': %s", m_fontPath.c_str(), text.c_str(), SDL_GetError());
        w = 0; h = 0;
    }
}

SDL_Point TextRenderer::getTextSize(const std::string& text, int fontSize) {
    SDL_Point size = {0, 0};
    measureText(text, fontSize, size.x, size.y);
    return size;
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
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Cleared TextRenderer cache (textures and fonts).");
}

SDL_Renderer* TextRenderer::getRenderer() const {
    return m_renderer;
}