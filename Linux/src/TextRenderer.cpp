//
// SPDX-License-Identifier: Apache-2.0
/*
 *  Copyright (C) 2025 MD S M Sarowar Hossain
 */
//
// Implements the singleton pattern for text rendering using SDL_ttf.
// Manages font loading, caching, and text-to-texture rendering, with
// specific considerations for cross-platform (Android/Desktop) font loading.
//

#include "TextRenderer.h"
#include <SDL3/SDL_log.h>     // For SDL_LogError, SDL_LogInfo
#include <fstream>            // For checking file existence on non-Android platforms
#include <SDL_ttf.h>          // SDL TrueType Font library

/**
 * @brief Retrieves the single instance of the TextRenderer class (Singleton).
 *
 * This function ensures that only one TextRenderer object exists throughout the application
 * lifecycle, providing a central point for all font and text services.
 *
 * @return Reference to the singleton TextRenderer instance.
 */
TextRenderer& TextRenderer::getInstance() {
    static TextRenderer instance;
    return instance;
}

/**
 * @brief Private constructor for the TextRenderer singleton.
 *
 * Initializes internal state variables.
 */
TextRenderer::TextRenderer() : m_renderer(nullptr), m_initialized(false) {}

/**
 * @brief Destructor for the TextRenderer.
 *
 * Cleans up all cached fonts and textures and quits the SDL_ttf subsystem.
 */
TextRenderer::~TextRenderer() {
    // Release all dynamically allocated resources
    clearCache();
    // Safely quit the SDL_ttf library only if it was initialized
    if (TTF_WasInit()) {
        TTF_Quit();
    }
}

/**
 * @brief Checks if the TextRenderer has been successfully initialized.
 *
 * Initialization requires a valid SDL_Renderer and successful startup of SDL_ttf.
 *
 * @return true if initialized, false otherwise.
 */
bool TextRenderer::isInitialized() const {
    return m_initialized;
}

/**
 * @brief Searches for a suitable bundled fallback font file across platforms.
 *
 * On Android, this attempts to load the font from the application's 'assets' directory
 * using SDL_IOFromFile, which correctly abstracts asset access.
 * On non-Android platforms, it checks for a path defined by XENUI_FALLBACK_FONT_PATH
 * or a common system font path as a last resort.
 *
 * @return The path or asset name of the found font file; an empty string on failure.
 */
std::string TextRenderer::findBundledFallbackFont() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "TextRenderer: Searching for bundled fallback font...");

    // Common fallback font names. One of these should be present in the bundle's 'fonts/' directory.
    const char* commonFontNames[] = {"DejaVuSans.ttf", "Roboto-Regular.ttf", "NotoSans-Regular.ttf", "Arial.ttf"};
    std::string foundFontName = "";

    #ifdef __ANDROID__
    // --- Android Asset Loading ---
    for (const char* assetName : commonFontNames) {
        // Construct the full path relative to the assets root
        std::string assetPath = "fonts/";
        assetPath += assetName;
        // SDL_IOFromFile is the correct way to open Android assets
        SDL_IOStream* rwops = SDL_IOFromFile(assetPath.c_str(), "rb");
        if (rwops) {
            // Close the stream as we only needed to verify its existence/readability
            SDL_CloseIO(rwops);
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "TextRenderer: Found bundled font in assets: %s", assetPath.c_str());
            foundFontName = assetPath; // Store the full asset path for later TTF loading
            break;
        } else {
            SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "TextRenderer: Bundled font '%s' not found in assets via SDL_IOFromFile: %s", assetPath.c_str(), SDL_GetError());
        }
    }
    if (foundFontName.empty()) {
         SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "TextRenderer: No common bundled font found in assets.");
    }
#else
    // --- Non-Android (Desktop/Other) File System Loading ---
    #ifdef XENUI_FALLBACK_FONT_PATH
        // 1. Check a path defined at compile time (e.g., via CMake)
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
        // 2. Try a common system path as a last resort (e.g., Linux/macOS)
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


/**
 * @brief Initializes the TextRenderer subsystem.
 *
 * This involves setting the SDL_Renderer pointer, initializing SDL_ttf, and locating
 * a suitable font file path for subsequent loading operations.
 *
 * @param renderer The active SDL_Renderer instance to use for texture creation.
 * @param preferredFamilies A list of desired font family names (currently unused for asset loading).
 */
void TextRenderer::init(SDL_Renderer* renderer, const std::vector<std::string>& preferredFamilies) {
    if (m_initialized) return;
    if (!renderer) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "TextRenderer init failed: Renderer is null.");
        return;
    }
    m_renderer = renderer;

    // Initialize SDL_ttf library
    if (TTF_Init() == -1) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_ttf could not initialize! SDL_ttf Error: %s", SDL_GetError());
        return; // Fail initialization
    }

    // Locate the font path/asset name
    // Note: preferredFamilies are not explicitly used here to pick from assets,
    // but findBundledFallbackFont could be extended to use them if you bundle multiple fonts
    // and want to select one based on preference.
    m_fontPath = findBundledFallbackFont();

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
        "Using font path: %s", m_fontPath.empty() ? "<none>" : m_fontPath.c_str());


    if (m_fontPath.empty()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "ERROR: TextRenderer init failed. Could not find any suitable bundled font!");
        TTF_Quit(); // Clean up TTF if font finding failed
        m_initialized=false;
        return;
    }

    m_initialized = true;
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "TextRenderer initialized successfully. Using font: %s", m_fontPath.c_str());
}


