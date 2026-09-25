// SPDX-License-Identifier: Apache-2.0
/*
 *  Copyright (C) 2025 MD S M Sarowar Hossain
 */
#include "Pager.h"
#include "TextRenderer.h"
#include "WindowUtil.h"
#include <algorithm>
#include <cmath>

using namespace XenUI;

// ====================== RETAINED MODE ======================

Pager::Pager(const std::string& id,
             const PositionParams& posParams,
             float width,
             float height,
             PagerStyle style)
    : m_id(id), m_posParams(posParams), m_style(style)
{
    // Initialize m_width and m_height for the positioning engine
    m_width = width;
    m_height = height;
    m_bounds = {0, 0, width, height};
    m_contentBounds = m_bounds;
    m_targetScrollX = 0.0f;
}

void Pager::updateLayout()
{
    if (m_pages.empty()) return;
    for (auto& page : m_pages) {
        // USE CONTENT BOUNDS, not total bounds, so pages fit around the tab bar
        page.scrollView->recalculateLayout(
            static_cast<int>(m_contentBounds.w),
            static_cast<int>(m_contentBounds.h)
        );
    }
}

void Pager::addPage(const std::string& title, std::vector<std::unique_ptr<IControl>> pageContent)
{
    // Initialize ScrollView to fit inside the content area
    PositionParams svPos = PositionParams::Absolute(0, 0,
        static_cast<int>(m_contentBounds.w), static_cast<int>(m_contentBounds.h));

    auto sv = std::make_unique<ScrollView>(svPos, ScrollViewStyle{});

    for (auto& ctrl : pageContent) {
        sv->addControl(std::move(ctrl));
    }

    Page p{title, std::move(sv)};
    m_pages.push_back(std::move(p));
    updateLayout();
}

void Pager::setCurrentPage(size_t index)
{
    if (index >= m_pages.size()) return;
    m_currentPage = index;
    // Scroll based on content width, not total screen width
    m_targetScrollX = m_currentPage * m_contentBounds.w;
    if (m_onPageChanged) m_onPageChanged(index);
}


void Pager::recalculateLayout(int parentWidth, int parentHeight) {
    // 1. Resolve overall Pager bounds relative to ITS parent
    SDL_Point pos = XenUI::CalculateFinalPosition(m_posParams, m_width, m_height, parentWidth, parentHeight);
    m_bounds = { (float)pos.x, (float)pos.y, (float)m_width, (float)m_height };

    if (!m_style.showTabs || m_pages.empty()) {
        m_tabBarBounds = {0, 0, 0, 0};
        m_contentBounds = m_bounds; // Pages take full space
        
        updateLayout(); // [FIX APPLIED HERE]
        return;
    }

    // 2. Resolve TabBar bounds relative to the PAGER
    int tbWidth = (m_style.tabBarPosition.width > 0) ? m_style.tabBarPosition.width : m_width;
    int tbHeight = m_style.tabBarHeight;

    SDL_Point tbPos = XenUI::CalculateFinalPosition(
        m_style.tabBarPosition, 
        tbWidth, 
        tbHeight, 
        m_width, 
        m_height
    );

    // Apply the Pager's absolute position to the TabBar's relative position
    m_tabBarBounds = {
        m_bounds.x + (float)tbPos.x,
        m_bounds.y + (float)tbPos.y,
        (float)tbWidth,
        (float)tbHeight
    };

    // 3. Resolve Content (Page) Bounds dynamically
    m_contentBounds = m_bounds;
    
    // Heuristic: If tab bar is in the top half, push content down. If bottom half, push up.
    if (tbPos.y < m_height / 2) {
        float occupiedTop = tbPos.y + tbHeight; 
        m_contentBounds.y += occupiedTop;
        m_contentBounds.h -= occupiedTop;
    } else {
        float occupiedBottom = m_height - tbPos.y;
        m_contentBounds.h -= occupiedBottom;
    }

    // [FIX APPLIED HERE]
    // Crucial step: Now that the content bounds are calculated, 
    // force all nested ScrollViews to adopt the new width/height so they don't overflow.
    updateLayout(); 
}

