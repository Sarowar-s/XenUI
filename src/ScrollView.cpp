//
// SPDX-License-Identifier: Apache-2.0
/*
 *  Copyright (C) 2025 MD S M Sarowar Hossain
 */
//
// Implements the retained-mode ScrollView container, which provides a viewport
// for scrollable content, managing child controls, layout, and scroll mechanics.
//

#include "ScrollView.h"
#include "WindowUtil.h" 
#include <algorithm>    
#include <stdexcept>   
#include <iostream>   

// --- Retained Mode ScrollView Implementation Starts Here ---

ScrollView::ScrollView(const XenUI::PositionParams& posParams, const ScrollViewStyle& style)
    : m_posParams(posParams), m_style(style) {
    // ScrollView requires explicit size to define the viewport area.
    if (m_posParams.width <= 0 || m_posParams.height <= 0) {
        throw std::invalid_argument("ScrollView requires an explicit width and height in PositionParams.");
    }
    
    SDL_Point winSize = XenUI::GetWindowSize();
    recalculateLayout(winSize.x, winSize.y); 
}

bool ScrollView::handleEvent(const SDL_Event& e) {
    return handleEvent(e, nullptr, SDL_FPoint{0.0f, 0.0f});
}

void ScrollView::addControl(std::unique_ptr<IControl> control) {
    if (!control) return;
    m_controls.push_back(std::move(control));

    m_controls.back()->recalculateLayout(static_cast<int>(m_viewRect.w), static_cast<int>(m_bounds.h));
    updateContentHeight();
   
    float maxScrollY = std::max(0.0f, m_contentHeight - m_viewRect.h);
    m_scrollY = std::clamp(m_scrollY, 0.0f, maxScrollY);
}


void ScrollView::updateContentHeight() {
    m_contentHeight = 0.0f;
    for (const auto& control : m_controls) {
        SDL_FRect childBounds = control->getBounds();
        m_contentHeight = std::max(m_contentHeight, childBounds.y + childBounds.h);
    }
}


void ScrollView::recalculateLayout(int parentWidth, int parentHeight) {
    
    int resolvedW = m_posParams.width > 0 ? m_posParams.width : parentWidth;
    int resolvedH = m_posParams.height > 0 ? m_posParams.height : parentHeight;

    SDL_Point finalPos = XenUI::CalculateFinalPosition(m_posParams, resolvedW, resolvedH, parentWidth, parentHeight);
    m_bounds = { static_cast<float>(finalPos.x), static_cast<float>(finalPos.y),
                 static_cast<float>(resolvedW), static_cast<float>(resolvedH) };

   
    float viewW = std::max(0.0f, m_bounds.w - static_cast<float>(m_style.scrollbarWidth));
    float viewH = std::max(0.0f, m_bounds.h);
    m_viewRect = { m_bounds.x, m_bounds.y, viewW, viewH };

      int pass1ParentW = static_cast<int>(m_viewRect.w);
    int pass1ParentH = static_cast<int>(m_viewRect.h);

    for (const auto& control : m_controls) {
       
        control->recalculateLayout(pass1ParentW, pass1ParentH);
    }

    
    float maxBottom = 0.0f;
    for (const auto& control : m_controls) {
        SDL_FRect b = control->getBounds();
        maxBottom = std::max(maxBottom, b.y + b.h);
    }

  
    m_contentHeight = maxBottom;

    
    if (m_contentHeight < m_viewRect.h) {
        m_contentHeight = m_viewRect.h;
    }
    int pass2ParentW = static_cast<int>(m_viewRect.w);
   
    int pass2ParentH = static_cast<int>(m_viewRect.h);
    for (const auto& control : m_controls) {
        control->recalculateLayout(pass2ParentW, pass2ParentH);
    }

   
    maxBottom = 0.0f;
    for (const auto& control : m_controls) {
        SDL_FRect b = control->getBounds();
        maxBottom = std::max(maxBottom, b.y + b.h);
    }
    m_contentHeight = maxBottom;
    if (m_contentHeight < m_viewRect.h) {
        m_contentHeight = m_viewRect.h;
    }

   
    float maxScrollY = std::max(0.0f, m_contentHeight - m_viewRect.h);
    m_scrollY = std::clamp(m_scrollY, 0.0f, maxScrollY);
}


