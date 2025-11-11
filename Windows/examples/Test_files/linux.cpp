
// SPDX-License-Identifier: Apache-2.0
/*
 *  Copyright (C) 2025 MD S M Sarowar Hossain
 */









// main.cpp
// dont remove first two comments
#include <SDL3/SDL.h>
#include <SDL3/SDL_log.h>
#include <SDL_ttf.h>
#include <SDL_image.h>    // <--- SDL3_image no longer needs IMG_Init/IMG_Quit

#include <iostream>
#include <vector>
#include <cstring>  // For memcpy
#include <string>
#include <chrono>
#include "XenUi/Image.h"           // <-- Image wrapper (SDL3)
#include "XenUi/TextRenderer.h"
#include "XenUi/Label.h"
#include "XenUi/InputBox.h"
#include "XenUi/Shape.h"
#include "XenUi/Button.h"
#include "XenUi/WindowUtil.h"
#include "XenUi/Anchor.h"
#include "XenUi/Position.h" // *** ADDED ***
#include "XenUi/RadioButton.h"
#include "XenUi/CheckBox.h"
#include "XenUi/Switch.h"  // Add this line
#include "XenUi/Slider.h"  // Add this line
#include "XenUi/Dropdown.h" 
// At the top of main.cpp, with your other XenUI includes
#include "XenUi/ScrollView.h"
#include "XenUi/UIElement.h"

// near the top of main.cpp
static SDL_Event gLastEvent;


// Global text renderer instance
TextRenderer& textRenderer = TextRenderer::getInstance();
std::vector<Label> labels;
std::vector<Button> buttons;                // Store buttons in a vector
std::vector<XenUI::InputBox> Inputs;        // Store InputBoxes in a vector
std::vector<XenUI::Rectangle> shapes;       // Example retained-mode shapes
std::vector<Checkbox> checkboxes;
std::vector<XenUI::Switch> switches;  // For retained mode switches


// Near your other global UI element vectors
// std::shared_ptr<XenUI::ScrollView> myRetainedScrollView;
std::unique_ptr<ScrollView> myRetainedScrollView;
int scrollX = 0, scrollY = 0; // for immediate mode example


// --- SLIDER ADDITIONS START ---
// Retained Mode Slider Variable
std::vector<Slider> sliders; // Store retained mode sliders
float retainedVolume = 0.5f; // Value for a retained mode slider
float retainedBrightness = 0.8f; // Another retained mode slider value

// Immediate Mode Slider Variable
float immediateAudioLevel = 0.75f; // Value for an immediate mode slider
float Level = 0.5f;
float Level1 = 0.5f;
float Level3 = 0.5f;
float Level4 = 0.5f;
// --- SLIDER ADDITIONS END ---

// --- DROPDOWN ADDITIONS START ---
// Retained Mode Dropdown Variables
std::vector<Dropdown> dropdowns; // Store retained mode dropdown objects
std::vector<std::string> themes = {"Dark", "Light", "High Contrast", "Custom"};
std::vector<std::string> languages = {"English", "Spanish", "French", "German", "Japanese"};
int selectedThemeIndex = 0; // Value for a retained mode dropdown
int selectedLanguageIndex = 0; // Another retained mode dropdown value

// Immediate Mode Dropdown Variables
int immediateSelectedFontIndex = 0; // Value for an immediate mode dropdown
int immediateSelectedFontIndexScroll = 0; // Value for an immediate mode dropdown
// --- DROPDOWN ADDITIONS END ---
int numItems = 15;
bool immediateSwitchState = false;  // For immediate mode switch
bool immediateSwitchState1 = false;  // For immediate mode switch
bool immediateChecked = false;     
bool toggled1 = false;   
bool toggled2 = false;   
bool toggled3 = false;   
bool retainedChecked = false;     
bool retainedChecked1 = false;     
bool retainedChecked2 = false;    
bool retainedChecked3 = false;    

// Let's say you have N buttons in the scrollview
int contentStartY = 10;
int rowGap        = 50;

// Position for the first checkbox (right after all buttons)
float checkboxBaseY = contentStartY + (numItems * rowGap) + 10;

// Global Image pointer
Image* gImage = nullptr;

// Test variables
int fps = 60;
int FPS = 60;

//immediate


// --- ADDITIONS START ---
// Retained Mode Radio Button Variables
int selectedChannel = -1;
std::unique_ptr<RadioButtonGroup> channelGroup = nullptr; // Use a pointer as it's typically heap-allocated