void Pager::update(float /*deltaTime*/)
{
    m_targetScrollX = m_targetScrollX * 0.82f + (m_currentPage * m_contentBounds.w) * 0.18f;
}

bool Pager::handleEvent(const SDL_Event& e, SDL_Window* window, const SDL_FPoint& viewOffset)
{
    bool changed = false;

    // 1. Forward to current page with the EXACT same offset logic used in draw()
    if (m_currentPage < m_pages.size()) {
        SDL_FPoint pageOffset = {
            viewOffset.x + m_contentBounds.x,
            viewOffset.y + m_contentBounds.y
        };
        
        float xOffset = (static_cast<float>(m_currentPage) * m_contentBounds.w) - m_targetScrollX;
        SDL_FPoint finalPageOffset = { pageOffset.x + xOffset, pageOffset.y };

        // Pass the context downward so InputBox/Button can calculate their hitboxes
        if (m_pages[m_currentPage].scrollView->handleEvent(e, window, finalPageOffset)) {
            changed = true;
        }
    }

    // 2. Get mouse position in screen space
    float mouseX = 0.0f, mouseY = 0.0f;
    if (e.type == SDL_EVENT_MOUSE_MOTION) {
        mouseX = (float)e.motion.x;
        mouseY = (float)e.motion.y;
    } else if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN || e.type == SDL_EVENT_MOUSE_BUTTON_UP) {
        mouseX = (float)e.button.x;
        mouseY = (float)e.button.y;
    }

    // 3. TAB BAR CLICK DETECTION (Updated to account for viewOffset)
    if (m_style.showTabs && e.type == SDL_EVENT_MOUSE_BUTTON_DOWN && e.button.button == SDL_BUTTON_LEFT) {
        float tabX = m_tabBarBounds.x + viewOffset.x;
        float tabY = m_tabBarBounds.y + viewOffset.y;

        if (mouseX >= tabX && mouseX <= tabX + m_tabBarBounds.w &&
            mouseY >= tabY && mouseY <= tabY + m_tabBarBounds.h) 
        {
            float tabWidth = m_tabBarBounds.w / std::max(1.0f, (float)m_pages.size());
            int clickedIndex = (int)((mouseX - tabX) / tabWidth);

            if (clickedIndex >= 0 && clickedIndex < (int)m_pages.size()) {
                setCurrentPage(clickedIndex);
                return true; 
            }
        }
    }

    // 4. Swipe support (Updated to account for viewOffset)
    if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN && e.button.button == SDL_BUTTON_LEFT) {
        float boundsX = m_bounds.x + viewOffset.x;
        float boundsY = m_bounds.y + viewOffset.y;

        if (mouseX >= boundsX && mouseX <= boundsX + m_bounds.w &&
            mouseY >= boundsY && mouseY <= boundsY + m_bounds.h) 
        {
            m_isDragging = true;
            m_dragStartX = mouseX;
        }
    } else if (e.type == SDL_EVENT_MOUSE_BUTTON_UP && e.button.button == SDL_BUTTON_LEFT && m_isDragging) {
        m_isDragging = false;
        float delta = mouseX - m_dragStartX;
        if (std::abs(delta) > m_contentBounds.w * m_style.swipeThreshold) {
            if (delta > 0 && m_currentPage > 0) setCurrentPage(m_currentPage - 1);
            else if (delta < 0 && m_currentPage + 1 < m_pages.size()) setCurrentPage(m_currentPage + 1);
        }
    }

    // 5. Arrow keys
    if (e.type == SDL_EVENT_KEY_DOWN) {
        if (e.key.key == SDLK_LEFT && m_currentPage > 0) {
            setCurrentPage(m_currentPage - 1);
            return true;
        }
        if (e.key.key == SDLK_RIGHT && m_currentPage + 1 < m_pages.size()) {
            setCurrentPage(m_currentPage + 1);
            return true;
        }
    }

    return changed;
}

// Route the 1-argument version to the new 3-argument implementation
bool Pager::handleEvent(const SDL_Event& e)
{
    return handleEvent(e, nullptr, {0.0f, 0.0f});
}


