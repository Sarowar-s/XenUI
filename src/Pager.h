//
// SPDX-License-Identifier: Apache-2.0
/*
 * Copyright (C) 2025 MD S M Sarowar Hossain
 */
#ifndef PAGER_H
#define PAGER_H

#include "ScrollView.h"
#include "Position.h"
#include "UIElement.h"
#include <vector>
#include <string>
#include <functional>
#include <unordered_map>

namespace XenUI {

struct PagerStyle {
    SDL_Color backgroundColor   = {25, 25, 30, 255};
    SDL_Color tabBarColor       = {40, 40, 45, 255};
    SDL_Color tabTextColor      = {220, 220, 220, 255};
    SDL_Color tabSelectedColor  = {10, 160, 230, 255};
    SDL_Color indicatorColor    = {10, 180, 240, 255};
    SDL_Color indicatorInactive = {70, 70, 80, 255};

    int   tabBarHeight   = 45;
    int   tabPadding     = 15;
    float swipeThreshold = 0.35f;
    bool  showTabs       = true;
    bool  showIndicators = true;
};

class Pager : public IControl {
public:
    Pager(const std::string& id, const PositionParams& posParams,
          float width, float height, PagerStyle style = {});

    void addPage(const std::string& title, std::vector<std::unique_ptr<IControl>> pageContent);
    void setCurrentPage(size_t index);
    size_t getCurrentPage() const { return m_currentPage; }
    void setOnPageChanged(std::function<void(size_t)> cb) { m_onPageChanged = std::move(cb); }

    bool handleEvent(const SDL_Event& e) override;
    void draw(SDL_Renderer* renderer, const SDL_FPoint& viewOffset) override;
    void recalculateLayout(int parentWidth, int parentHeight) override;
    void update(float deltaTime = 0.016f); // Call from main loop

    SDL_FRect getBounds() const override { return m_bounds; }

private:
    std::string m_id;
    PositionParams m_posParams;
    PagerStyle m_style;

    struct Page {
        std::string title;
        std::unique_ptr<ScrollView> scrollView;
    };

    std::vector<Page> m_pages;
    size_t m_currentPage = 0;
    std::function<void(size_t)> m_onPageChanged;

    SDL_FRect m_bounds{};
    SDL_FRect m_contentRect{};
    float m_targetScrollX = 0.0f;
    bool m_isDragging = false;
    float m_dragStartX = 0.0f;

    void updateLayout();
    void drawTabs(SDL_Renderer* renderer, const SDL_FPoint& offset);
    void drawIndicators(SDL_Renderer* renderer, const SDL_FPoint& offset);
};

namespace Detail {
    struct PagerState {
        size_t currentPage = 0;
        float scrollX = 0.0f;
        float dragStartX = 0.0f;
        bool isDragging = false;
        uint64_t activeFingerId = 0;
    };
}

// ====================== IMMEDIATE MODE ======================
bool BeginPager(
    const char* id,
    SDL_Renderer* renderer,                    // ← Added
    const PositionParams& posParams,
    float width,
    float height,
    size_t* currentPage,
    const std::vector<std::string>& pageTitles,
    std::function<void(size_t)> onPageChanged = nullptr,
    PagerStyle style = {},
    const SDL_FPoint& parentOffset = {0.0f, 0.0f},
    int parentW = -1,
    int parentH = -1);

void EndPager(const char* id, SDL_Renderer* renderer);

} // namespace XenUI

#endif // PAGER_H