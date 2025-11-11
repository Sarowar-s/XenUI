
// SPDX-License-Identifier: Apache-2.0
/*
 *  Copyright (C) 2025 MD S M Sarowar Hossain
 *
 * 
 */
// main.cpp
// dont remove first two comments
#include <SDL3/SDL.h>
#include <SDL3/SDL_log.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>     // <--- SDL3_image no longer needs IMG_Init/IMG_Quit
#include<SDL3/SDL_main.h>
#include <iostream>
#include <vector>
#include <cstring>  // For memcpy
#include <string>
#include <chrono>
#include <memory>
#include "Image.h"         // <-- Image wrapper (SDL3)
#include "TextRenderer.h"
#include "Label.h"
#include "InputBox.h"
#include "Shape.h"
#include "Button.h"
#include "WindowUtil.h"
#include "Anchor.h"
#include "Position.h"
#include "RadioButton.h"
#include "CheckBox.h"
#include "Switch.h"
#include "Slider.h"
#include "Dropdown.h"
#include "ScrollView.h"
#include "UIElement.h"


// near the top of main.cpp
static SDL_Event gLastEvent;


// Global text renderer instance
TextRenderer& textRenderer = TextRenderer::getInstance();

// These vectors can be used for UI elements outside a ScrollView.
// In this design, all elements are inside the ScrollView, so these will be empty.
std::vector<Label> labels;
std::vector<Button> buttons;
std::vector<XenUI::InputBox> Inputs;
std::vector<Checkbox> checkboxes;
std::vector<XenUI::Switch> switches;
std::vector<Slider> sliders;
std::vector<Dropdown> dropdowns;

// The main container for our UI
std::unique_ptr<ScrollView> myRetainedScrollView;
// Radio button group must persist
std::unique_ptr<RadioButtonGroup> colorModeGroup;

// --- STATE VARIABLES FOR UI CONTROLS ---

// Sliders
float retainedBrightness = 0.8f;
float retainedFontSize = 0.4f;

// Switch
bool adaptiveBrightnessEnabled = true;

// Dropdowns
std::vector<std::string> themes = {"Dark", "Light", "System Default"};
int selectedThemeIndex = 0;

// Radio Buttons
int selectedColorMode = 0; // 0: Vivid, 1: Natural, 2: Pro Mode

// --- END STATE VARIABLES ---