// Immediate Mode Radio Button Variables
int immediateSelectedOption = 0; // Variable for immediate mode radio group
// --- ADDITIONS END ---



// Add this function to main.cpp
void setupAboutDevice(SDL_Renderer* renderer) {
    SDL_Point winSize = XenUI::GetWindowSize();
    myRetainedScrollView = std::make_unique<ScrollView>(
        XenUI::PositionParams::Anchored(XenUI::Anchor::TOP_LEFT, 0, 0, winSize.x, winSize.y)
    );

    // Title
    myRetainedScrollView->addControl(
        std::make_unique<Label>(
            "About device",
            XenUI::PositionParams::Anchored(XenUI::Anchor::TOP_CENTER, 0, 20),
            32,
            SDL_Color{255, 255, 255, 255}
        )
    );

    // Blue banner background (using rectangle, no rounding)
    myRetainedScrollView->addControl(
        std::make_unique<XenUI::Rectangle>(
            XenUI::PositionParams::Absolute(0, 80, winSize.x, 120),
            winSize.x,
            120,
            SDL_Color{50, 100, 200, 255}  // Approximate blue
        )
    );

    // Banner text
    myRetainedScrollView->addControl(
        std::make_unique<Label>(
            "realme UI",
            XenUI::PositionParams::Anchored(XenUI::Anchor::TOP_CENTER, 0, 90),
            48,
            SDL_Color{255, 255, 255, 255}
        )
    );

    myRetainedScrollView->addControl(
        std::make_unique<Label>(
            "realme UI 4.0",
            XenUI::PositionParams::Anchored(XenUI::Anchor::TOP_CENTER, 0, 150),
            24,
            SDL_Color{255, 255, 255, 255}
        )
    );

    myRetainedScrollView->addControl(
        std::make_unique<Label>(
            "Official version",
            XenUI::PositionParams::Anchored(XenUI::Anchor::TOP_CENTER, 0, 180),
            20,
            SDL_Color{255, 255, 255, 255}
        )
    );

    int y = 220;

    // Dark card background for first row (device name + storage)
    myRetainedScrollView->addControl(
        std::make_unique<XenUI::Rectangle>(
            XenUI::PositionParams::Absolute(20, y, winSize.x - 40, 80),
            winSize.x - 40,
            80,
            SDL_Color{30, 30, 30, 255}
        )
    );

    // Device name
    myRetainedScrollView->addControl(
        std::make_unique<Label>(
            "Device name",
            XenUI::PositionParams::Absolute(40, y + 10),
            20,
            SDL_Color{255, 255, 255, 255}
        )
    );

    myRetainedScrollView->addControl(
        std::make_unique<Label>(
            "narzo 50",
            XenUI::PositionParams::Absolute(40, y + 40),
            20,
            SDL_Color{200, 200, 200, 255}
        )
    );

    // Storage
    myRetainedScrollView->addControl(
        std::make_unique<Label>(
            "Storage",
            XenUI::PositionParams::Absolute(winSize.x / 2 + 20, y + 10),
            20,
            SDL_Color{255, 255, 255, 255}
        )
    );

    myRetainedScrollView->addControl(
        std::make_unique<Label>(
            "55.2 GB / 64.0 GB",
            XenUI::PositionParams::Absolute(winSize.x / 2 + 20, y + 40),
            20,
            SDL_Color{200, 200, 200, 255}
        )
    );

    // Storage bar background
    myRetainedScrollView->addControl(
        std::make_unique<XenUI::Rectangle>(
            XenUI::PositionParams::Absolute(40, y + 70, winSize.x - 80, 4),
            winSize.x - 80,
            4,
            SDL_Color{50, 50, 50, 255}
        )
    );

    // Storage used bar
    float usedRatio = 55.2f / 64.0f;
    int usedWidth = static_cast<int>((winSize.x - 80) * usedRatio);
    myRetainedScrollView->addControl(
        std::make_unique<XenUI::Rectangle>(
            XenUI::PositionParams::Absolute(40, y + 70, usedWidth, 4),
            usedWidth,
            4,
            SDL_Color{50, 100, 200, 255}
        )
    );

    y += 100;

    // Dark card background for second row (model + processor)
    myRetainedScrollView->addControl(
        std::make_unique<XenUI::Rectangle>(
            XenUI::PositionParams::Absolute(20, y, winSize.x - 40, 80),
            winSize.x - 40,
            80,
            SDL_Color{30, 30, 30, 255}
        )
    );

    // Model
    myRetainedScrollView->addControl(
        std::make_unique<Label>(
            "Model",
            XenUI::PositionParams::Absolute(40, y + 10),
            20,
            SDL_Color{255, 255, 255, 255}
        )
    );

    myRetainedScrollView->addControl(
        std::make_unique<Label>(
            "RMX3286",
            XenUI::PositionParams::Absolute(40, y + 40),
            20,
            SDL_Color{200, 200, 200, 255}
        )
    );

    // Processor
    myRetainedScrollView->addControl(
        std::make_unique<Label>(
            "Processor",
            XenUI::PositionParams::Absolute(winSize.x / 2 + 20, y + 10),
            20,
            SDL_Color{255, 255, 255, 255}
        )
    );

    myRetainedScrollView->addControl(
        std::make_unique<Label>(
            "Helio G96",
            XenUI::PositionParams::Absolute(winSize.x / 2 + 20, y + 40),
            20,
            SDL_Color{200, 200, 200, 255}
        )
    );

    y += 100;

    // Dark card background for third row (battery + screen)
    myRetainedScrollView->addControl(
        std::make_unique<XenUI::Rectangle>(
            XenUI::PositionParams::Absolute(20, y, winSize.x - 40, 80),
            winSize.x - 40,
            80,
            SDL_Color{30, 30, 30, 255}
        )
    );

    // Battery capacity
    myRetainedScrollView->addControl(
        std::make_unique<Label>(
            "Battery capacity",
            XenUI::PositionParams::Absolute(40, y + 10),
            20,
            SDL_Color{255, 255, 255, 255}
        )
    );

    myRetainedScrollView->addControl(
        std::make_unique<Label>(
            "5000 mAh (TYP)",
            XenUI::PositionParams::Absolute(40, y + 40),
            20,
            SDL_Color{200, 200, 200, 255}
        )
    );

    // Screen size
    myRetainedScrollView->addControl(
        std::make_unique<Label>(
            "Screen size",
            XenUI::PositionParams::Absolute(winSize.x / 2 + 20, y + 10),
            20,
            SDL_Color{255, 255, 255, 255}
        )
    );

    myRetainedScrollView->addControl(
        std::make_unique<Label>(
            "6.6 in",
            XenUI::PositionParams::Absolute(winSize.x / 2 + 20, y + 40),
            20,
            SDL_Color{200, 200, 200, 255}
        )
    );

    y += 100;

    // Dark card background for RAM
    myRetainedScrollView->addControl(
        std::make_unique<XenUI::Rectangle>(
            XenUI::PositionParams::Absolute(20, y, winSize.x - 40, 80),
            winSize.x - 40,
            80,
            SDL_Color{30, 30, 30, 255}
        )
    );

    // RAM
    myRetainedScrollView->addControl(
        std::make_unique<Label>(
            "RAM",
            XenUI::PositionParams::Absolute(40, y + 10),
            20,
            SDL_Color{255, 255, 255, 255}
        )
    );

    myRetainedScrollView->addControl(
        std::make_unique<Label>(
            "4.00 GB + 4.00 GB",
            XenUI::PositionParams::Absolute(40, y + 40),
            20,
            SDL_Color{200, 200, 200, 255}
        )
    );

    y += 100;

    // Dark card background for cameras
    myRetainedScrollView->addControl(
        std::make_unique<XenUI::Rectangle>(
            XenUI::PositionParams::Absolute(20, y, winSize.x - 40, 100),
            winSize.x - 40,
            100,
            SDL_Color{30, 30, 30, 255}
        )
    );

    // Cameras
    myRetainedScrollView->addControl(
        std::make_unique<Label>(
            "Cameras",
            XenUI::PositionParams::Absolute(40, y + 10),
            20,
            SDL_Color{255, 255, 255, 255}
        )
    );

    myRetainedScrollView->addControl(
        std::make_unique<Label>(
            "Front 16MP",
            XenUI::PositionParams::Absolute(40, y + 40),
            20,
            SDL_Color{200, 200, 200, 255}
        )
    );

    myRetainedScrollView->addControl(
        std::make_unique<Label>(
            "Rear 50MP+2MP+2MP",
            XenUI::PositionParams::Absolute(40, y + 70),
            20,
            SDL_Color{200, 200, 200, 255}
        )
    );

    y += 120;

    // Dark card background for Android version
    myRetainedScrollView->addControl(
        std::make_unique<XenUI::Rectangle>(
            XenUI::PositionParams::Absolute(20, y, winSize.x - 40, 80),
            winSize.x - 40,
            80,
            SDL_Color{30, 30, 30, 255}
        )
    );

    // Android version
    myRetainedScrollView->addControl(
        std::make_unique<Label>(
            "Android version",
            XenUI::PositionParams::Absolute(40, y + 10),
            20,
            SDL_Color{255, 255, 255, 255}
        )
    );

    myRetainedScrollView->addControl(
        std::make_unique<Label>(
            "13",
            XenUI::PositionParams::Absolute(40, y + 40),
            20,
            SDL_Color{200, 200, 200, 255}
        )
    );

    // Recalculate layout
    myRetainedScrollView->recalculateLayout(winSize.x, winSize.y);
}