bool ScrollView::handleEvent(const SDL_Event& evt, SDL_Window* window, const SDL_FPoint& parentViewOffset) {
   
    SDL_Window* useWindow = window;
   
    if (!useWindow) {
        if (evt.type == SDL_EVENT_MOUSE_BUTTON_DOWN || evt.type == SDL_EVENT_MOUSE_BUTTON_UP) {
            useWindow = SDL_GetWindowFromID(evt.button.windowID);
        } else {
            useWindow = SDL_GetKeyboardFocus();
            if (!useWindow) useWindow = SDL_GetMouseFocus();
        }
    }

  
    SDL_FPoint mousePos = {0.0f, 0.0f};
    bool haveMouseCoords = false;

   
    switch (evt.type) {
    case SDL_EVENT_MOUSE_MOTION:
        mousePos.x = evt.motion.x;
        mousePos.y = evt.motion.y;
        haveMouseCoords = true;
        break;

    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_BUTTON_UP:
        mousePos.x = evt.button.x;
        mousePos.y = evt.button.y;
        haveMouseCoords = true;
        break;

    case SDL_EVENT_MOUSE_WHEEL:
       
        SDL_GetMouseState(&mousePos.x, &mousePos.y);
        haveMouseCoords = true;
        break;

    default:
        break;
    }

    if (!haveMouseCoords) {
       
        float gx = 0.0f, gy = 0.0f;
        SDL_GetGlobalMouseState(&gx, &gy);

        if (useWindow) {
            
            int winX = 0, winY = 0;
            if (SDL_GetWindowPosition(useWindow, &winX, &winY)) {
                int top=0, left=0, bottom=0, right=0;
                SDL_GetWindowBordersSize(useWindow, &top, &left, &bottom, &right);

                float clientOriginXpx = (float)winX + (float)left;
                float clientOriginYpx = (float)winY + (float)top;
                float clientXpx = gx - clientOriginXpx;
                float clientYpx = gy - clientOriginYpx;

                int winW = 0, winH = 0;
                int winWpx = 0, winHpx = 0;
                SDL_GetWindowSize(useWindow, &winW, &winH);
                SDL_GetWindowSizeInPixels(useWindow, &winWpx, &winHpx);

                float scaleX = 1.0f, scaleY = 1.0f;
                if (winW > 0 && winWpx > 0) scaleX = (float)winWpx / (float)winW;
                if (winH > 0 && winHpx > 0) scaleY = (float)winHpx / (float)winH;

                mousePos.x = clientXpx / scaleX;
                mousePos.y = clientYpx / scaleY;
                haveMouseCoords = true;
            }
        }

        // Last fallback: use SDL_GetMouseState (window-relative to focused window)
        if (!haveMouseCoords) {
            float mx=0.0f, my=0.0f;
            SDL_GetMouseState(&mx, &my);
            mousePos.x = mx;
            mousePos.y = my;
            haveMouseCoords = true;
        }
    }
    // --- End Mouse Position Resolution ---

   
    const SDL_FRect absViewRect = { m_viewRect.x + parentViewOffset.x, m_viewRect.y + parentViewOffset.y, m_viewRect.w, m_viewRect.h };

    // --- Scrollbar Interaction Handling (Mouse) ---

   
    if (evt.type == SDL_EVENT_MOUSE_MOTION) {
        const SDL_FRect thumbRel = getScrollbarThumbRect({0.0f, 0.0f});
        
        const SDL_FRect absThumb = { absViewRect.x + absViewRect.w, absViewRect.y + thumbRel.y, thumbRel.w, thumbRel.h };
        m_isScrollbarHovered = SDL_PointInRectFloat(&mousePos, &absThumb);
    }

   
    if (m_isScrollbarGrabbed) {
        if (evt.type == SDL_EVENT_MOUSE_MOTION) {
            float trackTop = absViewRect.y;
            float trackHeight = absViewRect.h;
            const SDL_FRect thumbRel = getScrollbarThumbRect({0.0f, 0.0f});
            float thumbHeight = thumbRel.h;
            float scrollableTrack = trackHeight - thumbHeight;

            if (scrollableTrack > 0.0f) {
              
                float newThumbYAbs = mousePos.y - m_scrollbarGrabOffsetY;
               
                float pct = std::clamp((newThumbYAbs - trackTop) / scrollableTrack, 0.0f, 1.0f);
                float maxScroll = std::max(0.0f, m_contentHeight - m_viewRect.h);
                
                m_scrollY = pct * maxScroll;
            }
            return true;
        }

        if (evt.type == SDL_EVENT_MOUSE_BUTTON_UP && evt.button.button == SDL_BUTTON_LEFT) {
            m_isScrollbarGrabbed = false;
            return true;
        }
    }

    // Start new scrollbar drag
    if (evt.type == SDL_EVENT_MOUSE_BUTTON_DOWN && evt.button.button == SDL_BUTTON_LEFT) {
        const SDL_FRect thumbRel = getScrollbarThumbRect({0.0f, 0.0f});
        const SDL_FRect absThumb = { absViewRect.x + absViewRect.w, absViewRect.y + thumbRel.y, thumbRel.w, thumbRel.h };

        if (SDL_PointInRectFloat(&mousePos, &absThumb)) {
            m_isScrollbarGrabbed = true;
            
            m_scrollbarGrabOffsetY = mousePos.y - absThumb.y;
            return true;
        }
    }

    // --- Input Forwarding and Focus Management ---

   
    const bool isMouseInsideView = SDL_PointInRectFloat(&mousePos, &absViewRect);

  
    if (evt.type == SDL_EVENT_MOUSE_BUTTON_DOWN && !isMouseInsideView) {
        if (m_focusedChild) {
            m_focusedChild->unfocus(useWindow);
            m_focusedChild = nullptr;
        }
        return false; 
    }

   
    if (!isMouseInsideView && evt.type != SDL_EVENT_MOUSE_WHEEL && evt.type != SDL_EVENT_FINGER_DOWN) {
        return false;
    }

   
    if (evt.type == SDL_EVENT_MOUSE_WHEEL) {
        m_scrollY -= evt.wheel.y * 25.0f; 
        m_scrollY = std::clamp(m_scrollY, 0.0f, std::max(0.0f, m_contentHeight - m_viewRect.h));
        return true;
    }

    
    const SDL_FPoint contentMousePos = { mousePos.x - absViewRect.x, mousePos.y - absViewRect.y + m_scrollY };

    // The screen-space offset required to draw children correctly
    const SDL_FPoint childViewOffset = {
        absViewRect.x,
        absViewRect.y - m_scrollY 
    };

    
    IControl* mouseTargetChild = nullptr;
    const bool isMouseEvent = (evt.type == SDL_EVENT_MOUSE_MOTION || evt.type == SDL_EVENT_MOUSE_BUTTON_DOWN || evt.type == SDL_EVENT_MOUSE_BUTTON_UP);

    if (isMouseEvent) {
        
        for (int i = static_cast<int>(m_controls.size()) - 1; i >= 0; --i) {
            auto &ctrl = m_controls[static_cast<size_t>(i)];
            if (!ctrl) continue;
            
            bool inside = ctrl->isInside(contentMousePos);
            if (inside) {
                mouseTargetChild = ctrl.get();
                break;
            }
        }
    }

  
    if (evt.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
      
        if (mouseTargetChild != m_focusedChild) {
            if (m_focusedChild) {
                m_focusedChild->unfocus(useWindow);
            }
            m_focusedChild = mouseTargetChild;
            if (m_focusedChild) {
              
                m_focusedChild->setWindow(useWindow);
                m_focusedChild->setViewOffset(childViewOffset);
                m_focusedChild->focus(useWindow);
            }
        }
    }

   

    const bool isKeyboardEvent = (evt.type == SDL_EVENT_KEY_DOWN || evt.type == SDL_EVENT_KEY_UP ||
                                  evt.type == SDL_EVENT_TEXT_INPUT || evt.type == SDL_EVENT_TEXT_EDITING);

    
    if (isKeyboardEvent && m_focusedChild) {
      
        if (m_focusedChild->handleEvent(evt, useWindow, childViewOffset)) {
            return true;
        }
    }
   
    else if (isMouseEvent && mouseTargetChild) {
        
        SDL_Event translatedEvt = evt;
        if (evt.type == SDL_EVENT_MOUSE_MOTION) {
            translatedEvt.motion.x = static_cast<Sint32>(contentMousePos.x);
            translatedEvt.motion.y = static_cast<Sint32>(contentMousePos.y);
        } else if (evt.type == SDL_EVENT_MOUSE_BUTTON_DOWN || evt.type == SDL_EVENT_MOUSE_BUTTON_UP) {
            translatedEvt.button.x = static_cast<Sint32>(contentMousePos.x);
            translatedEvt.button.y = static_cast<Sint32>(contentMousePos.y);
        }

        
        if (mouseTargetChild->handleEvent(translatedEvt, useWindow, childViewOffset)) {
            return true;
        }
    }


       if (evt.type == SDL_EVENT_FINGER_DOWN || evt.type == SDL_EVENT_FINGER_MOTION || evt.type == SDL_EVENT_FINGER_UP) {
        SDL_Window* w = useWindow ? useWindow : SDL_GetKeyboardFocus();
        int winW = 0, winH = 0;
        if (w) SDL_GetWindowSize(w, &winW, &winH);

       
        float fx = evt.tfinger.x;
        float fy = evt.tfinger.y;

       
        float touchX = fx * (winW > 0 ? (float)winW : 1.0f);
        float touchY = fy * (winH > 0 ? (float)winH : 1.0f);

      
        const bool isTouchInsideView = SDL_PointInRectFloat(&mousePos, &absViewRect);

       
        mousePos.x = touchX;
        mousePos.y = touchY;
        haveMouseCoords = true; 

        if (evt.type == SDL_EVENT_FINGER_DOWN && isTouchInsideView) {
            m_touchActive = true;
            m_lastTouchY = touchY;
            m_activeTouchId = evt.tfinger.touchID; // Track touch for multi-touch scenarios
           
            return true;
        } else if (evt.type == SDL_EVENT_FINGER_MOTION && m_touchActive && evt.tfinger.touchID == m_activeTouchId) {
           
            float dy = m_lastTouchY - touchY;
            m_scrollY += dy;
            m_scrollY = std::clamp(m_scrollY, 0.0f, std::max(0.0f, m_contentHeight - m_viewRect.h));
            m_lastTouchY = touchY;
            return true; 
        } else if (evt.type == SDL_EVENT_FINGER_UP && evt.tfinger.touchID == m_activeTouchId) {
            m_touchActive = false;
            m_activeTouchId = 0;
            
        }
    }

    return false; 
}