/**
 * @brief Retrieves a TTF_Font object for a specific size, loading it if necessary.
 *
 * This function implements caching of loaded fonts by size to avoid redundant file operations.
 * Font loading is performed via SDL_IOFromFile/TTF_OpenFontIO to handle asset management.
 *
 * @param fontSize The desired font size in points.
 * @return A pointer to the loaded TTF_Font object, or nullptr on failure.
 */
TTF_Font* TextRenderer::getFont(int fontSize) {
    // Basic validation
    if (!m_initialized || m_fontPath.empty() || fontSize <= 0) return nullptr;

    // Check the font cache first
    auto it = m_fontsBySize.find(fontSize);
    if (it != m_fontsBySize.end()) {
        return it->second;
    }

    // Font not found in cache, proceed to load it
    SDL_IOStream* rwops = nullptr;
    // Use SDL_IOFromFile which handles file paths on desktop and asset paths on Android
    rwops = SDL_IOFromFile(m_fontPath.c_str(), "rb");

    if (!rwops) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "getFont: Failed to open font '%s' as RWops: %s", m_fontPath.c_str(), SDL_GetError());
        return nullptr;
    }

    // TTF_OpenFontIO loads the font from the SDL_IOStream.
    // The second argument (true) tells SDL_ttf to close the rwops stream after loading,
    // even if loading fails. This prevents memory leaks.
    TTF_Font* font = TTF_OpenFontIO(rwops, true, fontSize);

    if (!font) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "getFont: Failed to open font from RWops for '%s' size %d. Error: %s", m_fontPath.c_str(), fontSize, SDL_GetError());
        return nullptr;
    }

    // Cache the newly loaded font object
    m_fontsBySize[fontSize] = font;
    return font;
}

/**
 * @brief Creates a unique key for caching textures based on text content and size.
 *
 * @param text The string content.
 * @param fontSize The size of the font used.
 * @return A concatenated string key.
 */
std::string TextRenderer::createCacheKey(const std::string& text, int fontSize) {
    std::stringstream ss;
    ss << text << '|' << fontSize;
    return ss.str();
}

/**
 * @brief Renders the given text into an SDL_Texture, utilizing a texture cache.
 *
 * This is the core rendering function. It checks the cache, and if not found,
 * renders the text using TTF_RenderText_Blended, creates an SDL_Texture, caches it,
 * and destroys the intermediate SDL_Surface.
 *
 * @param text The string to render.
 * @param color The SDL_Color to use for the text.
 * @param fontSize The desired font size.
 * @param outW Output parameter for the resulting texture width.
 * @param outH Output parameter for the resulting texture height.
 * @return A pointer to the cached SDL_Texture, or nullptr on failure.
 */
SDL_Texture* TextRenderer::renderTextToTexture(const std::string& text, SDL_Color color, int fontSize, int& outW, int& outH) {
    if (!m_initialized || text.empty()) {
        outW = 0; outH = 0;
        return nullptr;
    }

    // 1. Check Cache
    std::string key = createCacheKey(text, fontSize);
    auto cacheIt = m_textureCache.find(key);
    if (cacheIt != m_textureCache.end()) {
        outW = cacheIt->second.width;
        outH = cacheIt->second.height;
        return cacheIt->second.texture;
    }

    // 2. Get Font
    TTF_Font* font = getFont(fontSize);
    if (!font) {
        outW = 0; outH = 0;
        return nullptr;
    }

    // 3. Render to Surface (Blended mode for high quality)
    // Note: The color argument is ignored in TTF_RenderText_Blended as the color is passed in the SDL_Color struct.
    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), 0, color);
    if (!surface) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "renderTextToTexture: Failed to create surface: %s", SDL_GetError());
        outW = 0; outH = 0;
        return nullptr;
    }

    // 4. Create Texture from Surface
    SDL_Texture* texture = SDL_CreateTextureFromSurface(m_renderer, surface);
    if (!texture) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "renderTextToTexture: Failed to create texture: %s", SDL_GetError());
        outW = 0; outH = 0;
    } else {
        // 5. Cache Texture and Dimensions
        outW = surface->w;
        outH = surface->h;
        m_textureCache[key] = {texture, outW, outH};
    }

    // 6. Clean up Surface (it's now copied into the texture)
    SDL_DestroySurface(surface); // Use SDL_DestroySurface in SDL3
    return texture;
}