void setup(SDL_Window* window, SDL_Renderer* renderer) {
    XenUI::SetWindow(window); // Set window reference for GetWindowSize

    std::cout << "Initializing text renderer (framework finds font)...\n";
    if (!textRenderer.isInitialized()) {
        textRenderer.init(renderer);
    }
    if (!textRenderer.isInitialized()) {
        std::cerr << "CRITICAL ERROR: TEXT RENDERER FAILED TO INITIALIZE - NO FONT FOUND." << std::endl;
        exit(1);
    }

  
 
    //------------------
    setupAboutDevice(renderer);
    //------------------

    

    // Recalculate positions now that window exists
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    if (myRetainedScrollView) {
        SDL_Point winSize = XenUI::GetWindowSize();
myRetainedScrollView->recalculateLayout(winSize.x, winSize.y);// <-- ADD THIS
    }
    for (auto& lbl : labels)    lbl.recalculateLayout();
    for (auto& btn : buttons)   btn.recalculateLayout();
    for (auto& box : Inputs)    box.recalculatePosition();

        // --- ADDITIONS START ---
        if (channelGroup) {
            channelGroup->recalculateLayout(); // <--- Recalculate retained radio button positions
        }
        // --- ADDITIONS END ---
        for (auto& slider : sliders) { // <--- RECALCULATE RETAINED SLIDER POSITIONS
            slider.recalculateLayout();
        }
        for (auto& dropdown : dropdowns) { // <--- RECALCULATE RETAINED DROPDOWN POSITIONS
            dropdown.recalculateLayout();
        }
        

}