void ScrollView::draw(SDL_Renderer* renderer, const SDL_FPoint& parentViewOffset) {
    if (!renderer) return;

   
    SDL_FRect finalBounds = { m_bounds.x + parentViewOffset.x, m_bounds.y + parentViewOffset.y, m_bounds.w, m_bounds.h };
    SDL_FRect finalViewRect = { m_viewRect.x + parentViewOffset.x, m_viewRect.y + parentViewOffset.y, m_viewRect.w, m_viewRect.h };

  
    if (m_style.drawBackground) {
        SDL_SetRenderDrawColor(renderer, m_style.bgColor.r, m_style.bgColor.g, m_style.bgColor.b, m_style.bgColor.a);
        SDL_RenderFillRect(renderer, &finalBounds);
    }

   
    SDL_Rect oldClipRect;
    bool hadClip = SDL_RenderClipEnabled(renderer);
    
    if (hadClip) {
        SDL_GetRenderClipRect(renderer, &oldClipRect);
    }

   
    SDL_Rect clipRect = { static_cast<int>(finalViewRect.x), static_cast<int>(finalViewRect.y), static_cast<int>(finalViewRect.w), static_cast<int>(finalViewRect.h) };
    SDL_SetRenderClipRect(renderer, &clipRect);

  
  
    SDL_FPoint childOffset = { finalViewRect.x, finalViewRect.y - m_scrollY };
   
    for (auto& control : m_controls) {
        control->draw(renderer, childOffset);
    }

   
    if (hadClip) {
        SDL_SetRenderClipRect(renderer, &oldClipRect);
    } else {
        SDL_SetRenderClipRect(renderer, nullptr);
    }

   
    if (m_style.drawBorder) {
        SDL_SetRenderDrawColor(renderer, m_style.borderColor.r, m_style.borderColor.g, m_style.borderColor.b, m_style.borderColor.a);
        SDL_RenderRect(renderer, &finalBounds);
    }

   
    drawScrollbar(renderer, parentViewOffset);
}


