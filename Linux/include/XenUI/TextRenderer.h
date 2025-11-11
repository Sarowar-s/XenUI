// SPDX-License-Identifier: Apache-2.0
/*
 *  Copyright (C) 2025 MD S M Sarowar Hossain
 */
//
// Defines the TextRenderer singleton class, responsible for initializing SDL_ttf,
// loading fonts, and managing a cache of rendered text textures.
//

#ifndef TEXTRENDERER_H
#define TEXTRENDERER_H

#include <SDL3/SDL.h>
#include <SDL_ttf.h>       // Necessary for TTF_Font and SDL_ttf functions
#include <string>
#include <vector>
#include <map>
#include <sstream>         // Used for creating cache keys

/**
 * @brief Structure to store a rendered texture and its dimensions in the cache.
 */
struct CachedTextureInfo {
    SDL_Texture* texture;   ///< The rendered text texture.
    int width;              ///< Width of the texture in pixels.
    int height;             ///< Height of the texture in pixels.
};

/**
 * @brief Singleton class for managing font loading, text rendering, and caching using SDL_ttf.
 *
 * This class abstracts the low-level SDL_ttf and SDL rendering calls and manages
 * cross-platform font loading (assets on Android, filesystem elsewhere).
 */
class TextRenderer {
public:
    /**
     * @brief Provides access to the single instance of the TextRenderer (Singleton pattern).
     * @return Reference to the singleton TextRenderer object.
     */
    static TextRenderer& getInstance(); // Singleton access

    /**
     * @brief Initializes the SDL_ttf subsystem and locates a fallback font.
     *
     * Must be called successfully before any rendering operations.
     *
     * @param renderer The active SDL_Renderer instance to use for texture creation.
     * @param preferredFamilies Optional hint for font family preference (currently unused in fallback logic).
     */
    void init(SDL_Renderer* renderer, const std::vector<std::string>& preferredFamilies = {});

    /**
     * @brief Renders the given text at the specified screen coordinates using the internal texture cache.
     *
     * If the text/size combination is not cached, it is rendered to a new texture and cached.
     *
     * @param text The string content to draw.
     * @param x The screen X coordinate (top-left).
     * @param y The screen Y coordinate (top-left).
     * @param color The color of the text.
     * @param fontSize The size of the font.
     */
    void renderText(const std::string& text, int x, int y, SDL_Color color, int fontSize);

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
     * @return An SDL_Point containing the width (x) and height (y).
     */
    SDL_Point getTextSize(const std::string& text, int fontSize); // Convenience

    /**
     * @brief Destroys all cached textures and closes all loaded TTF_Font objects.
     */
    void clearCache(); // Clear both font and texture caches

    // -------------------------------------------------------------------------
    // --- Immediate Mode Functionality Starts Here ---
    // -------------------------------------------------------------------------

    /**
     * @brief Immediate Mode: Renders text into a new SDL_Texture without caching it.
     *
     * Useful for dynamic text that changes every frame. The caller is responsible for
     * destroying the returned texture via SDL_DestroyTexture when it is no longer needed.
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
    // --- Immediate Mode Functionality Ends Here ---
    // -------------------------------------------------------------------------

    /**
     * @brief Getter for the internal SDL_Renderer pointer.
     * @return The SDL_Renderer pointer.
     */
    SDL_Renderer* getRenderer() const;

    /**
     * @brief Checks the initialization status of the TextRenderer.
     * @return true if SDL_ttf and the font path are loaded, false otherwise.
     */
    bool isInitialized() const;

    /**
     * @brief Renders the given text into an SDL_Texture, utilizing or updating the texture cache.
     *
     * This function is the internal mechanism for cached rendering (Retained Mode).
     * The returned texture is owned by the cache and should not be destroyed by the caller.
     *
     * @param text The string to render.
     * @param color The SDL_Color to use for the text.
     * @param fontSize The desired font size.
     * @param outW Output parameter for the resulting texture width.
     * @param outH Output parameter for the resulting texture height.
     * @return A pointer to the cached SDL_Texture, or nullptr on failure.
     */
    SDL_Texture* renderTextToTexture(const std::string& text, SDL_Color color, int fontSize, int& outW, int& outH);

    /**
     * @brief Returns the font's ascent and descent metrics for the given size.
     *
     * @param fontSize The size of the font.
     * @param outAscent Output: The height above the baseline (in pixels).
     * @param outDescent Output: The depth below the baseline (in pixels, returned as a positive value).
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
     * @brief Private destructor for singleton cleanup.
     */
    ~TextRenderer();

    // Prevent copying and assignment to maintain singleton integrity
    TextRenderer(const TextRenderer&) = delete;
    TextRenderer& operator=(const TextRenderer&) = delete;

    // Internal state and resources
    SDL_Renderer* m_renderer;           ///< The SDL renderer context for creating textures.
    bool m_initialized;                 ///< Flag indicating successful initialization.
    std::string m_fontPath;             ///< The file path or asset name of the selected font.

    // Caching for reusable resources
    std::map<int, TTF_Font*> m_fontsBySize;     ///< Map caching TTF_Font objects keyed by size.
    std::map<std::string, CachedTextureInfo> m_textureCache; ///< Map caching rendered text textures keyed by text/size combination.

    /**
     * @brief Internal helper to generate a unique key for the texture cache.
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