// *** MODIFIED render function ***
void render(SDL_Renderer* renderer, const SDL_Event& evt) {
    // Clear screen to dark gray
   SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    //Image Location
    float ImageX = 12;
    float ImageY = 5;

    if (gImage && gImage->isLoaded()) {
        // Example: draw at (50, 500)
        gImage->render(renderer, ImageX*100, ImageY*100, 0.3f, 0.3f); // Calls your Image::render

        // Example: draw another instance, scaled and at different location
        // gImage->render(renderer, 200, 100, nullptr, 0.0, nullptr, SDL_FLIP_NONE, 0.5f, 0.5f); // 50% scale
    } else {
        // Optional: Log if you expect it to be loaded but it's not
      //SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "gImage is not loaded or is null, not rendering.");
    }



        if (myRetainedScrollView) {
        // Pass {0, 0} as the viewOffset because it's a top-level element directly on the window.
        myRetainedScrollView->draw(renderer, {0.0f, 0.0f});
    }
    // 3) Retained-mode labels
    for (Label& label : labels) {
        label.draw(renderer);
    }

    // 4) Retained-mode buttons
    for (Button& button : buttons) {
        button.draw(renderer, {0.0f, 0.0f}); // Pass a zero offset for top-level elements
    }

    // 5) Retained-mode input boxes
    for (auto& box : Inputs) {
        box.draw(renderer);
    }

        // --- ADDITIONS START ---
    // 6) Retained-mode RadioButtonGroup
    if (channelGroup) {
        channelGroup->draw(renderer);
    }
    // --- ADDITIONS END ---
    // Retained‑mode checkboxes
    for (auto& cb : checkboxes) {
        cb.draw(renderer);
    }

    // Retained-mode switches
    for (auto& switchElem : switches) {
        switchElem.draw(renderer);
    }   

    // --- SLIDER ADDITIONS START ---
    // Retained-mode sliders
    for (auto& slider : sliders) {
        slider.draw(renderer);
    }
    // --- SLIDER ADDITIONS END ---
    // --- DROPDOWN ADDITIONS START ---
    // Retained-mode dropdowns
    for (auto& dropdown : dropdowns) {
        dropdown.draw(renderer, {0,0});
    }
    // --- DROPDOWN ADDITIONS END ---


    SDL_RenderPresent(renderer);
}