SDL_FRect ScrollView::getScrollbarThumbRect(const SDL_FPoint& /*viewOffset*/) const {
    
    if (m_contentHeight <= m_viewRect.h) {
        return {0.0f, 0.0f, 0.0f, 0.0f}; // No scrollbar needed
    }

    float trackHeight = m_viewRect.h;
   
    float thumbHeight = std::max(20.0f, (trackHeight / m_contentHeight) * trackHeight);
    
    float scrollableTrackHeight = trackHeight - thumbHeight;
   
    float scrollableContentHeight = m_contentHeight - m_viewRect.h;

    float thumbY = 0.0f;
    if (scrollableContentHeight > 0.0f) {
       
        thumbY = (m_scrollY / scrollableContentHeight) * scrollableTrackHeight;
    }

    
    return { m_viewRect.w, thumbY, static_cast<float>(m_style.scrollbarWidth), thumbHeight };
}

void ScrollView::drawScrollbar(SDL_Renderer* renderer, const SDL_FPoint& parentViewOffset) {
   
    if (m_contentHeight <= m_viewRect.h) return;

   
    SDL_FRect finalBounds = { m_bounds.x + parentViewOffset.x, m_bounds.y + parentViewOffset.y, m_bounds.w, m_bounds.h };

    SDL_FRect scrollbarTrackRect = { finalBounds.x + finalBounds.w - static_cast<float>(m_style.scrollbarWidth), finalBounds.y, static_cast<float>(m_style.scrollbarWidth), finalBounds.h };
    SDL_SetRenderDrawColor(renderer, m_style.scrollbarBgColor.r, m_style.scrollbarBgColor.g, m_style.scrollbarBgColor.b, m_style.scrollbarBgColor.a);
    SDL_RenderFillRect(renderer, &scrollbarTrackRect);

   
    SDL_FRect thumbRelativeRect = getScrollbarThumbRect({0.0f, 0.0f});
    SDL_FRect scrollbarThumbRect = { finalBounds.x + finalBounds.w - static_cast<float>(m_style.scrollbarWidth), finalBounds.y + thumbRelativeRect.y, thumbRelativeRect.w, thumbRelativeRect.h };

   
    SDL_Color thumbColor = m_style.scrollbarThumbColor;
    if (m_isScrollbarGrabbed) {
        thumbColor = m_style.scrollbarThumbGrabbedColor;
    } else if (m_isScrollbarHovered) {
        thumbColor = m_style.scrollbarThumbHoverColor;
    }

   
    SDL_SetRenderDrawColor(renderer, thumbColor.r, thumbColor.g, thumbColor.b, thumbColor.a);
    SDL_RenderFillRect(renderer, &scrollbarThumbRect);
}


