// SPDX-License-Identifier: Apache-2.0
/*
 *  Copyright (C) 2025 MD S M Sarowar Hossain
 *
 * 
 * 
 * 
 * 
 */
// Implements the TextRenderer singleton class, providing font loading, text
// measurement, and efficient texture caching for text rendering using SDL_ttf.
// Modified for professional Windows and Desktop support with robust font fallback logic.
//

#include "TextRenderer.h"
#include <SDL3/SDL_log.h>
#include <fstream>       // For checking file existence on desktop platforms
#include <vector>
#include <cstdlib>       // For getenv on Windows
#include <algorithm>     // For std::transform (optional, but good practice)

// Platform-specific defines for clear conditional compilation logic
#if defined(_WIN32) || defined(_WIN64)
#define PLATFORM_WINDOWS
#elif defined(__linux__)
#define PLATFORM_LINUX
#elif defined(__APPLE__)
#define PLATFORM_MACOS
#endif


/**
 * Provides access to the single instance of the TextRenderer (Singleton pattern).
 *Reference to the singleton TextRenderer object.
 */
TextRenderer& TextRenderer::getInstance() {
    // The instance is lazily created upon first call.
    static TextRenderer instance;
    return instance;
}

/**
 *Private constructor for the TextRenderer singleton.
 * Initializes member variables to a safe default state.
 */
TextRenderer::TextRenderer() : m_renderer(nullptr), m_initialized(false) {}

/**
 * Private destructor for TextRenderer.
 * Cleans up all resources managed by the singleton before destruction.
 */
TextRenderer::~TextRenderer() {
    clearCache();
    // TTF_Quit is typically handled at the application level, but ensure no leaks occur.
}

/**
 *  Checks the initialization status of the TextRenderer.
 *  true if SDL_ttf is ready and a font path is configured, false otherwise.
 */
bool TextRenderer::isInitialized() const {
    return m_initialized;
}

/**
 *  Finds a suitable fallback font on the system or bundled assets.
 *
 * This function implements a hierarchy of search methods designed for desktop
 * platforms (Windows, Linux, macOS) to ensure a font is found:
 * 1. An absolute path defined during compilation (`XENUI_FALLBACK_FONT_PATH`).
 * 2. Common relative asset directories (e.g., "fonts/", "assets/").
 * 3. Standard system font directories based on the detected OS.
 *
 *  The full file system path to a found font, or an empty string on failure.
 */
std::string TextRenderer::findBundledFallbackFont() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "TextRenderer: Searching for fallback font on desktop platform...");

    // --- Font search configuration ---
    const std::vector<std::string> fontSearchPaths = {
        // 2. Check for relative asset folders first (good for portable builds)
        "fonts/",
        "assets/"
    };

    // A list of common font names to search for in directories
    const std::vector<std::string> commonFontNames = {
        // Preferred fonts are listed first for best appearance
        "SegoeUI.ttf",
        "Arial.ttf",
        "DejaVuSans.ttf",
        "Roboto-Regular.ttf",
        "NotoSans-Regular.ttf",
        "Tahoma.ttf",
        "Verdana.ttf"
    };

    // --- Search Logic ---

    // 1. Check for a path explicitly defined by the build system (most reliable if set)
    #ifdef XENUI_FALLBACK_FONT_PATH
        std::string embeddedPath = XENUI_FALLBACK_FONT_PATH;
        // Use std::ifstream to check if the file exists and is accessible
        std::ifstream f(embeddedPath.c_str());
        if (f.good()) {
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "TextRenderer: Found font at compile-time path: %s", embeddedPath.c_str());
            return embeddedPath;
        } else {
             SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "TextRenderer: Compile-time path was defined but not found: %s", embeddedPath.c_str());
        }
    #endif

    // 2. Search relative paths defined in fontSearchPaths
    for (const auto& path : fontSearchPaths) {
        for (const auto& fontName : commonFontNames) {
            std::string fullPath = path + fontName;
            std::ifstream f(fullPath.c_str());
            if (f.good()) {
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "TextRenderer: Found font in relative directory: %s", fullPath.c_str());
                return fullPath;
            }
        }
    }

    // 3. Search system-specific font directories as a last resort
    std::string systemFontPath;