int main(int argc, char** argv) {
    // 1. Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    // 2. Create the Window
    // SDL3’s signature: SDL_CreateWindow(title, x, y, width, height, flags)
    SDL_Window* window = SDL_CreateWindow(
        "Xenon UI",                    // Window title
               // x position
               // y position
        800,                           // width
        600,                           // height
        SDL_WINDOW_RESIZABLE           // flags
    );
    if (!window) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // Set minimum window size
    const int MIN_WINDOW_WIDTH  = 400;
    const int MIN_WINDOW_HEIGHT = 200;
    SDL_SetWindowMinimumSize(window, MIN_WINDOW_WIDTH, MIN_WINDOW_HEIGHT);
    std::cout << "Minimum window size set to " << MIN_WINDOW_WIDTH << "x" << MIN_WINDOW_HEIGHT << std::endl;

    // 3. Create the Renderer
    // SDL3’s signature: SDL_CreateRenderer(window, driver_index, flags)
    SDL_Renderer* renderer = SDL_CreateRenderer(
        window,
        nullptr
    );
    if (!renderer) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // 4. Initialize TTF
    if (TTF_Init() == -1) {
        std::cerr << "SDL_ttf could not initialize! TTF_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }


    // 5. Call setup (loads fonts, images, widgets)
    setup(window, renderer);



/*Hybrid mode loop
     best for normal apps where sometimes fast render is require and sometimes remain idle
     */

bool running       = true;
SDL_Event event;

// Dirty‐flag to indicate we should redraw
bool needsRedraw   = true;
bool handled = false;

// Whether we’re currently in a short “game‐style” tick for animations
bool isAnimating   = false;

// Track time to update animations
auto lastEventTime = std::chrono::high_resolution_clock::now();
Uint64 lastCounter = SDL_GetPerformanceCounter();
double invFreq     = 1.0 / static_cast<double>(SDL_GetPerformanceFrequency());

// If no events and no animations for IDLE_THRESHOLD seconds, stay in pure idle
const double IDLE_THRESHOLD = 2.0; // seconds

while (running) {
    if (!isAnimating) {
        // 1A) Idle: wait for events (or wake up every 500 ms to check if we need to exit idle)
        if (SDL_WaitEventTimeout(&event, 500)) {
            // We got an event → process below
        } else {
            // Timeout occurred (500 ms passed w/o events)
            // Check how long since last real event
            auto nowClock = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> sinceLastEvent = nowClock - lastEventTime;
            if (sinceLastEvent.count() >= IDLE_THRESHOLD) {
                // Still idle—no need to update or redraw
                continue;
            }
            // If we get here, it means we’re within the IDLE_THRESHOLD window
            // after an event → fall through to updates/redraw
            event.type = 0; // no new event, but keep lastEventTime as is
        }
    } else {
        // 1B) Animation mode: poll events (do NOT block)
        if (SDL_PollEvent(&event) == 0) {
            event.type = 0; // no new event but we stay in animation
        }
    }
 gLastEvent = event;
    // 2) If event arrived, process it
    if (event.type != 0) {
        lastEventTime = std::chrono::high_resolution_clock::now();

        switch (event.type) {
            case SDL_EVENT_QUIT:
                running = false;
                break;

            case SDL_EVENT_KEY_DOWN:
                if (event.key.key == SDLK_F11) {
                    Uint32 flags = SDL_GetWindowFlags(window);
                    bool isFull = (flags & SDL_WINDOW_FULLSCREEN) != 0;
                    SDL_SetWindowFullscreen(
                        window,
                        isFull ? 0 : SDL_WINDOW_FULLSCREEN
                    );
                    needsRedraw = true;
                }
                break;

            case SDL_EVENT_WINDOW_RESIZED:
                for (auto& btn : buttons)   btn.recalculateLayout();
                for (auto& lbl : labels)    lbl.recalculateLayout();
                for (auto& box : Inputs)    box.recalculatePosition();
                for (auto& cb : checkboxes) cb.recalculateLayout();

                // --- ADDITIONS START ---
                if (channelGroup) {
                    channelGroup->recalculateLayout(); // <--- Recalculate positions on resize
                }
                // --- ADDITIONS END ---
                for (auto& switchElem : switches) {
                    switchElem.recalculateLayout();
                }

                for (auto& slider : sliders) { // <--- RECALCULATE RETAINED SLIDER POSITIONS ON RESIZE
                    slider.recalculateLayout();
                }
                for (auto& dropdown : dropdowns) { // <--- RECALCULATE RETAINED DROPDOWN POSITIONS ON RESIZE
                    dropdown.recalculateLayout();
                }
                if (myRetainedScrollView) {
                     SDL_Point winSize = XenUI::GetWindowSize();
myRetainedScrollView->recalculateLayout(winSize.x, winSize.y);
                }



                needsRedraw = true;
                break;

            default:

if (myRetainedScrollView && myRetainedScrollView->handleEvent(event, window, {0.0f, 0.0f})) {
    needsRedraw = true;
    // handled = true;
    break;
}
// if (!handled) {
//     for (auto &cb : checkboxes) {
//         if (cb.handleEvent(event)) { handled = true; break; }
//     }
// }



    // Otherwise, pass to your other retained‐mode widgets:
    for (auto& btn : buttons) {
        if (btn.handleEvent(event)) {
            needsRedraw = true;
            isAnimating = true;
        }
    }
    if (channelGroup && channelGroup->handleEvent(event)) {
        needsRedraw = true;
    }
    for (auto& cb : checkboxes) {
        if (cb.handleEvent(event)) {
            needsRedraw = true;
            std::cout << "Checkbox is now " << (cb.isChecked() ? "ON\n" : "OFF\n");
        }
    }
    for (auto& sw : switches) {
        if (sw.handleEvent(event)) {
            needsRedraw = true;
        }
    }
    for (auto& slider : sliders) {
        if (slider.handleEvent(event)) {
            needsRedraw = true;
        }
    }
    for (auto& dd : dropdowns) {
        if (dd.handleEvent(event)) {
            needsRedraw = true;
        }
    }
    break;

        }
        // Pass event to input boxes
        for (auto& box : Inputs) {
            if (box.handleEvent(event, window, {0,0})) {
                needsRedraw = true;
                // If user typed or cursor moved, maybe start a short caret blink animation:
                isAnimating = true;
            }
        }

        // if (event.type == SDL_EVENT_TEXT_EDITING) {
        //     SDL_Log(
        //         "App: SDL_EVENT_TEXT_EDITING --- text: '%s', start: %d, length: %d",
        //         event.edit.text ? event.edit.text : "NULL",
        //         event.edit.start,
        //         event.edit.length
        //     );
        // }

        // Ensure we’ll redraw at least once after processing the event
        needsRedraw = true;
    }

    // 3) If in animation mode, update animations each frame
    if (isAnimating) {
        Uint64 nowCounter = SDL_GetPerformanceCounter();
        double deltaSec = (nowCounter - lastCounter) * invFreq;
        lastCounter = nowCounter;

        // Update input boxes (caret blink, text cursor animation, etc.)
        bool anyStillAnimating = false;
        for (auto& box : Inputs) {
            if (box.update(static_cast<float>(deltaSec))) {
                needsRedraw = true;
            }
            if (box.isAnimating()) {
                anyStillAnimating = true;
            }
        }

        // (You can update any other short animations here, e.g. button press fade, 
        //  and set anyStillAnimating = true if they are still running.)

        if (!anyStillAnimating) {
            // All animations finished → exit animation mode
            isAnimating = false;
        }
    }

    // 4) Only redraw when needed
    if (needsRedraw) {
        render(renderer, gLastEvent);
        needsRedraw = false;
    }
    // If still animating (isAnimating == true), we’ll loop again immediately,
    // calling SDL_PollEvent next iteration until animations finish.
    // Once isAnimating turns false and no new events arrive for IDLE_THRESHOLD,
    // we’ll return to idle and sleep in SDL_WaitEventTimeout.
}

// Cleanup (unchanged)
textRenderer.clearCache();
if (gImage) { delete gImage; gImage = nullptr; }
// --- ADDITIONS START ---
// if (channelGroup) {
//     delete channelGroup; // <--- Clean up retained radio button group
//     channelGroup = nullptr;
// }
// --- ADDITIONS END ---
if (myRetainedScrollView) {
    myRetainedScrollView.reset();
}
TTF_Quit();
SDL_DestroyRenderer(renderer);
SDL_DestroyWindow(window);
SDL_Quit();


}