// --- Retained Mode ScrollView Implementation Ends Here ---


// --- Immediate Mode ScrollView Implementation Starts Here ---


#include <unordered_map>
#include <vector>


struct ClipState {
    bool hadClip;       
    SDL_Rect oldClip;  
    std::string id;     
};
static std::vector<ClipState> g_clipStack; 


struct ScrollState {
    SDL_FPoint pos{0.0f, 0.0f};        
    SDL_FRect view;                   
    SDL_FPoint content;              
    bool thumb_dragging = false;    
    bool content_dragging = false; 
    float grabOffsetY = 0.0f;     
    float last_y = 0.0f;         
    ScrollViewStyle style_;     
    uint64_t activeFingerId = 0;
};
static std::unordered_map<std::string, ScrollState> g_states; 


static SDL_FRect ResolveViewRectFromParams(const XenUI::PositionParams& p, int w, int h, int parentW = -1, int parentH = -1) {
   
    if (parentW <= 0 || parentH <= 0) {
        SDL_Point win = XenUI::GetWindowSize();
        parentW = win.x;
        parentH = win.y;
    }
  
    SDL_Point pos = XenUI::CalculateFinalPosition(p, w, h, parentW, parentH);
    return { (float)pos.x, (float)pos.y, (float)w, (float)h };
}