#ifdef PLATFORM_WINDOWS
    // Attempt to retrieve the Windows directory environment variable
    const char* windir = std::getenv("windir");
    if (windir) {
        systemFontPath = std::string(windir) + "\\Fonts\\";
    }
    else {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Could not get 'windir' environment variable.");
    }
#elif defined(PLATFORM_LINUX)
    // Define multiple common Linux font directories
    const std::vector<std::string> linuxPaths = {
        "/usr/share/fonts/truetype/dejavu/",
        "/usr/share/fonts/truetype/liberation/",
        "/usr/share/fonts/truetype/noto/",
        "/usr/share/fonts/truetype/msttcorefonts/" // Common path for Microsoft fonts on Linux
    };
    // Iterate through common Linux paths and try to find any of the common font names
    for (const auto& path : linuxPaths) {
         for (const auto& fontName : commonFontNames) {
             std::string fullPath = path + fontName;
             // Check if the specific font file exists at this path
             std::ifstream f(fullPath.c_str());
             if (f.good()) {
                 // Found a specific font file, return the full path immediately
                 SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "TextRenderer: Found Linux system font: %s", fullPath.c_str());
                 return fullPath;
             }
         }
    }
    // Note: The original implementation used a goto, which is generally avoided.
    // The refactored logic above removes the need for `goto found_linux_path:;`
#elif defined(PLATFORM_MACOS)
    // Define common macOS font directories
    const std::vector<std::string> macPaths = {
        "/Library/Fonts/",
        "/System/Library/Fonts/",
        // Check the user's home directory fonts folder
        std::string(std::getenv("HOME")) + "/Library/Fonts/"
    };
    // Iterate through all macOS font paths and common names
    for (const auto& path : macPaths) {
        for (const auto& fontName : commonFontNames) {
            std::string fullPath = path + fontName;
            if (std::ifstream(fullPath).good()) {
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "TextRenderer: Found macOS system font: %s", fullPath.c_str());
                return fullPath;
            }
        }
    }
#endif

    // For Windows, check common font names in the determined system font path
    if (!systemFontPath.empty()) {
        for (const auto& fontName : commonFontNames) {
            std::string fullPath = systemFontPath + fontName;
            std::ifstream f(fullPath.c_str());
            if (f.good()) {
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "TextRenderer: Found system font: %s", fullPath.c_str());
                return fullPath;
            }
        }
    }

    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "TextRenderer: CRITICAL - No fallback font could be located anywhere.");
    return ""; // Return empty string on failure
}

/**
 * Initializes the TextRenderer with the SDL context and locates a font.
 *
 * renderer The active SDL_Renderer instance, required for texture creation.
 *  preferredFamilies Optional hint for font family preference (currently ignored by the fallback logic).
 */
void TextRenderer::init(SDL_Renderer* renderer, const std::vector<std::string>& preferredFamilies) {
    if (m_initialized) return; // Prevent double initialization

    if (!renderer) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "TextRenderer init failed: Renderer is null.");
        return;
    }
    m_renderer = renderer;

    // Check if SDL_ttf is initialized and attempt to initialize if not
    if (!TTF_WasInit() && TTF_Init() == -1) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_ttf could not initialize! SDL_ttf Error: %s", SDL_GetError());
        return;
    }

    // Find the font using the platform-specific fallback logic
    m_fontPath = findBundledFallbackFont();

    if (m_fontPath.empty()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "ERROR: TextRenderer init failed. Could not find any suitable font!");
        m_initialized = false;
        return;
    }

    m_initialized = true;
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "TextRenderer initialized successfully. Using font: %s", m_fontPath.c_str());
}

/**
 * Retrieves a loaded TTF_Font object for a specific size, loading it if necessary and caching it.
 *
 * The loaded font is managed by the internal cache (Retained Mode resource management).
 *
 * fontSize The desired font size in points.
 * A pointer to the loaded TTF_Font object, or nullptr on failure.
 */