void setupDisplaySettings(SDL_Renderer* renderer) {
    // Force a big scale for mobile devices (change this to 1.0f for desktop)
    const float MOBILE_UI_SCALE = 2.0f; // <-- tweak this: 2.0-3.5 is usually good for phones
    const float uiScale = MOBILE_UI_SCALE;

    auto S = [uiScale](float v) -> int { return static_cast<int>(v * uiScale + 0.5f); };
    auto SF = [uiScale](float v) -> float { return v * uiScale; };

    SDL_Point winSize = XenUI::GetWindowSize();

    myRetainedScrollView = std::make_unique<ScrollView>(
        XenUI::PositionParams::Anchored(XenUI::Anchor::TOP_LEFT,0,0,winSize.x, winSize.y)
    );

    // Radio button group
    colorModeGroup = std::make_unique<RadioButtonGroup>(&selectedColorMode);

    int y = S(24); // bigger top margin

    // Main Title (much larger)
    myRetainedScrollView->addControl(
        std::make_unique<Label>(
            "Display & Brightness",
            XenUI::PositionParams::Anchored(XenUI::Anchor::TOP_CENTER, 0, y),
            S(44), // big font
            SDL_Color{255, 255, 255, 255}
        )
    );
    y += S(80);

    // --- Brightness Card ---
    int cardX = S(24);
    int cardWidth = winSize.x - S(48);
    myRetainedScrollView->addControl(
        std::make_unique<XenUI::Rectangle>(
            XenUI::PositionParams::Absolute(cardX, y, cardWidth, S(220)),
            cardWidth, S(220), SDL_Color{30, 30, 30, 255}
        )
    );

    myRetainedScrollView->addControl(
        std::make_unique<Label>(
            "Brightness",
            XenUI::PositionParams::Absolute(cardX + S(22), y + S(18)),
            S(30)
        )
    );

    // Brightness slider (longer and taller)
    myRetainedScrollView->addControl(
        std::make_unique<Slider>(
            "brightnessSlider",
            XenUI::Orientation::HORIZONTAL,
            XenUI::PositionParams::Anchored(XenUI::Anchor::TOP_CENTER, 0, y + S(75), cardWidth - S(60), S(30)),
            static_cast<float>(cardWidth - S(50)), // length
            retainedBrightness,                    // initial
            1.0f,
            100.0f,
            SliderStyle{},
            [](float val) { retainedBrightness = val; }
        )
    );

    myRetainedScrollView->addControl(
        std::make_unique<Label>(
            "Adaptive brightness",
            XenUI::PositionParams::Absolute(cardX + S(22), y + S(140)),
            S(26)
        )
    );

    // Switch style scaled up
    XenUI::SwitchStyle switchStyle;
    switchStyle.trackHeight    = SF(40.0f);
    switchStyle.trackWidth     = SF(85.0f);
    switchStyle.thumbPadding   = SF(6.0f);
    switchStyle.labelFontSize  = S(18);
    switchStyle.labelOff       = "Off";
    switchStyle.labelOn        = "On";
    switchStyle.labelColor     = SDL_Color{20,20,20,255};

    myRetainedScrollView->addControl(
        std::make_unique<XenUI::Switch>(
            XenUI::PositionParams::Anchored(XenUI::Anchor::TOP_RIGHT, -S(40), y + S(170)),
            switchStyle,
            std::function<void(bool)>([](bool val){ adaptiveBrightnessEnabled = val; }),
            adaptiveBrightnessEnabled
        )
    );

    y += S(260); // card height + spacing

    // --- Color Mode Card ---
    myRetainedScrollView->addControl(
        std::make_unique<XenUI::Rectangle>(
            XenUI::PositionParams::Absolute(cardX, y, cardWidth, S(240)),
            cardWidth, S(240), SDL_Color{30, 30, 30, 255}
        )
    );

    myRetainedScrollView->addControl(
        std::make_unique<Label>(
            "Screen color mode",
            XenUI::PositionParams::Absolute(cardX + S(22), y + S(18)),
            S(30)
        )
    );

    // Radio buttons with large font & spacing
    myRetainedScrollView->addControl(
        std::make_unique<RadioButton>(
            *colorModeGroup, "Vivid", 0,
            XenUI::PositionParams::Absolute(cardX + S(22), y + S(80)),
            RadioButtonStyle{}, S(28)
        )
    );
    myRetainedScrollView->addControl(
        std::make_unique<RadioButton>(
            *colorModeGroup, "Natural", 1,
            XenUI::PositionParams::Absolute(cardX + S(22), y + S(130)),
            RadioButtonStyle{}, S(28)
        )
    );
    myRetainedScrollView->addControl(
        std::make_unique<RadioButton>(
            *colorModeGroup, "Pro Mode", 2,
            XenUI::PositionParams::Absolute(cardX + S(22), y + S(180)),
            RadioButtonStyle{}, S(28)
        )
    );

    y += S(280);

    // --- Theme & Font Card ---
    myRetainedScrollView->addControl(
        std::make_unique<XenUI::Rectangle>(
            XenUI::PositionParams::Absolute(cardX, y, cardWidth, S(200)),
            cardWidth, S(200), SDL_Color{30, 30, 30, 255}
        )
    );

    myRetainedScrollView->addControl(
        std::make_unique<Label>(
            "Theme",
            XenUI::PositionParams::Absolute(cardX + S(22), y + S(22)),
            S(26)
        )
    );

    myRetainedScrollView->addControl(
        std::make_unique<Dropdown>(
            "themeDropdown",
            XenUI::PositionParams::Anchored(XenUI::Anchor::TOP_RIGHT, -S(40), y + S(18), S(50), S(42)),
            static_cast<float>(S(150)), themes, selectedThemeIndex, DropdownStyle{},
            [](int idx){ selectedThemeIndex = idx; }
        )
    );

    myRetainedScrollView->addControl(
        std::make_unique<Label>(
            "Font size",
            XenUI::PositionParams::Absolute(cardX + S(22), y + S(94)),
            S(26)
        )
    );

    // Font slider scaled
    myRetainedScrollView->addControl(
        std::make_unique<Slider>(
            "fontSlider",
            XenUI::Orientation::HORIZONTAL,
            XenUI::PositionParams::Anchored(XenUI::Anchor::TOP_CENTER, 0, y + S(140), cardWidth - S(60), S(30)),
            static_cast<float>(cardWidth - S(60)),
            retainedFontSize,
            1.0f,
            100.0f,
            SliderStyle{},
            [](float val) { retainedFontSize = val; }
        )
    );

    y += S(240);

     // --- Color Mode Card ---
    myRetainedScrollView->addControl(
        std::make_unique<XenUI::Rectangle>(
            XenUI::PositionParams::Absolute(cardX, y, cardWidth, S(240)),
            cardWidth, S(240), SDL_Color{30, 30, 30, 255}
        )
    );

    myRetainedScrollView->addControl(
        std::make_unique<Label>(
            "Checker",
            XenUI::PositionParams::Absolute(cardX + S(22), y + S(18)),
            S(30)
        )
    );

    // Radio buttons with large font & spacing
    myRetainedScrollView->addControl(
        std::make_unique<Checkbox>(
            "My checkbox 1",
            XenUI::PositionParams::Absolute(cardX + S(22), y + S(80)),
            false,
            CheckboxStyle{},
            S(28)
        )
    );
    myRetainedScrollView->addControl(
        std::make_unique<Checkbox>(
            "My checkbox 2",
             XenUI::PositionParams::Absolute(cardX + S(22), y + S(130)),
            false,
            CheckboxStyle{},
            S(28)
        )
    );
    myRetainedScrollView->addControl(
        std::make_unique<Checkbox>(
            "My checkbox 3",
             XenUI::PositionParams::Absolute(cardX + S(22), y + S(180)),
            false,
            CheckboxStyle{},
            S(28)
        )
    );
    
  
    

    y+=480;

    ButtonStyle bstyle;
    bstyle.bgColor = {100,200,100,255};
    bstyle.paddingY = 40;
    bstyle.paddingX = 10;
    // --- Apply Button (much larger) ---
    myRetainedScrollView->addControl(
        std::make_unique<Button>(
            "Apply Changes",
            XenUI::PositionParams::Anchored(XenUI::Anchor::TOP_CENTER, 0, y + S(28), S(300), S(130)),
            bstyle,
            []() { std::cerr << "Apply Changes button clicked!\n"; },
            65
        )
    );

    // Final layout calculation
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
    // Setup the main UI for the application
    setupDisplaySettings(renderer);
    //------------------

    // Recalculate positions now that window exists
    if (myRetainedScrollView) {
        SDL_Point winSize = XenUI::GetWindowSize();
        myRetainedScrollView->recalculateLayout(winSize.x, winSize.y);
    }
    // Recalculate any elements outside the scroll view (if any)
    for (auto& lbl : labels)   lbl.recalculateLayout();
    for (auto& btn : buttons)  btn.recalculateLayout();
    for (auto& box : Inputs)   box.recalculatePosition();
    for (auto& slider : sliders) slider.recalculateLayout();
    for (auto& dropdown : dropdowns) dropdown.recalculateLayout();
}