namespace XenUI {

SDL_FPoint BeginScrollView(
    const std::string&            id,
    const XenUI::PositionParams&  posParams,
    int                           viewWidth,
    int                           viewHeight,
    const SDL_FPoint&             contentSize,
    SDL_Renderer* renderer,
    const SDL_Event&              event,
    const ScrollViewStyle&        style)
{
    SDL_FRect viewRect = ResolveViewRectFromParams(posParams, viewWidth, viewHeight);
    return BeginScrollView(id, viewRect, contentSize, renderer, event, style);
}


SDL_FPoint BeginScrollView(
    const std::string&            id,
    const XenUI::PositionParams&  posParams,
    int                           viewWidth,
    int                           viewHeight,
    int                           parentWidth,
    int                           parentHeight,
    const SDL_FPoint&             contentSize,
    SDL_Renderer* renderer,
    const SDL_Event&              event,
    const ScrollViewStyle&        style)
{

    SDL_FRect viewRect = ResolveViewRectFromParams(posParams, viewWidth, viewHeight, parentWidth, parentHeight);
    return BeginScrollView(id, viewRect, contentSize, renderer, event, style);
}


bool IsScrollViewDragging(const std::string& id) {
    auto it = g_states.find(id);
    return (it != g_states.end()) ? (it->second.thumb_dragging || it->second.content_dragging) : false;
}

SDL_FPoint BeginScrollView(
    const std::string& id,
    const SDL_FRect& viewRect,
    const SDL_FPoint& contentSize,
    SDL_Renderer* renderer,
    const SDL_Event& event,
    const ScrollViewStyle& style) {

    // Draw background
    if (style.drawBackground) {
        SDL_SetRenderDrawColor(renderer, style.bgColor.r, style.bgColor.g, style.bgColor.b, style.bgColor.a);
        SDL_RenderFillRect(renderer, &viewRect);
    }

       bool had = SDL_RenderClipEnabled(renderer);
    SDL_Rect old;
    if (had) SDL_GetRenderClipRect(renderer, &old);
    g_clipStack.push_back({had, old, id});

    
    bool needsScrollbar = (contentSize.y > viewRect.h);
    float scrollbarW = static_cast<float>(style.scrollbarWidth);
    int clipW = needsScrollbar ? static_cast<int>(viewRect.w - scrollbarW) : static_cast<int>(viewRect.w);

    
    SDL_Rect clip = { static_cast<int>(viewRect.x), static_cast<int>(viewRect.y), clipW, static_cast<int>(viewRect.h) };
    SDL_SetRenderClipRect(renderer, &clip);
    auto& st = g_states[id];
    st.view = viewRect;
    st.content = contentSize;
    st.style_ = style;

   
    SDL_FPoint mp{0.0f, 0.0f};
    bool haveMP = false;

   
    if (event.type == SDL_EVENT_FINGER_DOWN ||
        event.type == SDL_EVENT_FINGER_MOTION ||
        event.type == SDL_EVENT_FINGER_UP) {

      
        SDL_Log("BeginScrollView: finger event type=%d fingerId=%llu x=%f y=%f window=%u",
                event.type,
                (unsigned long long)event.tfinger.touchID,
                (double)event.tfinger.x,
                (double)event.tfinger.y,
                (unsigned)event.tfinger.windowID);

        
        SDL_Window* useWindow = nullptr;
        if (event.tfinger.windowID != 0) {
            useWindow = SDL_GetWindowFromID(event.tfinger.windowID);
        }
        if (!useWindow) {
            useWindow = SDL_GetKeyboardFocus();
            if (!useWindow) useWindow = SDL_GetMouseFocus();
        }

        int winW = 0, winH = 0;
        if (useWindow) {
            SDL_GetWindowSize(useWindow, &winW, &winH);
        }

       
        mp.x = (winW > 0) ? (event.tfinger.x * (float)winW) : event.tfinger.x;
        mp.y = (winH > 0) ? (event.tfinger.y * (float)winH) : event.tfinger.y;
        haveMP = true;

       
        uint64_t fid = event.tfinger.touchID;

        if (event.type == SDL_EVENT_FINGER_DOWN) {
           
            st.activeFingerId = fid;
        }
        else if (event.type == SDL_EVENT_FINGER_UP) {
           
            if (st.activeFingerId == fid) {
                st.thumb_dragging = false;
                st.content_dragging = false;
                st.activeFingerId = 0;
            }
        }
    }
   
    else {
        SDL_GetMouseState(&mp.x, &mp.y);
        haveMP = true;
    }

   
    if (event.type == SDL_EVENT_MOUSE_WHEEL) {
       
        if (haveMP && SDL_PointInRectFloat(&mp, &viewRect)) {
            st.pos.y -= event.wheel.y * 25.0f;
        }
    }

       float trackH = viewRect.h;
    float thumbH = std::max(20.0f, (trackH / contentSize.y) * trackH);
    float scrollable = std::max(0.0f, contentSize.y - trackH);
    float rangeH = trackH - thumbH;
    float thumbY = viewRect.y + (scrollable > 0.0f ? (st.pos.y / scrollable) * rangeH : 0.0f);

    SDL_FRect thumbRect = { viewRect.x + viewRect.w - scrollbarW, thumbY, scrollbarW, thumbH };

    bool overThumb = haveMP && SDL_PointInRectFloat(&mp, &thumbRect);

    if ((event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button == SDL_BUTTON_LEFT) ||
        (event.type == SDL_EVENT_FINGER_DOWN)) {

        if (overThumb) {
            // Start thumb drag
            st.thumb_dragging = true;
            st.content_dragging = false; 
            st.grabOffsetY = mp.y - thumbY;
            if (event.type == SDL_EVENT_FINGER_DOWN) st.activeFingerId = event.tfinger.touchID;
        } else if (haveMP && SDL_PointInRectFloat(&mp, &viewRect)) {
           
            st.content_dragging = true;
            st.thumb_dragging = false;
            st.last_y = mp.y;
            if (event.type == SDL_EVENT_FINGER_DOWN) st.activeFingerId = event.tfinger.touchID;
        }
    }
   
    else if ((event.type == SDL_EVENT_MOUSE_BUTTON_UP && event.button.button == SDL_BUTTON_LEFT) ||
             (event.type == SDL_EVENT_FINGER_UP)) {
        
        if (event.type == SDL_EVENT_FINGER_UP) {
            if (st.activeFingerId == event.tfinger.touchID) {
                st.thumb_dragging = false;
                st.content_dragging = false;
                st.activeFingerId = 0;
            }
        } else {
           
            st.thumb_dragging = false;
            st.content_dragging = false;
        }
    }
    
    else if ((event.type == SDL_EVENT_MOUSE_MOTION) ||
             (event.type == SDL_EVENT_FINGER_MOTION)) {

       
        if (st.thumb_dragging) {
            bool process = true;
         
            if (event.type == SDL_EVENT_FINGER_MOTION && st.activeFingerId != event.tfinger.touchID) {
                process = false;
            }
            if (process) {
               
                float rel = mp.y - st.grabOffsetY - viewRect.y;
                float pct = (rangeH > 0.0f) ? std::clamp(rel / rangeH, 0.0f, 1.0f) : 0.0f;
                st.pos.y = pct * scrollable;
            }
        }

       
        if (st.content_dragging) {
            bool process = true;
          
            if (event.type == SDL_EVENT_FINGER_MOTION && st.activeFingerId != event.tfinger.touchID) {
                process = false;
            }
            if (process) {
             
                float delta = st.last_y - mp.y;
                st.pos.y += delta;
                st.last_y = mp.y; 
            }
        }
    }

  
    st.pos.y = std::clamp(st.pos.y, 0.0f, scrollable);

    
    return { viewRect.x, viewRect.y - st.pos.y };
}


void EndScrollView(SDL_Renderer* renderer) {
    if (g_clipStack.empty()) return;

   
    ClipState cs = g_clipStack.back();
    g_clipStack.pop_back();

    
    if (cs.hadClip) {
        SDL_SetRenderClipRect(renderer, &cs.oldClip);
    } else {
        SDL_SetRenderClipRect(renderer, nullptr);
    }

    auto& st = g_states[cs.id];
    auto& v = st.view; 
    auto& S = st.style_; 
    auto& c = st.content; 

    
    if (S.drawBorder) {
        SDL_SetRenderDrawColor(renderer, S.borderColor.r, S.borderColor.g, S.borderColor.b, S.borderColor.a);
        SDL_RenderRect(renderer, &v);
    }

   
    if (c.y > v.h) {
       
        float scrollbarW = static_cast<float>(S.scrollbarWidth);
        SDL_FRect track = { v.x + v.w - scrollbarW, v.y, scrollbarW, v.h };
        SDL_SetRenderDrawColor(renderer, S.scrollbarBgColor.r, S.scrollbarBgColor.g, S.scrollbarBgColor.b, S.scrollbarBgColor.a);
        SDL_RenderFillRect(renderer, &track);

      
        float trackH = v.h;
        float thumbH = std::max(20.0f, (trackH / c.y) * trackH);
        float scrollable = std::max(0.0f, c.y - trackH);
        float rangeH = trackH - thumbH;
        float thumbY = v.y + (scrollable > 0.0f ? (st.pos.y / scrollable) * rangeH : 0.0f);

        SDL_FRect thumb = { v.x + v.w - scrollbarW, thumbY, scrollbarW, thumbH };

      
        SDL_FPoint mp;
        SDL_GetMouseState(&mp.x, &mp.y);
        SDL_Color col = S.scrollbarThumbColor;
        if (st.thumb_dragging) {
            col = S.scrollbarThumbGrabbedColor;
        } else if (SDL_PointInRectFloat(&mp, &thumb)) {
            col = S.scrollbarThumbHoverColor;
        }

        // Draw the thumb
        SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, col.a);
        SDL_RenderFillRect(renderer, &thumb);
    }
}
} // namespace XenUI

// --- Immediate Mode ScrollView Implementation Ends Here ---
