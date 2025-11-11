// SPDX-License-Identifier: Apache-2.0
/*
 *  Copyright (C) 2025 MD S M Sarowar Hossain
 *
 * 
 * 
 * 
 * 
 */
// Defines the TextRenderer singleton class, responsible for initializing SDL_ttf,
// loading fonts, and managing a cache of rendered text textures for efficient
// Retained Mode rendering. It also exposes an Immediate Mode rendering utility.
//

#ifndef TEXTRENDERER_H
#define TEXTRENDERER_H

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h> // Necessary for TTF_Font and SDL_ttf functions
#include <string>
#include <vector>
#include <map>
#include <sstream>         // Used for creating unique cache keys

/**
 * @brief Structure to store a rendered texture and its dimensions in the cache.
 *
 * This structure is used internally by the TextRenderer to manage the lifetime
 * and metadata of pre-rendered text textures for Retained Mode UI.
 */
struct CachedTextureInfo {
    SDL_Texture* texture;   ///< The rendered text texture pointer.
    int width;              ///< Width of the texture in pixels.
    int height;             ///< Height of the texture in pixels.
};

/**
 * @brief Singleton class for managing font loading, text rendering, and caching using SDL_ttf.
 *
 * This class abstracts the low-level SDL_ttf and SDL rendering calls, managing
 * font resources (TTF_Font objects) and rendered text textures efficiently.
 * It supports both Retained Mode (cached) and Immediate Mode (uncached) rendering.
 */
class TextRenderer {
public:
    /**
     * @brief Provides access to the single instance of the TextRenderer (Singleton pattern).
     * @return Reference to the singleton TextRenderer object.
     */
    static TextRenderer& getInstance();

    /**
     * @brief Initializes the SDL_ttf subsystem and locates a fallback font.
     *
     * This must be called successfully before any rendering operations are attempted.
     *
     * @param renderer The active SDL_Renderer instance to use for all texture creation.
     * @param preferredFamilies Optional hint for font family preference (used in font selection logic).
     */
    void init(SDL_Renderer* renderer, const std::vector<std::string>& preferredFamilies = {});

    // -------------------------------------------------------------------------
    // --- Retained Mode Functionality (Cached) Starts Here ---
    // -------------------------------------------------------------------------

    /**
     * @brief Renders the given text at the specified screen coordinates using the internal texture cache.
     *
     * This is the primary rendering method for Retained Mode. If the texture is not
     * cached (by text, color, and size), it is rendered, cached, and then drawn.
     *
     * @param text The string content to draw.
     * @param x The screen X coordinate (top-left of the text bounding box).
     * @param y The screen Y coordinate (top-left of the text bounding box).
     * @param color The color of the text.
     * @param fontSize The size of the font.
     */
    void renderText(const std::string& text, int x, int y, SDL_Color color, int fontSize);

    /**
     * @brief Retrieves a texture for the given text, utilizing or creating an entry in the texture cache.
     *
     * This internal function is the core of the Retained Mode logic. The returned
     * texture is owned and managed by the TextRenderer's cache.
     *
     * @param text The string to render.
     * @param color The SDL_Color to use for the text.
     * @param fontSize The desired font size.
     * @param outW Output parameter for the resulting texture width.
     * @param outH Output parameter for the resulting texture height.
     * @return A pointer to the cached SDL_Texture, or nullptr on failure.
     */
    SDL_Texture* renderTextToTexture(const std::string& text, SDL_Color color, int fontSize, int& outW, int& outH);

    // -------------------------------------------------------------------------
    // --- Retained Mode Functionality (Cached) Ends Here ---
    // -------------------------------------------------------------------------


    /**
     * @brief Calculates the pixel dimensions required to render a given text string.
     *
     * @param text The string content.
     * @param fontSize The size of the font.
     * @param w Output parameter for the resulting pixel width.
     * @param h Output parameter for the resulting pixel height.
     */
    void measureText(const std::string& text, int fontSize, int& w, int& h);