TTF_Font* TextRenderer::getFont(int fontSize) {
    if (!m_initialized || m_fontPath.empty() || fontSize <= 0) return nullptr;

    // Check the font cache first (Retained Mode)
    auto it = m_fontsBySize.find(fontSize);
    if (it != m_fontsBySize.end()) {
        return it->second;
    }

    // If not cached, load the font using the SDL_IOStream abstraction
    // SDL_IOFromFile is used to correctly handle file system access across platforms
    // The "rb" flag ensures the file is read in binary mode, which is important.
    SDL_IOStream* rwops = SDL_IOFromFile(m_fontPath.c_str(), "rb");

    if (!rwops) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "getFont: Failed to open font '%s' as IOStream: %s", m_fontPath.c_str(), SDL_GetError());
        return nullptr;
    }

    // TTF_OpenFontIO takes ownership of the rwops stream and will close it automatically.
    // The second parameter 'true' indicates this ownership transfer.
    TTF_Font* font = TTF_OpenFontIO(rwops, true, fontSize);

    if (!font) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "getFont: Failed to open font from IOStream for '%s' size %d. Error: %s", m_fontPath.c_str(), fontSize, SDL_GetError());
        // Since TTF_OpenFontIO failed, it should have destroyed the stream; no need to close rwops explicitly.
        return nullptr;
    }

    // Store the newly loaded font in the cache (Retained Mode)
    m_fontsBySize[fontSize] = font;
    return font;
}

// -------------------------------------------------------------------------
// --- Retained Mode Functionality Starts Here ---
// -------------------------------------------------------------------------

/**
 * Internal helper to generate a unique key for the texture cache.
 *
 * The key is a concatenation of the text string and the font size, separated by a pipe '|'.
 *
 * The text content.
 * The font size.
 * The unique cache key string.
 */
std::string TextRenderer::createCacheKey(const std::string& text, int fontSize) {
    std::stringstream ss;
    ss << text << '|' << fontSize;
    return ss.str();
}

/**
 * Renders the given text into an SDL_Texture, utilizing or updating the texture cache.
 *
 * This function handles the core Retained Mode logic: check cache, render if miss, and update cache.
 * The returned texture is owned by the cache and must NOT be destroyed by the caller.
 *
 *  The string to render.
 *  The SDL_Color to use for the text.
 *  The desired font size.
 * Output parameter for the resulting texture width.
 *  Output parameter for the resulting texture height.
 *  A pointer to the cached SDL_Texture, or nullptr on failure.
 */
SDL_Texture* TextRenderer::renderTextToTexture(const std::string& text, SDL_Color color, int fontSize, int& outW, int& outH) {
    if (!m_initialized || text.empty()) {
        outW = 0; outH = 0;
        return nullptr;
    }

    // Check if the texture is already in the cache
    std::string key = createCacheKey(text, fontSize);
    auto cacheIt = m_textureCache.find(key);
    if (cacheIt != m_textureCache.end()) {
        // Cache hit: return the cached texture and dimensions
        outW = cacheIt->second.width;
        outH = cacheIt->second.height;
        return cacheIt->second.texture;
    }

    // Cache miss: proceed to render
    TTF_Font* font = getFont(fontSize);
    if (!font) {
        outW = 0; outH = 0;
        return nullptr;
    }

    // Render the text into an SDL_Surface using blended rendering (for smooth anti-aliasing)
    // The final parameter '0' indicates that the background color is ignored for blended rendering.
    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), 0, color);
    if (!surface) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "renderTextToTexture: Failed to create surface: %s", SDL_GetError());
        outW = 0; outH = 0;
        return nullptr;
    }

    // Convert the SDL_Surface into an optimized SDL_Texture
    SDL_Texture* texture = SDL_CreateTextureFromSurface(m_renderer, surface);
    if (!texture) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "renderTextToTexture: Failed to create texture: %s", SDL_GetError());
        outW = 0; outH = 0;
    } else {
        // Store dimensions and cache the new texture
        outW = surface->w;
        outH = surface->h;
        m_textureCache[key] = {texture, outW, outH};
    }

    // The surface is temporary and must be destroyed
    SDL_DestroySurface(surface);
    return texture;
}

/**
 *Renders the given text to the screen using the internal texture cache.
 *
 * This is the primary Retained Mode rendering function. It retrieves the texture,
 * calculates the destination rectangle, and issues the render call.
 *
 * The string content to draw.
 * The screen X coordinate (top-left).
 * The screen Y coordinate (top-left).
 * The color of the text.
 * The size of the font.
 */
void TextRenderer::renderText(const std::string& text, int x, int y, SDL_Color color, int fontSize) {
    if (!m_initialized) return;

    int texW = 0, texH = 0;
    // Get the cached or newly rendered texture (Retained Mode)
    SDL_Texture* texture = renderTextToTexture(text, color, fontSize, texW, texH);
    if (!texture) return;

    // Define the destination rectangle on the screen
    SDL_FRect dstRect = { (float)x, (float)y, (float)texW, (float)texH };
    // Issue the render command
    SDL_RenderTexture(m_renderer, texture, nullptr, &dstRect);
}

