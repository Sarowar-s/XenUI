#ifndef TEXTRENDERER_H
#define TEXTRENDERER_H
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
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h> 
#include <string>
#include <vector>
#include <map>
#include <sstream> 

// Structure to hold texture and its dimensions in cache
struct CachedTextureInfo {
    SDL_Texture* texture;
    int width;
    int height;
};

class TextRenderer {
public:
    static TextRenderer& getInstance(); // Singleton access

    // init: renderer is required. preferredFamilies is a hint.
    void init(SDL_Renderer* renderer, const std::vector<std::string>& preferredFamilies = {});
    void renderText(const std::string& text, int x, int y, SDL_Color color, int fontSize);
    void measureText(const std::string& text, int fontSize, int& w, int& h);
    SDL_Point getTextSize(const std::string& text, int fontSize); 
    void clearCache(); // Clear both font and texture caches

    // For immediate-mode rendering (caller manages texture lifecycle)
    SDL_Texture* renderTextImmediateToTexture(const std::string& text, SDL_Color color, int fontSize, int& outW, int& outH);

    SDL_Renderer* getRenderer() const; // Getter for the renderer
    bool isInitialized() const; // Added to check initialization status
    SDL_Texture* renderTextToTexture(const std::string& text, SDL_Color color, int fontSize, int& outW, int& outH);
    /// Returns the font metrics for the given size

void getFontMetrics(int fontSize, int &outAscent, int &outDescent);
 TTF_Font* getFont(int fontSize); 


private:
    TextRenderer(); // Private constructor for singleton
    ~TextRenderer(); // Private destructor

    // Delete copy constructor and assignment operator for singleton
    TextRenderer(const TextRenderer&) = delete;
    TextRenderer& operator=(const TextRenderer&) = delete;

    SDL_Renderer* m_renderer;
    bool m_initialized;
    std::string m_fontPath; 

    // Map to cache fonts by size
    std::map<int, TTF_Font*> m_fontsBySize;
 

    // Map to cache rendered textures
    std::map<std::string, CachedTextureInfo> m_textureCache;
    std::string createCacheKey(const std::string& text, int fontSize); // Helper for cache keys

    // Internal helper to render text to a texture (used by cached renderText)
    

    // Platform-specific font finding
    std::string findBundledFallbackFont();
};

#endif // TEXTRENDERER_H