    /**
     * @brief Convenience function to calculate text size and return it as an SDL_Point.
     *
     * @param text The string content.
     * @param fontSize The size of the font.
     * @return An SDL_Point structure containing the width (x) and height (y).
     */
    SDL_Point getTextSize(const std::string& text, int fontSize);

    /**
     * @brief Destroys all textures in the texture cache and closes all loaded fonts in the font cache.
     */
    void clearCache();

    // -------------------------------------------------------------------------
    // --- Immediate Mode Functionality (Uncached) Starts Here ---
    // -------------------------------------------------------------------------

    /**
     * @brief Immediate Mode: Renders text into a new SDL_Texture without caching it.
     *
     * This function is intended for highly dynamic text. The **caller** is responsible
     * for managing the lifecycle of the returned texture (i.e., calling SDL_DestroyTexture).
     *
     * @param text The string to render.
     * @param color The SDL_Color to use for the text.
     * @param fontSize The desired font size.
     * @param outW Output parameter for the resulting texture width.
     * @param outH Output parameter for the resulting texture height.
     * @return A pointer to the new SDL_Texture, or nullptr on failure.
     */
    SDL_Texture* renderTextImmediateToTexture(const std::string& text, SDL_Color color, int fontSize, int& outW, int& outH);

    // -------------------------------------------------------------------------
    // --- Immediate Mode Functionality (Uncached) Ends Here ---
    // -------------------------------------------------------------------------

    /**
     * @brief Getter for the internal SDL_Renderer pointer.
     * @return The SDL_Renderer pointer used by this instance.
     */
    SDL_Renderer* getRenderer() const;

    /**
     * @brief Checks the initialization status of the TextRenderer.
     * @return true if initialization was successful, false otherwise.
     */
    bool isInitialized() const;

    /**
     * @brief Retrieves the font metrics (ascent and descent) for a specific size.
     *
     * @param fontSize The size of the font.
     * @param outAscent Output: The height above the baseline (in pixels).
     * @param outDescent Output: The depth below the baseline (in pixels, positive value).
     */
    void getFontMetrics(int fontSize, int &outAscent, int &outDescent);

    /**
     * @brief Retrieves a loaded TTF_Font object for a specific size, loading it if necessary and caching it.
     *
     * @param fontSize The desired font size.
     * @return A pointer to the loaded TTF_Font object, or nullptr on failure.
     */
    TTF_Font* getFont(int fontSize);


private:
    /**
     * @brief Private constructor for singleton enforcement.
     */
    TextRenderer();

    /**
     * @brief Private destructor for resource cleanup.
     */
    ~TextRenderer();

    // Delete copy constructor and assignment operator to prevent copies of the singleton
    TextRenderer(const TextRenderer&) = delete;
    TextRenderer& operator=(const TextRenderer&) = delete;

    // --- Internal State ---

    SDL_Renderer* m_renderer;           ///< The SDL renderer context for creating textures.
    bool m_initialized;                 ///< Flag indicating successful initialization.
    std::string m_fontPath;             ///< The file path or asset name of the selected font.

    /**
     * Notes on m_fontPath:
     * On platforms like Android, this stores the *asset name* (e.g., "Roboto-Regular.ttf").
     * On desktop platforms, it stores the standard file system path.
     */

    // --- Caching for Retained Mode ---

    // Map to cache TTF_Font objects keyed by requested size.
    std::map<int, TTF_Font*> m_fontsBySize;

    // Map to cache rendered textures keyed by a unique string derived from text content and size/color.
    std::map<std::string, CachedTextureInfo> m_textureCache;

    /**
     * @brief Internal helper to generate a unique key for the texture cache.
     *
     * @param text The text content.
     * @param fontSize The font size.
     * @return The unique cache key string.
     */
    std::string createCacheKey(const std::string& text, int fontSize);

    /**
     * @brief Platform-specific logic to locate a bundled or system fallback font.
     * @return The path/asset name of the found font file.
     */
    std::string findBundledFallbackFont();
};

#endif // TEXTRENDERER_H