// -------------------------------------------------------------------------
// --- Retained Mode Functionality Ends Here ---
// -------------------------------------------------------------------------


// -------------------------------------------------------------------------
// --- Immediate Mode Functionality Starts Here ---
// -------------------------------------------------------------------------

/**
 * Immediate Mode: Renders text into a new SDL_Texture without caching it.
 *
 * The caller is responsible for calling SDL_DestroyTexture on the returned pointer.
 * This function should be used for highly dynamic text that changes frequently.
 *
 *  The string to render.
 * The SDL_Color to use for the text.
 * The desired font size.
 * Output parameter for the resulting texture width.
 *  Output parameter for the resulting texture height.
 * A pointer to the new, unmanaged SDL_Texture, or nullptr on failure.
 */
SDL_Texture* TextRenderer::renderTextImmediateToTexture(const std::string& text, SDL_Color color, int fontSize, int& outW, int& outH) {
    outW = 0; outH = 0;
    if (!m_initialized || text.empty() || fontSize <= 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error: TextRenderer not ready or invalid input for immediate render.");
        return nullptr;
    }

    // Load font (retrieved from retained font cache, but texture creation is immediate mode)
    TTF_Font* font = getFont(fontSize);
    if (!font) return nullptr;

    // Render text to a temporary surface
    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), 0, color);
    if (!surface) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error creating immediate surface: %s", SDL_GetError());
        return nullptr;
    }

    // Create texture from surface
    SDL_Texture* texture = SDL_CreateTextureFromSurface(m_renderer, surface);
    if (texture) {
        outW = surface->w;
        outH = surface->h;
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error creating immediate texture: %s", SDL_GetError());
    }

    // Destroy the temporary surface
    SDL_DestroySurface(surface);
    return texture; // Caller is responsible for SDL_DestroyTexture
}

// -------------------------------------------------------------------------
// --- Immediate Mode Functionality Ends Here ---
// -------------------------------------------------------------------------


/**
 * Calculates the pixel dimensions required to render a given text string.
 *
 * This function uses TTF_GetStringSize, which performs the measurement without
 * actually creating a surface or texture.
 *
 * The string content.
 *  The size of the font.
 *  Output parameter for the resulting pixel width.
 * Output parameter for the resulting pixel height.
 */
void TextRenderer::measureText(const std::string& text, int fontSize, int& w, int& h) {
    w = 0; h = 0;
    if (!m_initialized || text.empty()) return;

    TTF_Font* font = getFont(fontSize);
    if (!font) return;

    // The TTF_GetStringSize function calculates the size
    if (!TTF_GetStringSize(font, text.c_str(), 0, &w, &h)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "measureText: TTF_GetStringSize failed for font '%s', text '%s': %s", m_fontPath.c_str(), text.c_str(), SDL_GetError());
        w = 0; h = 0;
    }
}

/**
 * Convenience function to calculate text size and return it as an SDL_Point.
 *
 *  The string content.
 *  The size of the font.
 *  An SDL_Point containing the width (x) and height (y).
 */
SDL_Point TextRenderer::getTextSize(const std::string& text, int fontSize) {
    SDL_Point size = {0, 0};
    measureText(text, fontSize, size.x, size.y);
    return size;
}

/**
 * Destroys all cached textures and closes all loaded TTF_Font objects.
 *
 * This releases all Retained Mode resources managed by the TextRenderer.
 */
void TextRenderer::clearCache() {
    // Destroy all cached textures
    for (auto const& [key, val] : m_textureCache) {
        if (val.texture) SDL_DestroyTexture(val.texture);
    }
    m_textureCache.clear();

    // Close all loaded fonts
    for (auto const& [size, font] : m_fontsBySize) {
        if (font) TTF_CloseFont(font);
    }
    m_fontsBySize.clear();

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Cleared TextRenderer cache (textures and fonts).");
}

/**
 * Getter for the internal SDL_Renderer pointer.
 * The SDL_Renderer pointer.
 */
SDL_Renderer* TextRenderer::getRenderer() const {
    return m_renderer;
}