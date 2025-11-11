


//
//
//
// SPDX-License-Identifier: Apache-2.0
/*
 *  Copyright (C) 2025 MD S M Sarowar Hossain
 *
 * 
 * 
 * 
 * 
 */
//
//
//



#include "TextRenderer.h"
#include <SDL3/SDL_log.h>
#include <fstream>      
#include <vector>
#include <cstdlib>   


#if defined(_WIN32) || defined(_WIN64)
#define PLATFORM_WINDOWS
#elif defined(__linux__)
#define PLATFORM_LINUX
#elif defined(__APPLE__)
#define PLATFORM_MACOS
#endif



TextRenderer& TextRenderer::getInstance() {
    static TextRenderer instance;
    return instance;
}

TextRenderer::TextRenderer() : m_renderer(nullptr), m_initialized(false) {}

TextRenderer::~TextRenderer() {
    clearCache();
    
}

bool TextRenderer::isInitialized() const {
    return m_initialized;
}

/**
 * @brief Finds a suitable fallback font on the system.
 *
 * This function is now designed for desktop platforms (Windows, Linux, etc.).
 * It searches for a font in a specific order:
 * 1. A path provided by the build system (XENUI_FALLBACK_FONT_PATH).
 * 2. Common relative asset directories ("fonts/", "assets/").
 * 3. Standard system font directories for the detected OS.
 *
 * @return The full path to a found font, or an empty string if none are found.
 */
std::string TextRenderer::findBundledFallbackFont() {
 // --- STRATEGY 1: Search system directories first. ---
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "TextRenderer: Searching for system fonts...");

    const std::vector<std::string> commonFontNames = {
        "Arial.ttf", "DejaVuSans.ttf", "Verdana.ttf", "Tahoma.ttf", "Verdana.ttf", "Georgia.ttf"
    };
    
    std::vector<std::string> systemSearchPaths;

#ifdef PLATFORM_WINDOWS
    const char* windir = std::getenv("windir");
    if (windir) {
        systemSearchPaths.push_back(std::string(windir) + "\\Fonts\\");
    }
#elif defined(PLATFORM_LINUX)
    systemSearchPaths = {
        "/usr/share/fonts/truetype/dejavu/",
        "/usr/share/fonts/truetype/liberation/",
        "/usr/share/fonts/truetype/noto/",
        "/usr/share/fonts/truetype/"
    };
#elif defined(PLATFORM_MACOS)
    const char* home = std::getenv("HOME");
    systemSearchPaths = { "/System/Library/Fonts/Supplemental/", "/Library/Fonts/" };
    if (home) {
        systemSearchPaths.push_back(std::string(home) + "/Library/Fonts/");
    }
#endif

    for (const auto& path : systemSearchPaths) {
        for (const auto& fontName : commonFontNames) {
            std::string fullPath = path + fontName;
            if (std::ifstream(fullPath).good()) {
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "TextRenderer: Found system font: %s", fullPath.c_str());
                return fullPath; // Success, return immediately
            }
        }
    }
    
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "TextRenderer: No system font found.");

    // --- STRATEGY 2: If system search fails, try the bundled font path from CMake. ---
#ifdef XENUI_FALLBACK_FONT_PATH
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "TextRenderer: Attempting to load bundled font as a fallback...");
    
    // This is the crucial part. We use SDL_GetBasePath() to resolve the @rpath correctly.
    const char* basePath = SDL_GetBasePath();
    if (basePath) {
        
        
        std::string fullPath = std::string(basePath) + "../Resources/fonts/XenUI/DejaVuSans.ttf";
        SDL_free((void*)basePath);

        if (std::ifstream(fullPath).good()) {
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "TextRenderer: Found bundled font at resolved path: %s", fullPath.c_str());
            return fullPath;
        } else {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "TextRenderer: Bundled font not found at expected path: %s", fullPath.c_str());
        }
    } else {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Could not get application base path to resolve bundled font path.");
    }
#endif

    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "TextRenderer: CRITICAL - No fallback font could be located anywhere.");
    return ""; // Return empty string on failure
}

void TextRenderer::init(SDL_Renderer* renderer, const std::vector<std::string>& preferredFamilies) {
    if (m_initialized) return;
    if (!renderer) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "TextRenderer init failed: Renderer is null.");
        return;
    }
    m_renderer = renderer;

 
    if (!TTF_WasInit() && TTF_Init() == -1) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_ttf could not initialize! SDL_ttf Error: %s", SDL_GetError());
        return;
    }

    m_fontPath = findBundledFallbackFont();

    if (m_fontPath.empty()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "ERROR: TextRenderer init failed. Could not find any suitable font!");
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
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "getFont: Failed to open font '%s' as IOStream: %s", m_fontPath.c_str(), SDL_GetError());
        return nullptr;
    }

  
    TTF_Font* font = TTF_OpenFontIO(rwops, true, fontSize);

    if (!font) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "getFont: Failed to open font from IOStream for '%s' size %d. Error: %s", m_fontPath.c_str(), fontSize, SDL_GetError());
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
    return texture; // Caller is responsible for SDL_DestroyTexture
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