void render(SDL_Renderer* renderer, const SDL_Event& evt) {
    // Clear screen to dark gray
    SDL_SetRenderDrawColor(renderer, 18, 18, 18, 255);
    SDL_RenderClear(renderer);
    
    // The ScrollView handles drawing all of its children (buttons, sliders, etc.)
    if (myRetainedScrollView) {
        myRetainedScrollView->draw(renderer, {0.0f, 0.0f});
    }

    // Draw any UI elements that are NOT inside the scroll view.
    // In this design, there are none, but the code is here if you add them.
    for (Label& label : labels) label.draw(renderer);
    for (Button& button : buttons) button.draw(renderer, {0.0f, 0.0f});
    for (auto& box : Inputs) box.draw(renderer);
    for (auto& cb : checkboxes) cb.draw(renderer);
    for (auto& switchElem : switches) switchElem.draw(renderer);
    for (auto& slider : sliders) slider.draw(renderer);
    for (auto& dropdown : dropdowns) dropdown.draw(renderer, {0,0});


    SDL_RenderPresent(renderer);
}



extern "C" int SDL_main(int argc, char** argv) {
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
                if (colorModeGroup) {
                    SDL_Point winSize = XenUI::GetWindowSize();
                    colorModeGroup->recalculateLayout(winSize.x, winSize.y); // <--- Recalculate positions on resize
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
    if (colorModeGroup && colorModeGroup->handleEvent(event)) {
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
// if (gImage) { delete gImage; gImage = nullptr; }
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






















// /*
//  *  Copyright (c) 2025 XenonUI
//  *  Author: MD S M Sarowar Hossain
//  *
//  * 
//  * This is the main file that shows how to use ui elements in a project.
//  * It has Retained mode ui elements only.
//  * 
//  */


// #include <SDL3/SDL.h>
// #include <SDL3/SDL_log.h>
// #include <SDL3_ttf/SDL_ttf.h>
// #include <SDL3_image/SDL_image.h>    
// #include<SDL3/SDL_main.h>
// #include <iostream>
// #include <vector>
// #include <cstring>  // For memcpy
// #include <string>
// #include <chrono>
// #include "Image.h"        
// #include "TextRenderer.h"
// #include "Label.h"
// #include "InputBox.h"
// #include "Shape.h"
// #include "Button.h"
// #include "WindowUtil.h"
// #include "Anchor.h"
// #include "Position.h" 
// #include "RadioButton.h"
// #include "CheckBox.h"
// #include "Switch.h"  
// #include "Slider.h"  
// #include "Dropdown.h" 

// #include "ScrollView.h"
// #include "UIElement.h"


// static SDL_Event gLastEvent;


// // Global text renderer instance
// TextRenderer& textRenderer = TextRenderer::getInstance();
// std::vector<Label> labels;
// std::vector<Button> buttons;                // Store buttons in a vector
// std::vector<XenUI::InputBox> Inputs;        // Store InputBoxes in a vector
// std::vector<XenUI::Rectangle> shapes;       // Example retained-mode shapes
// std::vector<Checkbox> checkboxes;
// std::vector<std::unique_ptr<ImageControl>> Images;
// std::vector<XenUI::Switch> switches;  // For retained mode switches

// std::unique_ptr<ScrollView> myRetainedScrollView;



// std::vector<Slider> sliders; // Store retained mode sliders
// float retainedVolume = 0.5f; // Value for a retained mode slider
// float retainedBrightness = 0.8f; 

// // Immediate Mode Slider Variable
// float immediateAudioLevel = 0.75f; // Value for an immediate mode slider
// float Level = 0.5f;
// float Level1 = 0.5f;
// float Level3 = 0.5f;
// float Level4 = 0.5f;

// std::vector<Dropdown> dropdowns; // Store retained mode dropdown objects
// std::vector<std::string> themes = {"Dark", "Light", "High Contrast", "Custom"};
// std::vector<std::string> languages = {"English", "Spanish", "French", "German", "Japanese"};
// int selectedThemeIndex = 0; // Value for a retained mode dropdown
// int selectedLanguageIndex = 0; // Another retained mode dropdown value


// int numItems = 15;

// bool toggled1 = false;   
// bool toggled2 = false;   
// bool toggled3 = false;   
// bool retainedChecked = false;     
// bool retainedChecked1 = false;     
// bool retainedChecked2 = false;    
// bool retainedChecked3 = false;    

// // Let's say you have N buttons in the scrollview
// int contentStartY = 10;
// int rowGap        = 50;

// // Position for the first checkbox (right after all buttons)
// float checkboxBaseY = contentStartY + (numItems * rowGap) + 10;

// // Global Image pointer
// Image* gImage = nullptr;

// // Test variables
// int fps = 60;
// int FPS = 60;


// int selectedChannel = -1;
// std::unique_ptr<RadioButtonGroup> channelGroup = nullptr; 






// void setupLabels() {
//     std::string fpsText = "Fps : " + std::to_string(fps);
    
//     labels.emplace_back(
//         "Xenon UI",
//         XenUI::PositionParams::Anchored(XenUI::Anchor::TOP_CENTER),
//         60, // textSize
//         SDL_Color{255, 255, 255, 255}
//     );

//     labels.emplace_back(
//         "Developed by MD S M Sarowar Hossain",
//         XenUI::PositionParams::Anchored(XenUI::Anchor::TOP_CENTER, 0, 60),
//         40, // textSize
//         SDL_Color{255, 255, 255, 255}
//     );
//     labels.emplace_back(
//         "Version 0.9.0",
//         XenUI::PositionParams::Anchored(XenUI::Anchor::TOP_CENTER, 0, 100),
//         30, // textSize
//         SDL_Color{255, 255, 255, 200}
//     );
// }

// void setupButtons() {
//     ButtonStyle style1;
//     style1.textColor = {255, 255, 255, 255};
//     style1.bgColor = {10, 200, 100, 255};

//     buttons.emplace_back(
//         "Button 1",                                           // text
//         XenUI::PositionParams::Anchored(XenUI::Anchor::TOP_LEFT, 50, 200),             // Position
//         style1,                                                                        // style
//         []() {
//             fps++;
            
//             std::cout << "Button 1 Clicked! fps:"<<fps << std::endl;
//         },
//         35 // fontSize
//     );

//     buttons.emplace_back(
//         "Button 2",                                                                    // text
//         XenUI::PositionParams::Anchored(XenUI::Anchor::TOP_LEFT, 250, 200),              // Position
//         style1,                                                                        // style
//         []() {
//             FPS++;
            
//             std::cout << "Button 1 Clicked! fps:" <<FPS<< std::endl;
//         },
//         35 // fontSize
//     );

//     ButtonStyle style2;
//     style2.textColor = {255, 255, 255, 255};
//     style2.bgColor = {50, 50, 50, 255};

//     buttons.emplace_back(
//         "Second Button",                                                               // text
//         XenUI::PositionParams::Anchored(XenUI::Anchor::BOTTOM_RIGHT, -50, -50),        // Position
//         style2,                                                                        // style
//         []() { std::cout << "Button 2 Clicked!" << std::endl; },
//         30
//         // fontSize defaults to 16 if not specified
//     );
// }

// void myinput() {
//     XenUI::InputBoxStyle customStyle;
//     customStyle.bgColor = {30, 70, 90, 255};    // Example: Opaque dark blue
//     customStyle.textColor = {220, 220, 220, 255};
//     customStyle.drawBackground = true;
//     customStyle.drawBorder     = true;
//     customStyle.paddingX       = 8;
//     customStyle.paddingY       = 4;
//     // ... set other style properties ...

//     Inputs.emplace_back(
//         XenUI::PositionParams::Anchored(XenUI::Anchor::TOP_LEFT, 50, 280),
//         "Edit Me!",
//         300,         // width
//         40,          // fontSize
//         customStyle, // PASS THE STYLE OBJECT
//         false        // isPassword
//     );
// }


// void setupRadioButtons() {

//    channelGroup = std::make_unique<RadioButtonGroup>(&selectedChannel, [](int newValue) {
//         std::cout << "Retained Radio: Selection changed to: " << newValue << std::endl;

//     });


//     RadioButtonStyle retainedRadioStyle;
//     retainedRadioStyle.circleColor = {150, 150, 255, 255}; // Light Blue
//     retainedRadioStyle.selectedColor = {50, 50, 200, 255}; // Dark Blue
//     retainedRadioStyle.labelColor = {255, 255, 255, 255};
//     retainedRadioStyle.circleRadius = 18;
//     retainedRadioStyle.circlePadding = 15;
//     retainedRadioStyle.innerCirclePadding = 8;


//     channelGroup->addButton(
//         "Option A",
//         0, // Value
//         XenUI::PositionParams::Anchored(XenUI::Anchor::TOP_LEFT, 50, 380), // Position of first button
//         retainedRadioStyle, // Pass the style
//         35 // Font size
//     );

//     channelGroup->addButton(
//         "Option B",
//         1, // Value
//         XenUI::PositionParams::Anchored(XenUI::Anchor::TOP_LEFT, 50, 430), // Position of second button
//         retainedRadioStyle, // Pass the style
//         35 // Font size
//     );

//     channelGroup->addButton(
//         "Option C",
//         2, // Value
//         XenUI::PositionParams::Anchored(XenUI::Anchor::TOP_LEFT, 50, 480), // Position of third button
//         retainedRadioStyle, // Pass the style
//         35 // Font size
//     );
// }





// void setupCheckbox(){

// checkboxes.emplace_back(
//     "Feature X",
//     XenUI::PositionParams::Anchored(XenUI::Anchor::CENTER_RIGHT, -150),
//     /*initialState=*/retainedChecked,
//     CheckboxStyle{},
//     30
//   );
// checkboxes.emplace_back(
//     "Feature Y",
//     XenUI::PositionParams::Anchored(XenUI::Anchor::CENTER_RIGHT, -150, 50),
//     /*initialState=*/retainedChecked2,
//     CheckboxStyle{},
//     30,
//     [](bool isChecked){
//         std::cout<<"Retained CheckBox w is now "<< (isChecked ? "On\n" : "Off\n");
//     }
//   );
// }


// void setupSwitches() {
//     XenUI::SwitchStyle switchStyle;
//     switchStyle.trackColorOff = {180, 180, 180, 255};   // Light grey when off
//     switchStyle.trackColorOn = {100, 200, 100, 255};    // Green when on
//     switchStyle.thumbColorOff = {255, 255, 255, 255};   // White thumb when off
//     switchStyle.thumbColorOn = {255, 255, 255, 255};    // White thumb when on
//     switchStyle.hoverTrackColor = {200, 200, 200, 255}; // Slightly darker grey on hover
//     switchStyle.hoverThumbColor = {240, 240, 240, 255}; // Slightly darker white on hover
//     switchStyle.trackHeight = 35.0f;
//     switchStyle.trackWidth = 65.0f;
//     switchStyle.thumbPadding = 5.0f;

//     switches.emplace_back(

//         XenUI::PositionParams::Anchored(XenUI::Anchor::TOP_LEFT, 50, 610),  // Position at (100, 300)
//         switchStyle,
//         [](bool isOn) {  // Callback for state changes
//             std::cout << "Retained Switch is now " << (isOn ? "ON" : "OFF") << std::endl;
//         },
//         false  // Initial state: OFF
//     );
// }




// void setupSliders() {

//     SliderStyle volumeSliderStyle;
//     volumeSliderStyle.trackColor = {80, 80, 80, 255};
//     volumeSliderStyle.thumbColor = {0, 150, 255, 255}; // Blue thumb
//     volumeSliderStyle.thumbHoverColor = {50, 180, 255, 255};
//     volumeSliderStyle.trackThickness = 15;
//     volumeSliderStyle.thumbSize = 30;
//     volumeSliderStyle.valueTextFontSize = 18;

//     sliders.emplace_back(
//         "retainedVolumeSlider", // Unique ID
//         XenUI::Orientation::HORIZONTAL,
//         XenUI::PositionParams::Anchored(XenUI::Anchor::TOP_LEFT, 450, 340), // Position
//         150.0f, // Length: 250 pixels wide
//         retainedVolume, 0.0f, 1.0f, // Value range 0.0 to 1.0
//         volumeSliderStyle,
//         [&](float newValue) { // Lambda callback for value changes
//             retainedVolume = newValue;
//             std::cout << "Retained Volume: " << retainedVolume << std::endl;
//         }
//     );

//     SliderStyle brightnessSliderStyle;
//     brightnessSliderStyle.trackColor = {60, 60, 60, 255};
//     brightnessSliderStyle.thumbColor = {200, 200, 50, 255}; // Yellow thumb
//     brightnessSliderStyle.thumbHoverColor = {220, 220, 70, 255};
//     brightnessSliderStyle.trackThickness = 15;
//     brightnessSliderStyle.thumbSize = 30;
//     brightnessSliderStyle.valueTextFontSize = 18;

//     sliders.emplace_back(
//         "retainedBrightnessSlider", // Unique ID
//         XenUI::Orientation::VERTICAL,
//         XenUI::PositionParams::Anchored(XenUI::Anchor::TOP_LEFT, 450, 400), 
//         150.0f, // Length: 180 pixels tall
//         retainedBrightness, 0.0f, 1.0f, // Value range 0.0 to 1.0
//         brightnessSliderStyle,
//         [&](float newValue) { // Lambda callback for value changes
//             retainedBrightness = newValue;
//             std::cout << "Retained Brightness: " << retainedBrightness << std::endl;
//         }
//     );
// }

// void setupDropdowns() {

    
//     DropdownStyle themeDropdownStyle;
//     themeDropdownStyle.mainButtonBgColor = {50, 50, 70, 255};
//     themeDropdownStyle.mainButtonTextColor = {255, 255, 255, 255};
//     themeDropdownStyle.listBgColor = {40, 40, 60, 255};
//     themeDropdownStyle.listItemHoverBgColor = {70, 70, 90, 255};
//     themeDropdownStyle.mainButtonFontSize = 28;
//     themeDropdownStyle.listItemFontSize = 25;
//     themeDropdownStyle.paddingX = 15;
//     themeDropdownStyle.paddingY = 10;

//     dropdowns.emplace_back(
//         "themeSelector", // Unique ID
//         XenUI::PositionParams::Anchored(XenUI::Anchor::CENTER_LEFT, 50),
//         250.0f, // Width of the dropdown
//         themes,
//         selectedThemeIndex, // Initial selection
//         themeDropdownStyle,
//         [&](int newIndex) { 
//             selectedThemeIndex = newIndex;
//             std::cout << "Selected Theme: " << themes[selectedThemeIndex] << std::endl;
          
//         }
//     );

// }


// void setupImage(SDL_Renderer* renderer){
//    Images.emplace_back(std::make_unique<ImageControl>(
//         renderer,
//         "Images/Image.png",
//         XenUI::PositionParams::Anchored(XenUI::Anchor::CENTER_RIGHT, -200, 50),
//         120.0f,
//         100.0f
//     ));
// }



// void setupScrollView(SDL_Renderer* renderer) {

//     myRetainedScrollView = std::make_unique<ScrollView>(
        

//         XenUI::PositionParams::Anchored(XenUI::Anchor::BOTTOM_CENTER, 0, -200, 500, 500)
//     );

//     // Button style
//     ButtonStyle scrollButtonStyle;
//     scrollButtonStyle.bgColor = {20, 120, 200, 255};
//     scrollButtonStyle.hoverColor = {40, 140, 220, 255};
//     scrollButtonStyle.pressedColor = {0, 100, 180, 255};
//     scrollButtonStyle.paddingX = 15;
//     scrollButtonStyle.paddingY = 8;

//     // Content layout constants
//     const int contentPaddingLeft = 10;
//     const int contentStartY = 10;
//     const int rowGap = 50;


//         myRetainedScrollView->addControl(
//             std::make_unique<Button>(
//                 "Click me",
//                 XenUI::PositionParams::Anchored(XenUI::Anchor::TOP_LEFT, 5,5),
//                 scrollButtonStyle,
//                 []() { std::cout << "Clicked scroll button " << std::endl; },
//                 20
//             )
//         );


//     myRetainedScrollView->addControl(
//         std::make_unique<ImageControl>(
//             renderer,
//             "Images/Image.png", // The path to the image file
//             XenUI::PositionParams::Anchored(XenUI::Anchor::BOTTOM_CENTER),
//             120.0f,  // Desired width (e.g., 120 pixels)
//             80.0f     // Desired height, 0.0f will preserve aspect ratio
//         )
//     );
    

// static int selectedChannel = 0; 


// auto channelGroup = std::make_unique<RadioButtonGroup>(&selectedChannel,
//     [](int newValue) {
//         std::cout << "Retained Radio: Selection changed to: " << newValue << std::endl;
        
//     }
// );

// // Define a custom style
// RadioButtonStyle retainedRadioStyle;
// retainedRadioStyle.circleColor = {150, 150, 255, 255}; // Light Blue
// retainedRadioStyle.selectedColor = {50, 50, 200, 255}; // Dark Blue
// retainedRadioStyle.labelColor = {255, 255, 255, 255};
// retainedRadioStyle.circleRadius = 12;
// retainedRadioStyle.circlePadding = 10;
// retainedRadioStyle.innerCirclePadding = 5;


// channelGroup->addButton(
//     "Option A",
//     0,
//     XenUI::PositionParams::Absolute(contentPaddingLeft, 450),
//     retainedRadioStyle,
//     25
// );

// channelGroup->addButton(
//     "Option B",
//     1,
//     XenUI::PositionParams::Absolute(contentPaddingLeft, 490),
//     retainedRadioStyle,
//     25
// );

// channelGroup->addButton(
//     "Option C",
//     2,
//     XenUI::PositionParams::Absolute(contentPaddingLeft , 530),
//     retainedRadioStyle,
//     25
// );


// SDL_Point winSize = XenUI::GetWindowSize();
// channelGroup->recalculateLayout(winSize.x, winSize.y);


// myRetainedScrollView->addControl(std::move(channelGroup));




// //draw a rectangle
// myRetainedScrollView->addControl(
//     std::make_unique<XenUI::Rectangle>(
//         XenUI::PositionParams::Anchored(XenUI::Anchor::CENTER),
//         200, 100,
//         SDL_Color{0, 255, 0, 255}
//     )
// );

// //draw a circle
// myRetainedScrollView->addControl(
//     std::make_unique<XenUI::Circle>(
//         XenUI::PositionParams::Anchored(XenUI::Anchor::CENTER, 5),
//         45,
//         SDL_Color{200, 5, 0, 255}
//     )
// );



//  DropdownStyle themeDropdownStyle;
//     themeDropdownStyle.mainButtonBgColor = {50, 50, 70, 255};
//     themeDropdownStyle.mainButtonTextColor = {255, 255, 255, 255};
//     themeDropdownStyle.listBgColor = {40, 40, 60, 255};
//     themeDropdownStyle.listItemHoverBgColor = {70, 70, 90, 255};
//     themeDropdownStyle.mainButtonFontSize = 20;
//     themeDropdownStyle.listItemFontSize = 18;
//     themeDropdownStyle.paddingX = 15;
//     themeDropdownStyle.paddingY = 10;
//     myRetainedScrollView->addControl(
//         std::make_unique<Dropdown>(
//             "themeSelector", // Unique ID
//             XenUI::PositionParams::Anchored(XenUI::Anchor::TOP_RIGHT), // Position
//             200.0f, // Width of the dropdown
//             themes,
//             selectedThemeIndex, // Initial selection
//             themeDropdownStyle,
//             [&](int newIndex) { // Callback for selection changes
//                 selectedThemeIndex = newIndex;
//                 std::cout << "Selected Theme: " << themes[selectedThemeIndex] << std::endl;
               
//             }
//         )
//     );


//     SliderStyle volumeSliderStyle1;
//     volumeSliderStyle1.trackColor = {80, 80, 80, 255};
//     volumeSliderStyle1.thumbColor = {0, 150, 255, 255}; // Blue thumb
//     volumeSliderStyle1.thumbHoverColor = {50, 180, 255, 255};
//     volumeSliderStyle1.trackThickness = 10;
//     volumeSliderStyle1.thumbSize = 25;
//     volumeSliderStyle1.valueTextFontSize = 16;

//     myRetainedScrollView->addControl(



//     std::make_unique<Slider>(
//         "retainedVolumeSlider", // Unique ID
//         XenUI::Orientation::HORIZONTAL,
//         XenUI::PositionParams::Absolute(50, 400), // Position
//         250.0f, // Length: 250 pixels wide
//         retainedVolume, 0.0f, 1.0f, // Value range 0.0 to 1.0
//         volumeSliderStyle1,
//         [&](float newValue) { // Lambda callback for value changes
//             retainedVolume = newValue;
//             std::cout << "Retained Volume: " << retainedVolume << std::endl;
//         }
//     )
// );





// XenUI::SwitchStyle switchStyle1;
//     switchStyle1.trackColorOff = {180, 180, 180, 255};   // Light grey when off
//     switchStyle1.trackColorOn = {100, 200, 100, 255};    // Green when on
//     switchStyle1.thumbColorOff = {255, 255, 255, 255};   // White thumb when off
//     switchStyle1.thumbColorOn = {255, 255, 255, 255};    // White thumb when on
//     switchStyle1.hoverTrackColor = {200, 200, 200, 255}; // Slightly darker grey on hover
//     switchStyle1.hoverThumbColor = {240, 240, 240, 255}; // Slightly darker white on hover
//     switchStyle1.trackHeight = 30.0f;
//     switchStyle1.trackWidth = 60.0f;
//     switchStyle1.thumbPadding = 3.0f;
//      myRetainedScrollView->addControl(

//     std::make_unique<XenUI::Switch>(
       
//         XenUI::PositionParams::Anchored(XenUI::Anchor::CENTER_LEFT, 5),  
//         switchStyle1,
//         [](bool isOn) {  // Callback for state changes
//             std::cout << "Retained Switch is now " << (isOn ? "ON" : "OFF") << std::endl;
//         },
//         false  // Initial state: OFF
//     )
// );



  
// myRetainedScrollView->recalculateLayout(winSize.x, winSize.y);
// }




// void setup(SDL_Window* window, SDL_Renderer* renderer) {
//     XenUI::SetWindow(window); // Set window reference for GetWindowSize

//     std::cout << "Initializing text renderer (framework finds font)...\n";
//     if (!textRenderer.isInitialized()) {
//         textRenderer.init(renderer);
//     }
//     if (!textRenderer.isInitialized()) {
//         std::cerr << "CRITICAL ERROR: TEXT RENDERER FAILED TO INITIALIZE - NO FONT FOUND." << std::endl;
//         exit(1);
//     }

//     setupLabels();
//     setupButtons();
//     myinput();
//     setupRadioButtons();
//     setupImage(renderer);
//     setupCheckbox();
//     setupSwitches();
//     setupSliders();
//     setupDropdowns();
//     setupScrollView(renderer);

   
//     int w, h;
//     SDL_GetWindowSize(window, &w, &h);
//     if (myRetainedScrollView) {
//         SDL_Point winSize = XenUI::GetWindowSize();
// myRetainedScrollView->recalculateLayout(winSize.x, winSize.y);
//     }
//     for (auto& lbl : labels)    lbl.recalculateLayout();
//     for (auto& btn : buttons)   btn.recalculateLayout();
//     for (auto& box : Inputs)    box.recalculatePosition();
//     SDL_Point winSize = XenUI::GetWindowSize();
//     for (auto& img : Images)    img->recalculateLayout(winSize.x, winSize.y);


      
//         if (channelGroup) {
//             SDL_Point winSize = XenUI::GetWindowSize();
//             channelGroup->recalculateLayout(winSize.x, winSize.y); // <--- Recalculate retained radio button positions
//         }
 
//         for (auto& slider : sliders) { // <--- RECALCULATE RETAINED SLIDER POSITIONS
//             slider.recalculateLayout();
//         }
//         for (auto& dropdown : dropdowns) { // <--- RECALCULATE RETAINED DROPDOWN POSITIONS
//             dropdown.recalculateLayout();
//         }
        

// }


// void render(SDL_Renderer* renderer, const SDL_Event& evt) {
//     // Clear screen to dark gray
//     SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
//     SDL_RenderClear(renderer);
//     //Image Location
//     float ImageX = 12;
//     float ImageY = 5;

//     if (gImage && gImage->isLoaded()) {
//         // Example: draw at (50, 500)
//         gImage->render(renderer, ImageX*100, ImageY*100, 0.3f, 0.3f, 10.0); // Calls your Image::render

//         // Example: draw another instance, scaled and at different location
//         // gImage->render(renderer, 200, 100, nullptr, 0.0, nullptr, SDL_FLIP_NONE, 0.5f, 0.5f); // 50% scale
//     } else {
//         // Optional
//       SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "gImage is not loaded or is null, not rendering.");
//     }



//     if (myRetainedScrollView) {
//         // Pass {0, 0} as the viewOffset because it's a top-level element directly on the window.
//         myRetainedScrollView->draw(renderer, {0.0f, 0.0f});
//     }
//     // 3) Retained-mode labels
//     for (Label& label : labels) {
//         label.draw(renderer);
//     }

//     // 4) Retained-mode buttons
//     for (Button& button : buttons) {
//         button.draw(renderer, {0.0f, 0.0f}); // Pass a zero offset for top-level elements
//     }

//     // 5) Retained-mode input boxes
//     for (auto& box : Inputs) {
//         box.draw(renderer);
//     }

//     // 6) Retained-mode RadioButtonGroup
//     if (channelGroup) {
//         channelGroup->draw(renderer);
//     }

//     // Retained‑mode checkboxes
//     for (auto& cb : checkboxes) {
//         cb.draw(renderer);
//     }

//     // Retained-mode switches
//     for (auto& switchElem : switches) {
//         switchElem.draw(renderer);
//     }   
//     for (auto &imgPtr : Images) {
//     if (imgPtr) imgPtr->draw(renderer, {0,0});
// }
     

//     for (auto& slider : sliders) {
//         slider.draw(renderer);
//     }
   
//     for (auto& dropdown : dropdowns) {
//         dropdown.draw(renderer, {0,0});
//     }
 





//     SDL_RenderPresent(renderer);
// }



// extern "C" int SDL_main(int argc, char** argv) {
//     // 1. Initialize SDL
//     if (SDL_Init(SDL_INIT_VIDEO) < 0) {
//         std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
//         return 1;
//     }

//     // 2. Create the Window
//     // SDL3’s signature: SDL_CreateWindow(title, x, y, width, height, flags)
//     SDL_Window* window = SDL_CreateWindow(
//         "Xenon UI",                    // Window title
//                // x position
//                // y position
//         1400,                           // width, default window size(width)
//         900,                           // height, default window size(height)
//         SDL_WINDOW_RESIZABLE           // flags
//     );
//     if (!window) {
//         std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
//         SDL_Quit();
//         return 1;
//     }

//     // Set minimum window size
//     const int MIN_WINDOW_WIDTH  = 400;
//     const int MIN_WINDOW_HEIGHT = 200;
//     SDL_SetWindowMinimumSize(window, MIN_WINDOW_WIDTH, MIN_WINDOW_HEIGHT);
//     std::cout << "Minimum window size set to " << MIN_WINDOW_WIDTH << "x" << MIN_WINDOW_HEIGHT << std::endl;


//     SDL_Renderer* renderer = SDL_CreateRenderer(
//         window,
//         nullptr
//     );
//     if (!renderer) {
//         std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
//         SDL_DestroyWindow(window);
//         SDL_Quit();
//         return 1;
//     }

//     // 4. Initialize TTF
//     if (TTF_Init() == -1) {
//         std::cerr << "SDL_ttf could not initialize! TTF_Error: " << SDL_GetError() << std::endl;
//         SDL_DestroyRenderer(renderer);
//         SDL_DestroyWindow(window);
//         SDL_Quit();
//         return 1;
//     }


//     // 5. Call setup (loads fonts, images, widgets)
//     setup(window, renderer);



// /*Hybrid mode loop
//      best for normal apps where sometimes fast render is require and sometimes remain idle
//      */

// bool running       = true;
// SDL_Event event;

// // Dirty‐flag to indicate we should redraw
// bool needsRedraw   = true;
// bool handled = false;

// // Whether we’re currently in a short “game‐style” tick for animations
// bool isAnimating   = false;

// // Track time to update animations
// auto lastEventTime = std::chrono::high_resolution_clock::now();
// Uint64 lastCounter = SDL_GetPerformanceCounter();
// double invFreq     = 1.0 / static_cast<double>(SDL_GetPerformanceFrequency());

// // If no events and no animations for IDLE_THRESHOLD seconds, stay in pure idle
// const double IDLE_THRESHOLD = 2.0; // seconds

// while (running) {
//     if (!isAnimating) {
//         // 1A) Idle: wait for events (or wake up every 500 ms to check if we need to exit idle)
//         if (SDL_WaitEventTimeout(&event, 500)) {
//             // We got an event → process below
//         } else {
//             // Timeout occurred (500 ms passed w/o events)
//             // Check how long since last real event
//             auto nowClock = std::chrono::high_resolution_clock::now();
//             std::chrono::duration<double> sinceLastEvent = nowClock - lastEventTime;
//             if (sinceLastEvent.count() >= IDLE_THRESHOLD) {
//                 // Still idle—no need to update or redraw
//                 continue;
//             }
//             // If we get here, it means we’re within the IDLE_THRESHOLD window
//             // after an event → fall through to updates/redraw
//             event.type = 0; // no new event, but keep lastEventTime as is
//         }
//     } else {
//         // 1B) Animation mode: poll events (do NOT block)
//         if (SDL_PollEvent(&event) == 0) {
//             event.type = 0; // no new event but we stay in animation
//         }
//     }
//  gLastEvent = event;
//     // 2) If event arrived, process it
//     if (event.type != 0) {
//         lastEventTime = std::chrono::high_resolution_clock::now();

//         switch (event.type) {
//             case SDL_EVENT_QUIT:
//                 running = false;
//                 break;

//             case SDL_EVENT_KEY_DOWN:
//                 if (event.key.key == SDLK_F11) {
//                     Uint32 flags = SDL_GetWindowFlags(window);
//                     bool isFull = (flags & SDL_WINDOW_FULLSCREEN) != 0;
//                     SDL_SetWindowFullscreen(
//                         window,
//                         isFull ? 0 : SDL_WINDOW_FULLSCREEN
//                     );
//                     needsRedraw = true;
//                 }
//                 break;

//             case SDL_EVENT_WINDOW_RESIZED:
//                 for (auto& btn : buttons)   btn.recalculateLayout();
//                 for (auto& lbl : labels)    lbl.recalculateLayout();
//                 for (auto& box : Inputs)    box.recalculatePosition();
//                 for (auto& cb : checkboxes) cb.recalculateLayout();
                
                 
//                 if (channelGroup) {
//                     SDL_Point winSize = XenUI::GetWindowSize();
//                     channelGroup->recalculateLayout(winSize.x, winSize.y); 
//                 }

//                 for (auto& switchElem : switches) {
//                     switchElem.recalculateLayout();
//                 }

//                 for (auto& slider : sliders) {
//                     slider.recalculateLayout();
//                 }
//                 for (auto& dropdown : dropdowns) { 
//                     dropdown.recalculateLayout();
//                 }
//                 if (myRetainedScrollView) {
//                      SDL_Point winSize = XenUI::GetWindowSize();
// myRetainedScrollView->recalculateLayout(winSize.x, winSize.y);
//                 }



//                 needsRedraw = true;
//                 break;

//             default:

// if (myRetainedScrollView && myRetainedScrollView->handleEvent(event, window, {0.0f, 0.0f})) {
//     needsRedraw = true;

//     break;
// }

//     for (auto& btn : buttons) {
//         if (btn.handleEvent(event)) {
//             needsRedraw = true;
//             isAnimating = true;
//         }
//     }
//     if (channelGroup && channelGroup->handleEvent(event)) {
//         needsRedraw = true;
//     }
//     for (auto& cb : checkboxes) {
//         if (cb.handleEvent(event)) {
//             needsRedraw = true;
//             std::cout << "Checkbox is now " << (cb.isChecked() ? "ON\n" : "OFF\n");
//         }
//     }
//     for (auto& sw : switches) {
//         if (sw.handleEvent(event)) {
//             needsRedraw = true;
//         }
//     }
//     for (auto& slider : sliders) {
//         if (slider.handleEvent(event)) {
//             needsRedraw = true;
//         }
//     }
//     for (auto& dd : dropdowns) {
//         if (dd.handleEvent(event)) {
//             needsRedraw = true;
//         }
//     }
//     for (auto& img : Images) {
//         if (img->handleEvent(event)) {
//             needsRedraw = true;
//         }
//     }
    
//     break;

//         }
//         // Pass event to input boxes
//         for (auto& box : Inputs) {
//             if (box.handleEvent(event, window, {0,0})) {
//                 needsRedraw = true;
//                 isAnimating = true;
//             }
//         }


//         // Ensure we’ll redraw at least once after processing the event
//         needsRedraw = true;
//     }

//     // 3) If in animation mode, update animations each frame
//     if (isAnimating) {
//         Uint64 nowCounter = SDL_GetPerformanceCounter();
//         double deltaSec = (nowCounter - lastCounter) * invFreq;
//         lastCounter = nowCounter;

//         // Update input boxes (caret blink, text cursor animation, etc.)
//         bool anyStillAnimating = false;
//         for (auto& box : Inputs) {
//             if (box.update(static_cast<float>(deltaSec))) {
//                 needsRedraw = true;
//             }
//             if (box.isAnimating()) {
//                 anyStillAnimating = true;
//             }
//         }


//         if (!anyStillAnimating) {
//             // All animations finished → exit animation mode
//             isAnimating = false;
//         }
//     }

//     if (needsRedraw) {
//         render(renderer, gLastEvent);
//         needsRedraw = false;
//     }
 
// }


// textRenderer.clearCache();
// if (gImage) { delete gImage; gImage = nullptr; }

// if (myRetainedScrollView) {
//     myRetainedScrollView.reset();
// }
// TTF_Quit();
// SDL_DestroyRenderer(renderer);
// SDL_DestroyWindow(window);
// SDL_Quit();


// }