void Pager::draw(SDL_Renderer* renderer, const SDL_FPoint& viewOffset)
{
    if (!renderer || m_pages.empty()) return;

    SDL_FRect screenBounds = m_bounds;
    screenBounds.x += viewOffset.x;
    screenBounds.y += viewOffset.y;

    SDL_SetRenderDrawColor(renderer, m_style.backgroundColor.r, m_style.backgroundColor.g,
                           m_style.backgroundColor.b, m_style.backgroundColor.a);
    SDL_RenderFillRect(renderer, &screenBounds);

    // [FIX APPLIED HERE]
    // Pass the absolute content bounds directly as the offset. 
    // The ScrollView's internal origin (0,0) will now correctly start at this absolute position.
    SDL_FPoint pageOffset = {
        viewOffset.x + m_contentBounds.x,
        viewOffset.y + m_contentBounds.y
    };

    for (size_t i = 0; i < m_pages.size(); ++i) {
        float xOffset = (static_cast<float>(i) * m_contentBounds.w) - m_targetScrollX;
        m_pages[i].scrollView->draw(renderer, {pageOffset.x + xOffset, pageOffset.y});
    }

    if (m_style.showTabs) drawTabs(renderer, viewOffset);
    if (m_style.showIndicators) drawIndicators(renderer, viewOffset);
}


void Pager::drawTabs(SDL_Renderer* renderer, const SDL_FPoint& offset)
{
    if (!m_style.showTabs || m_pages.empty()) return;

    // Apply view translation offset to the pre-calculated tab bar bounds
    SDL_FRect renderTabBar = {
        m_tabBarBounds.x + offset.x,
        m_tabBarBounds.y + offset.y,
        m_tabBarBounds.w,
        m_tabBarBounds.h
    };

    // Tab bar background
    SDL_SetRenderDrawColor(renderer, m_style.tabBarColor.r, m_style.tabBarColor.g, m_style.tabBarColor.b, 255);
    SDL_RenderFillRect(renderer, &renderTabBar);

    float tabW = renderTabBar.w / (float)m_pages.size();

    for (size_t i = 0; i < m_pages.size(); ++i) {
        SDL_FRect r = {renderTabBar.x + i * tabW, renderTabBar.y, tabW, renderTabBar.h};
        bool selected = (i == m_currentPage);

        if (selected) {
            SDL_SetRenderDrawColor(renderer, m_style.tabSelectedColor.r, m_style.tabSelectedColor.g,
                                   m_style.tabSelectedColor.b, 255);
            SDL_RenderFillRect(renderer, &r);
        }

        // Text
        if (TextRenderer::getInstance().isInitialized()) {
            int tw, th;
            TextRenderer::getInstance().measureText(m_pages[i].title, m_style.fontSize, tw, th);
            int tx = (int)(r.x + (r.w - tw)/2);
            int ty = (int)(r.y + (r.h - th)/2);
            SDL_Color col = selected ? m_style.tabSelectedTextColor : m_style.tabTextColor;
            TextRenderer::getInstance().renderText(m_pages[i].title, tx, ty, col, m_style.fontSize);
        }
    }
}

void Pager::drawIndicators(SDL_Renderer* renderer, const SDL_FPoint& offset)
{
    if (!m_style.showIndicators || m_pages.size() <= 1) return;

    float y = m_bounds.y + m_bounds.h - 25 + offset.y;
    float spacing = 14.0f;
    float startX = m_bounds.x + m_bounds.w / 2.0f - (m_pages.size() - 1) * spacing / 2.0f + offset.x;

    for (size_t i = 0; i < m_pages.size(); ++i) {
        SDL_Color c = (i == m_currentPage) ? m_style.indicatorColor : m_style.indicatorInactive;
        SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);

        SDL_FRect dot = {
            startX + i * spacing - 5.0f,
            y - 5.0f,
            10.0f, 10.0f
        };
        SDL_RenderFillRect(renderer, &dot);
    }
}

// ====================== IMMEDIATE MODE ======================

