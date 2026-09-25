// SPDX-License-Identifier: Apache-2.0
/*
 *  Copyright (C) 2025 MD S M Sarowar Hossain
 */

#ifndef TEXTRENDERER_H
#define TEXTRENDERER_H

#include <SDL3/SDL.h>
#include <SDL_ttf.h> 
#include <string>
#include <vector>
#include <map>
#include <sstream>

struct CachedTextureInfo {
    SDL_Texture* texture;
    int width;
    int height;
};

class TextRenderer {
public:
    static TextRenderer& getInstance(); 

    void init(SDL_Renderer* renderer, const std::vector<std::string>& preferredFamilies = {});
    
    // Updated with optional wrapWidth = 0
    void renderText(const std::string& text, int x, int y, SDL_Color color, int fontSize, int wrapWidth = 0);
    void measureText(const std::string& text, int fontSize, int& w, int& h, int wrapWidth = 0);
    SDL_Point getTextSize(const std::string& text, int fontSize, int wrapWidth = 0);
    
    void clearCache(); 

    SDL_Texture* renderTextImmediateToTexture(const std::string& text, SDL_Color color, int fontSize, int& outW, int& outH, int wrapWidth = 0);
    SDL_Texture* renderTextToTexture(const std::string& text, SDL_Color color, int fontSize, int& outW, int& outH, int wrapWidth = 0);

    SDL_Renderer* getRenderer() const;
    bool isInitialized() const;

    void getFontMetrics(int fontSize, int &outAscent, int &outDescent);
    TTF_Font* getFont(int fontSize); 

private:
    TextRenderer(); 
    ~TextRenderer(); 

    TextRenderer(const TextRenderer&) = delete;
    TextRenderer& operator=(const TextRenderer&) = delete;

    SDL_Renderer* m_renderer;
    bool m_initialized;
    std::string m_fontPath; 
    std::map<int, TTF_Font*> m_fontsBySize;

    std::map<std::string, CachedTextureInfo> m_textureCache;
    std::string createCacheKey(const std::string& text, int fontSize, int wrapWidth); 

    std::string findBundledFallbackFont();
};

#endif // TEXTRENDERER_H