/**
 * @brief Draws the text at the specified screen coordinates (using the internal texture cache).
 *
 * This is the high-level drawing function.
 *
 * @param text The string content to draw.
 * @param x The screen X coordinate (top-left).
 * @param y The screen Y coordinate (top-left).
 * @param color The color of the text.
 * @param fontSize The size of the font.
 */
void TextRenderer::renderText(const std::string& text, int x, int y, SDL_Color color, int fontSize) {
    if (!m_initialized) return;
    int texW = 0, texH = 0;
    // Attempt to retrieve or render the text texture
    SDL_Texture* texture = renderTextToTexture(text, color, fontSize, texW, texH);
    if (!texture) return;

    // Define the destination rectangle on the screen
    SDL_FRect dstRect = { (float)x, (float)y, (float)texW, (float)texH };
    // Render the texture to the screen
    SDL_RenderTexture(m_renderer, texture, nullptr, &dstRect); // Use SDL_RenderTexture in SDL3
}

/**
 * @brief Renders text into a new SDL_Texture without caching it.
 *
 * This function is useful for dynamic, frequently changing text where caching would be inefficient
 * (e.g., frame counters). The caller is responsible for destroying the returned texture.
 *
 * @param text The string to render.
 * @param color The SDL_Color to use for the text.
 * @param fontSize The desired font size.
 * @param outW Output parameter for the resulting texture width.
 * @param outH Output parameter for the resulting texture height.
 * @return A pointer to the new SDL_Texture, or nullptr on failure.
 */
SDL_Texture* TextRenderer::renderTextImmediateToTexture(const std::string& text, SDL_Color color, int fontSize, int& outW, int& outH) {
    outW = 0; outH = 0;
    // Check initialization and basic input validity
    if (!m_initialized || text.empty() || fontSize <= 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error: TextRenderer not ready or invalid input for immediate render.");
        return nullptr;
    }

    TTF_Font* font = getFont(fontSize);
    if (!font) return nullptr;

    // Render to surface (immediate, uncached)
    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), 0, color);
    if (!surface) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error creating immediate surface: %s", SDL_GetError());
        return nullptr;
    }

    // Create texture
    SDL_Texture* texture = SDL_CreateTextureFromSurface(m_renderer, surface);
    if (texture) {
        outW = surface->w;
        outH = surface->h;
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error creating immediate texture: %s", SDL_GetError());
    }

    SDL_DestroySurface(surface); // Clean up surface
    return texture; // Caller must call SDL_DestroyTexture later
}

/**
 * @brief Calculates the pixel dimensions required to render a given text string.
 *
 * @param text The string content.
 * @param fontSize The size of the font.
 * @param w Output parameter for the resulting pixel width.
 * @param h Output parameter for the resulting pixel height.
 */
void TextRenderer::measureText(const std::string& text, int fontSize, int& w, int& h) {
    w = 0; h = 0;
    if (!m_initialized || text.empty()) return;

    TTF_Font* font = getFont(fontSize);
    if (!font) return;

    // Use TTF_GetStringSize to calculate dimensions without rendering
    // Note: The second argument (0) is flags, usually 0 or TTF_MEASURE_PERFECT.
    if (!TTF_GetStringSize(font, text.c_str(), 0, &w, &h)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "measureText: TTF_GetStringSize failed for font '%s', text '%s': %s", m_fontPath.c_str(), text.c_str(), SDL_GetError());
        w = 0; h = 0;
    }
}

/**
 * @brief Calculates the pixel dimensions required to render a given text string.
 *
 * This is a convenience function wrapper around measureText returning an SDL_Point.
 *
 * @param text The string content.
 * @param fontSize The size of the font.
 * @return An SDL_Point containing the width (x) and height (y).
 */
SDL_Point TextRenderer::getTextSize(const std::string& text, int fontSize) {
    SDL_Point size = {0, 0};
    measureText(text, fontSize, size.x, size.y);
    return size;
}

/**
 * @brief Retrieves the font's ascent and descent metrics for the specified size.
 *
 * These metrics are useful for precise vertical text alignment.
 *
 * @param fontSize The size of the font.
 * @param outAscent Output parameter for the font ascent (max height above baseline).
 * @param outDescent Output parameter for the font descent (max depth below baseline, positive value).
 */
void TextRenderer::getFontMetrics(int fontSize, int &outAscent, int &outDescent) {
    outAscent = outDescent = 0;
    if (!m_initialized) return;
    TTF_Font* font = getFont(fontSize);
    if (!font) return;
    outAscent  = TTF_GetFontAscent(font);
    // TTF_GetFontDescent returns a *negative* value for the depth below the baseline.
    // Negate it to get a standard positive descent metric.
    outDescent = -TTF_GetFontDescent(font);
}

/**
 * @brief Destroys all cached textures and closes all loaded fonts.
 *
 * Should be called upon application shutdown or when the graphics context is lost/reset.
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
 * @brief Provides access to the underlying SDL_Renderer.
 *
 * @return The SDL_Renderer pointer used by this TextRenderer instance.
 */
SDL_Renderer* TextRenderer::getRenderer() const {
    return m_renderer;
}