namespace XenUI {
namespace Detail {
    static std::unordered_map<std::string, PagerState> pagerStates;
}

bool BeginPager(
    const char* id,
    SDL_Renderer* renderer,
    const PositionParams& posParams,
    float width,
    float height,
    size_t* currentPage,
    const std::vector<std::string>& pageTitles,
    std::function<void(size_t)> onPageChanged,
    PagerStyle style,
    const SDL_FPoint& parentOffset,
    int parentW,
    int parentH)
{
    if (!currentPage || pageTitles.empty() || !renderer) return false;

    auto& st = Detail::pagerStates[id];
    if (st.currentPage != *currentPage) st.currentPage = *currentPage;
    st.currentPage = std::min(st.currentPage, pageTitles.size() - 1);

    SDL_Point pos = CalculateFinalPosition(posParams,
        static_cast<int>(width), static_cast<int>(height), parentW, parentH);

    SDL_FRect bounds = {
        static_cast<float>(pos.x) + parentOffset.x,
        static_cast<float>(pos.y) + parentOffset.y,
        width, height
    };

    // Background
    SDL_SetRenderDrawColor(renderer,
        style.backgroundColor.r, style.backgroundColor.g,
        style.backgroundColor.b, style.backgroundColor.a);
    SDL_RenderFillRect(renderer, &bounds);

    // === DRAW TAB BAR ===
    if (style.showTabs) {
        SDL_FRect tabBar = bounds;
        tabBar.h = (float)style.tabBarHeight;

        // Tab bar background
        SDL_SetRenderDrawColor(renderer, style.tabBarColor.r, style.tabBarColor.g,
                               style.tabBarColor.b, style.tabBarColor.a);
        SDL_RenderFillRect(renderer, &tabBar);

        float tabW = bounds.w / (float)pageTitles.size();

        for (size_t i = 0; i < pageTitles.size(); ++i) {
            SDL_FRect tabRect = { tabBar.x + i * tabW, tabBar.y, tabW, tabBar.h };
            bool selected = (i == st.currentPage);

            if (selected) {
                SDL_SetRenderDrawColor(renderer, style.tabSelectedColor.r, style.tabSelectedColor.g,
                                       style.tabSelectedColor.b, style.tabSelectedColor.a);
                SDL_RenderFillRect(renderer, &tabRect);
            }

            // Draw tab text
            if (TextRenderer::getInstance().isInitialized()) {
                int tw, th;
                TextRenderer::getInstance().measureText(pageTitles[i], style.fontSize, tw, th);
                int tx = static_cast<int>(tabRect.x + (tabRect.w - tw) / 2);
                int ty = static_cast<int>(tabRect.y + (tabRect.h - th) / 2);
                SDL_Color col = selected ? style.tabSelectedTextColor : style.tabTextColor;
                TextRenderer::getInstance().renderText(pageTitles[i], tx, ty, col, style.fontSize);
            }
        }
    }

    // === INPUT HANDLING (Click on tabs) ===
    SDL_FPoint mp{0.0f, 0.0f};
    SDL_GetMouseState(&mp.x, &mp.y);

    if (style.showTabs) {
        SDL_FRect tabBar = bounds;
        tabBar.h = (float)style.tabBarHeight;

        bool mouseDown = (SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON_LMASK) != 0;

        if (mouseDown && mp.y >= tabBar.y && mp.y <= tabBar.y + tabBar.h) {
            float tabW = bounds.w / (float)pageTitles.size();
            int clicked = (int)((mp.x - tabBar.x) / tabW);

            if (clicked >= 0 && clicked < (int)pageTitles.size() && clicked != (int)st.currentPage) {
                st.currentPage = clicked;
                if (onPageChanged) onPageChanged(clicked);
               // std::cout << "Immediate Pager switched to page: " << clicked << std::endl;
            }
        }
    }

    *currentPage = st.currentPage;
    return true;
}
void EndPager(const char* id, SDL_Renderer* renderer)
{
    auto it = Detail::pagerStates.find(id);
    if (it == Detail::pagerStates.end() || !renderer) return;

    auto& st = it->second;

    // Draw indicators at bottom
    // if (/* assume you have pageTitles or pass count */) {
    //     // Simple dot indicators
    // }
}
} // namespace XenUI
