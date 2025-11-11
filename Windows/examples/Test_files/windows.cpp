

// SPDX-License-Identifier: Apache-2.0
/*
 *  Copyright (C) 2025 MD S M Sarowar Hossain
 *
 *
 * 
 * This is the main file that shows how to use ui elements in a project.
 * It has both Retained and Immediate mode ui elements together in it.
 * 
 */


#include <SDL3/SDL.h>
#include <SDL3/SDL_log.h>
#include <SDL_ttf.h>
#include <SDL_image.h>   

#include <iostream>
#include <vector>
#include <cstring>  // For memcpy
#include <string>
#include <chrono>
#include "XenUI/Image.h"         
#include "XenUI/TextRenderer.h"
#include "XenUI/Label.h"
#include "XenUI/InputBox.h"
#include "XenUI/Shape.h"
#include "XenUI/Button.h"
#include "XenUI/WindowUtil.h"
#include "XenUI/Anchor.h"
#include "XenUI/Position.h" 
#include "XenUI/RadioButton.h"
#include "XenUI/CheckBox.h"
#include "XenUI/Switch.h"  
#include "XenUI/Slider.h"  
#include "XenUI/Dropdown.h" 

#include "XenUI/ScrollView.h"
#include "XenUI/UIElement.h"


static SDL_Event gLastEvent;


// Global text renderer instance
TextRenderer& textRenderer = TextRenderer::getInstance();
std::vector<Label> labels;
std::vector<Button> buttons;                // Store buttons in a vector
std::vector<XenUI::InputBox> Inputs;        // Store InputBoxes in a vector
std::vector<XenUI::Rectangle> shapes;       // Example retained-mode shapes
std::vector<Checkbox> checkboxes;
std::vector<std::unique_ptr<ImageControl>> Images;
std::vector<XenUI::Switch> switches;  // For retained mode switches



std::unique_ptr<ScrollView> myRetainedScrollView;
int scrollX = 0, scrollY = 0; // for immediate mode example

std::vector<Slider> sliders; // Store retained mode sliders
float retainedVolume = 0.5f; // Value for a retained mode slider
float retainedBrightness = 0.8f; // Another retained mode slider value


float immediateAudioLevel = 0.75f; // Value for an immediate mode slider
float Level = 0.5f;
float Level1 = 0.5f;
float Level3 = 0.5f;
float Level4 = 0.5f;

std::vector<Dropdown> dropdowns; // Store retained mode dropdown objects
std::vector<std::string> themes = {"Dark", "Light", "High Contrast", "Custom"};
std::vector<std::string> languages = {"English", "Spanish", "French", "German", "Japanese"};
int selectedThemeIndex = 0; // Value for a retained mode dropdown
int selectedLanguageIndex = 0; // Another retained mode dropdown value


int immediateSelectedFontIndex = 0; // Value for an immediate mode dropdown
int immediateSelectedFontIndexScroll = 0; // Value for an immediate mode dropdown

int numItems = 15;
bool immediateSwitchState = false;  // For immediate mode switch
bool immediateSwitchState1 = false;  // For immediate mode switch
bool immediateChecked = false;     
bool immediateChecked1 = false;     
bool toggled1 = false;   
bool toggled2 = false;   
bool toggled3 = false;   
bool retainedChecked = false;     
bool retainedChecked1 = false;     
bool retainedChecked2 = false;    
bool retainedChecked3 = false;    


int contentStartY = 10;
int rowGap        = 50;

float checkboxBaseY = contentStartY + (numItems * rowGap) + 10;

// Global Image pointer
Image* gImage = nullptr;

// Test variables
int fps = 60;
int FPS = 60;

int selectedChannel = -1;
std::unique_ptr<RadioButtonGroup> channelGroup = nullptr; 


int immediateSelectedOption = 0;



void setupLabels() {
    std::string fpsText = "Fps : " + std::to_string(fps);
    
    labels.emplace_back(
        "Xenon UI",
        XenUI::PositionParams::Anchored(XenUI::Anchor::TOP_CENTER),
        60, // textSize
        SDL_Color{255, 255, 255, 255}
    );

    labels.emplace_back(
        "Developed by MD S M Sarowar Hossain",
        XenUI::PositionParams::Anchored(XenUI::Anchor::TOP_CENTER, 0, 60),
        40, // textSize
        SDL_Color{255, 255, 255, 255}
    );
    labels.emplace_back(
        "Version 0.9.0",
        XenUI::PositionParams::Anchored(XenUI::Anchor::TOP_CENTER, 0, 100),
        30, // textSize
        SDL_Color{255, 255, 255, 200}
    );
}

void setupButtons() {
    ButtonStyle style1;
    style1.textColor = {255, 255, 255, 255};
    style1.bgColor = {10, 200, 100, 255};

    buttons.emplace_back(
        "Button 1",                                           // text
        XenUI::PositionParams::Anchored(XenUI::Anchor::TOP_LEFT, 50, 200),             // Position
        style1,                                                                        // style
        []() {
            fps++;
            
            std::cout << "Button 1 Clicked! fps:"<<fps << std::endl;
        },
        30 // fontSize
    );

    buttons.emplace_back(
        "Button 2",                                                                    // text
        XenUI::PositionParams::Anchored(XenUI::Anchor::TOP_LEFT, 250, 200),              // Position
        style1,                                                                        // style
        []() {
            FPS++;
            
            std::cout << "Button 1 Clicked! fps:" <<FPS<< std::endl;
        },
        30 // fontSize
    );

    ButtonStyle style2;
    style2.textColor = {255, 255, 255, 255};
    style2.bgColor = {50, 50, 50, 255};

    buttons.emplace_back(
        "Second Button",                                                               // text
        XenUI::PositionParams::Anchored(XenUI::Anchor::BOTTOM_RIGHT, -50, -50),        // Position
        style2,                                                                        // style
        []() { std::cout << "Button 2 Clicked!" << std::endl; },
        30
        // fontSize defaults to 16 if not specified
    );
}

void myinput() {
    XenUI::InputBoxStyle customStyle;
    customStyle.bgColor = {30, 70, 90, 255};    //Opaque dark blue
    customStyle.textColor = {220, 220, 220, 255};
    customStyle.drawBackground = true;
    customStyle.drawBorder     = true;
    customStyle.paddingX       = 8;
    customStyle.paddingY       = 4;


    Inputs.emplace_back(
        XenUI::PositionParams::Anchored(XenUI::Anchor::TOP_LEFT, 50, 280),
        "Edit Me!",
        300,         // width
        40,          // fontSize
        customStyle, // PASS THE STYLE OBJECT
        false        // isPassword
    );
}


void setupRadioButtons() {
   
   channelGroup = std::make_unique<RadioButtonGroup>(&selectedChannel, [](int newValue) {
        std::cout << "Retained Radio: Selection changed to: " << newValue << std::endl;
      
    });

    // Define a custom style for retained radio buttons
    RadioButtonStyle retainedRadioStyle;
    retainedRadioStyle.circleColor = {150, 150, 255, 255}; // Light Blue
    retainedRadioStyle.selectedColor = {50, 50, 200, 255}; // Dark Blue
    retainedRadioStyle.labelColor = {255, 255, 255, 255};
    retainedRadioStyle.circleRadius = 12;
    retainedRadioStyle.circlePadding = 10;
    retainedRadioStyle.innerCirclePadding = 5;

    // Add buttons to the group
    channelGroup->addButton(
        "Option A",
        0, // Value
        XenUI::PositionParams::Anchored(XenUI::Anchor::TOP_LEFT, 50, 380), // Position of first button
        retainedRadioStyle, // Pass the style
        25 // Font size
    );

    channelGroup->addButton(
        "Option B",
        1, // Value
        XenUI::PositionParams::Anchored(XenUI::Anchor::TOP_LEFT, 50, 410), // Position of second button
        retainedRadioStyle, // Pass the style
        25 // Font size
    );

    channelGroup->addButton(
        "Option C",
        2, // Value
        XenUI::PositionParams::Anchored(XenUI::Anchor::TOP_LEFT, 50, 440), // Position of third button
        retainedRadioStyle, // Pass the style
        25 // Font size
    );
}


void setupSliders() {
   
    SliderStyle volumeSliderStyle;
    volumeSliderStyle.trackColor = {80, 80, 80, 255};
    volumeSliderStyle.thumbColor = {0, 150, 255, 255}; // Blue thumb
    volumeSliderStyle.thumbHoverColor = {50, 180, 255, 255};
    volumeSliderStyle.trackThickness = 10;
    volumeSliderStyle.thumbSize = 25;
    volumeSliderStyle.valueTextFontSize = 16;

    sliders.emplace_back(
        "retainedVolumeSlider", // Unique ID
        XenUI::Orientation::HORIZONTAL,
        XenUI::PositionParams::Anchored(XenUI::Anchor::TOP_LEFT, 450, 340), 
        150.0f, // Length: 150 pixels wide
        retainedVolume, 0.0f, 1.0f, // Value range 0.0 to 1.0
        volumeSliderStyle,
        [&](float newValue) { // Lambda callback for value changes
            retainedVolume = newValue;
            std::cout << "Retained Volume: " << retainedVolume << std::endl;
        }
    );

    SliderStyle brightnessSliderStyle;
    brightnessSliderStyle.trackColor = {60, 60, 60, 255};
    brightnessSliderStyle.thumbColor = {200, 200, 50, 255}; // Yellow thumb
    brightnessSliderStyle.thumbHoverColor = {220, 220, 70, 255};
    brightnessSliderStyle.trackThickness = 8;
    brightnessSliderStyle.thumbSize = 20;
    brightnessSliderStyle.valueTextFontSize = 14;

    sliders.emplace_back(
        "retainedBrightnessSlider", // Unique ID
        XenUI::Orientation::VERTICAL,
        XenUI::PositionParams::Anchored(XenUI::Anchor::TOP_LEFT, 450, 400), 
        150.0f, // Length: 180 pixels tall
        retainedBrightness, 0.0f, 1.0f, // Value range 0.0 to 1.0
        brightnessSliderStyle,
        [&](float newValue) { // Lambda callback for value changes
            retainedBrightness = newValue;
            std::cout << "Retained Brightness: " << retainedBrightness << std::endl;
        }
    );
}

void setupDropdowns() {
    // Retained Mode Dropdown 1: Theme Selector
    
    DropdownStyle themeDropdownStyle;
    themeDropdownStyle.mainButtonBgColor = {50, 50, 70, 255};
    themeDropdownStyle.mainButtonTextColor = {255, 255, 255, 255};
    themeDropdownStyle.listBgColor = {40, 40, 60, 255};
    themeDropdownStyle.listItemHoverBgColor = {70, 70, 90, 255};
    themeDropdownStyle.mainButtonFontSize = 20;
    themeDropdownStyle.listItemFontSize = 18;
    themeDropdownStyle.paddingX = 15;
    themeDropdownStyle.paddingY = 10;

    dropdowns.emplace_back(
        "themeSelector", // Unique ID
        XenUI::PositionParams::Anchored(XenUI::Anchor::TOP_LEFT, 50, 600),
        200.0f, // Width of the dropdown
        themes,
        selectedThemeIndex, // Initial selection
        themeDropdownStyle,
        [&](int newIndex) { // Callback for selection changes
            selectedThemeIndex = newIndex;
            std::cout << "Selected Theme: " << themes[selectedThemeIndex] << std::endl;
            
        }
    );

 
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

    setupLabels();
    setupButtons();
    myinput();
    setupRadioButtons();

    setupSliders();
    setupDropdowns();



    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    if (myRetainedScrollView) {
        SDL_Point winSize = XenUI::GetWindowSize();
myRetainedScrollView->recalculateLayout(winSize.x, winSize.y);
    }
    for (auto& lbl : labels)    lbl.recalculateLayout();
    for (auto& btn : buttons)   btn.recalculateLayout();
    for (auto& box : Inputs)    box.recalculatePosition();
    SDL_Point winSize = XenUI::GetWindowSize();
    for (auto& img : Images)    img->recalculateLayout(winSize.x, winSize.y);


      
        if (channelGroup) {
            SDL_Point winSize = XenUI::GetWindowSize();
            channelGroup->recalculateLayout(winSize.x, winSize.y); // <--- Recalculate retained radio button positions
        }
       
        for (auto& slider : sliders) { // <--- RECALCULATE RETAINED SLIDER POSITIONS
            slider.recalculateLayout();
        }
        for (auto& dropdown : dropdowns) { // <--- RECALCULATE RETAINED DROPDOWN POSITIONS
            dropdown.recalculateLayout();
        }
        

}


void render(SDL_Renderer* renderer, const SDL_Event& evt) {
    
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_RenderClear(renderer);
   
    float ImageX = 12;
    float ImageY = 5;

    if (gImage && gImage->isLoaded()) {
        
        gImage->render(renderer, ImageX*100, ImageY*100, 0.3f, 0.3f, 10.0); 

        
    } else {
        // Optional
      SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "gImage is not loaded or is null, not rendering.");
    }



        if (myRetainedScrollView) {
       
        myRetainedScrollView->draw(renderer, {0.0f, 0.0f});
    }

    for (Label& label : labels) {
        label.draw(renderer);
    }

 
    for (Button& button : buttons) {
        button.draw(renderer, {0.0f, 0.0f}); 
    }


    for (auto& box : Inputs) {
        box.draw(renderer);
    }

    if (channelGroup) {
        channelGroup->draw(renderer);
    }

    for (auto& cb : checkboxes) {
        cb.draw(renderer);
    }


    for (auto& switchElem : switches) {
        switchElem.draw(renderer);
    }   
    for (auto &imgPtr : Images) {
    if (imgPtr) imgPtr->draw(renderer, {0,0});
}
     

    for (auto& slider : sliders) {
        slider.draw(renderer);
    }

    for (auto& dropdown : dropdowns) {
        dropdown.draw(renderer, {0,0});
    }

   
        if (XenUI::Checkbox(
            "chk_immediate",
         "Checkbox 1",
         &immediateChecked,
            XenUI::PositionParams::Anchored(XenUI::Anchor::CENTER_RIGHT, -150)))
        {
        // toggled this frame
        std::cout << "Immediate checkbox is now " 
                 << (immediateChecked ? "on\n" : "off\n");
        }
        if (XenUI::Checkbox(
            "chk_immediate1",
         "Checkbox 2",
         &immediateChecked1,
            XenUI::PositionParams::Anchored(XenUI::Anchor::CENTER_RIGHT, -150, 30)))
        {
        // toggled this frame
        std::cout << "Immediate checkbox is now " 
                 << (immediateChecked1 ? "on\n" : "off\n");
        }

        XenUI::SwitchStyle immediateSwitchStyle;  // Default style, or customize
        immediateSwitchStyle.trackColorOff = {180, 180, 180, 255};   // Light grey when off
        immediateSwitchStyle.trackColorOn = {255, 0, 0, 255};        // Red when on
        immediateSwitchStyle.thumbColorOff = {255, 255, 255, 255};   // White thumb when off
        immediateSwitchStyle.thumbColorOn = {255, 255, 255, 255};    // White thumb when on
        immediateSwitchStyle.hoverTrackColor = {200, 200, 200, 255}; // Slightly darker grey on hover
        immediateSwitchStyle.hoverThumbColor = {240, 240, 240, 255}; // Slightly darker white on hover
        immediateSwitchStyle.trackHeight = 30.0f;
        immediateSwitchStyle.trackWidth = 60.0f;
        immediateSwitchStyle.thumbPadding = 3.0f;
        
        immediateSwitchStyle.labelOff = "Off";
        immediateSwitchStyle.labelOn  = "On";
        immediateSwitchStyle.labelFontSize = 14;


if (XenUI::SwitchImmediate(
        "immediate_switch_1",  // Unique ID
        // textRenderer,
        XenUI::PositionParams::Anchored(XenUI::Anchor::TOP_LEFT, 50, 510), 
        immediateSwitchStyle,
        &immediateSwitchState,  // State variable (updated automatically)
        false  // Trigger on release
    
    )) {
    std::cout << "Immediate Switch is now " << (immediateSwitchState ? "ON" : "OFF") << std::endl;
}





    XenUI::DrawImage(
    "scroll_image2", // unique cache key for the image
    renderer,
    "C:/Program Files (x86)/My Application/Images/Image.png", // path to the image
    XenUI::PositionParams::Anchored(XenUI::Anchor::CENTER_RIGHT, -60, 100),
    0,0, {0,0}, 100, 100

);



ScrollViewStyle style;
style.bgColor               = {30, 30,  60, 255};
style.borderColor           = {80, 80, 110, 255};
style.scrollbarBgColor      = {20, 20,  20, 255};
style.scrollbarThumbColor   = {100,100,100,255};
style.scrollbarThumbHoverColor  = {140,140,140,255};
style.scrollbarThumbGrabbedColor = {180,180,180,255};
style.scrollbarWidth        = 14;


//Position params for immedate mode scrollview 
XenUI::PositionParams p = XenUI::PositionParams::Anchored(XenUI::Anchor::BOTTOM_CENTER, 0, 0, 300, 100);
//----------------------X-----Y----W----H---
SDL_FRect   view    = { 500, 50, 500, 300 };
SDL_FPoint content = { 0, 600 };
static SDL_FPoint scroll;  // persistent scroll state

int viewW = static_cast<int>(view.w);
int viewH = static_cast<int>(view.h);

SDL_FPoint ofs = XenUI::BeginScrollView("settings_list", p, viewW, viewH, content, renderer, evt, style);

int contentClipW = (content.y > viewH) ? (viewW - style.scrollbarWidth) : viewW;
int contentClipH = viewH;



ButtonStyle scrollButtonStyle;
    scrollButtonStyle.bgColor = {20, 120, 200, 255};
    scrollButtonStyle.hoverColor = {40, 140, 220, 255};
    scrollButtonStyle.pressedColor = {0, 100, 180, 255};
    scrollButtonStyle.paddingX = 15;
    scrollButtonStyle.paddingY = 8;
XenUI::Button("ok_btnn",
                      "Click me",
                      XenUI::PositionParams::Anchored(XenUI::Anchor::TOP_LEFT, ofs.x+5, ofs.y+5),
                      renderer,
                      {0.0f, 0.0f},
                      scrollButtonStyle,
                      30,
                      true,
                    contentClipW, 
                    contentClipH); // triggerOnPress = true
    


XenUI::DrawImage(
    "scroll_image1", // unique cache key for the image
    renderer,
    "C:/Program Files (x86)/My Application/Images/Image.png"
, // path to the image
    XenUI::PositionParams::Anchored(XenUI::Anchor::BOTTOM_CENTER),
    contentClipW,
    contentClipH,
    ofs, // The view offset is crucial for scrolling
    100.0f, // desired width
    100.0f // desired height
);

    XenUI::Label("Immadeiate label",
                 XenUI::PositionParams::Absolute(5, 450),
                 30,
                 SDL_Color{200, 200, 50, 255},
                 contentClipW, 
                 contentClipH,
                ofs);

 std::vector<std::string> immediateOptions = {"Yes", "No", "Maybe"};
    RadioButtonStyle retainedRadioStyle;
    retainedRadioStyle.circleColor = {150, 150, 255, 255}; // Light Blue
    retainedRadioStyle.selectedColor = {50, 50, 200, 255}; // Dark Blue
    retainedRadioStyle.labelColor = {255, 255, 255, 255};
    retainedRadioStyle.circleRadius = 12;
    retainedRadioStyle.circlePadding = 10;
    retainedRadioStyle.innerCirclePadding = 5;

        if (XenUI::RadioGroupImmediate(
            "my_immediate_radio_group1",
            immediateOptions,
            &immediateSelectedOption,
            XenUI::PositionParams::Absolute(5, 350), // Position the group
            retainedRadioStyle,
            18, // Font size for immediate
            30,  // Spacing between options
            ofs,
            contentClipW, 
                 contentClipH
        )) {
   
        std::cout << "Immediate Radio: Selection changed to option index: " << immediateSelectedOption << std::endl;

    }



SliderStyle immediateSliderStyle1;
    immediateSliderStyle1.trackColor = {40, 40, 40, 255};
    immediateSliderStyle1.thumbColor = {255, 100, 0, 255}; // Orange thumb
    immediateSliderStyle1.thumbHoverColor = {255, 150, 50, 255};
    immediateSliderStyle1.trackThickness = 12;
    immediateSliderStyle1.thumbSize = 28;
    immediateSliderStyle1.valueTextFontSize = 18;
    
    if (XenUI::Slider(
            "immediateAudioSlider4", // Unique ID
            XenUI::Orientation::HORIZONTAL,
            XenUI::PositionParams::Anchored(XenUI::Anchor::CENTER), 
            250.0f, // Length: 250 pixels wide
            &Level4, 0.0f, 1.0f, // Value range 0.0 to 1.0
            contentClipW, 
                 contentClipH,
            immediateSliderStyle1, 
            ofs
        )) {
      
        std::cout << "Immediate Audio Level: " << Level4 << std::endl;
    }



    if (XenUI::SwitchImmediate(
        "immediate_switch_2",  // Unique ID
        XenUI::PositionParams::Anchored(XenUI::Anchor::BOTTOM_LEFT,5),  // Position at 
        immediateSwitchStyle,
        &immediateSwitchState1,  // State variable (updated automatically)
        false , // Trigger on release
        contentClipW,     // parentWidth — important for anchors
        contentClipH, 
        ofs
    )) {
    std::cout << "Immediate Switch is now " << (immediateSwitchState1 ? "ON" : "OFF") << std::endl;
}


 std::vector<std::string> fonts = {"Arial", "Verdana", "Times New Roman", "Courier New"};
    DropdownStyle fontDropdownStyle;
    fontDropdownStyle.mainButtonBgColor = {90, 90, 90, 255};
    fontDropdownStyle.mainButtonTextColor = {255, 255, 255, 255};
    fontDropdownStyle.listBgColor = {70, 70, 70, 255};
    fontDropdownStyle.listItemHoverBgColor = {120, 120, 120, 255};
    fontDropdownStyle.mainButtonFontSize = 18;
    fontDropdownStyle.listItemFontSize = 16;
    fontDropdownStyle.paddingX = 12;
    fontDropdownStyle.paddingY = 9;
if (XenUI::Dropdown(
            "immediateFontSelectorScroll", // Unique ID
            XenUI::PositionParams::Anchored(XenUI::Anchor::TOP_RIGHT, -5,5), // Position
            180.0f, // Width
            fonts,
            &immediateSelectedFontIndexScroll, // Pointer to the variable to modify
            fontDropdownStyle,
            ofs,
            contentClipW,     // parentWidth — important for anchors
        contentClipH

        )) {
       
        std::cout << "Immediate Font Selection: " << fonts[immediateSelectedFontIndexScroll] << std::endl;
       
    }



   
// End and restore:
XenUI::EndScrollView(renderer);



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
        1400,                           // width, default window size(width)
        900,                           // height, default window size(height)
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
        //  Idle: wait for events (or wake up every 500 ms to check if we need to exit idle)
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
        //  Animation mode: poll events
        if (SDL_PollEvent(&event) == 0) {
            event.type = 0; // no new event but we stay in animation
        }
    }
 gLastEvent = event;
    //  If event arrived, process it
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
                
                 

               
                if (channelGroup) {
                    SDL_Point winSize = XenUI::GetWindowSize();
                    channelGroup->recalculateLayout(winSize.x, winSize.y); // <--- Recalculate positions on resize
                }
              
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
  
    break;
}

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
    for (auto& img : Images) {
        if (img->handleEvent(event)) {
            needsRedraw = true;
        }
    }
    
    break;

        }
        // Pass event to input boxes
        for (auto& box : Inputs) {
            if (box.handleEvent(event, window, {0,0})) {
                needsRedraw = true;
   
                isAnimating = true;
            }
        }

 
        needsRedraw = true;
    }

    // 3) If in animation mode, update animations each frame
    if (isAnimating) {
        Uint64 nowCounter = SDL_GetPerformanceCounter();
        double deltaSec = (nowCounter - lastCounter) * invFreq;
        lastCounter = nowCounter;

        bool anyStillAnimating = false;
        for (auto& box : Inputs) {
            if (box.update(static_cast<float>(deltaSec))) {
                needsRedraw = true;
            }
            if (box.isAnimating()) {
                anyStillAnimating = true;
            }
        }



        if (!anyStillAnimating) {
            // All animations finished → exit animation mode
            isAnimating = false;
        }
    }


    if (needsRedraw) {
        render(renderer, gLastEvent);
        needsRedraw = false;
    }

}


textRenderer.clearCache();
if (gImage) { delete gImage; gImage = nullptr; }

if (myRetainedScrollView) {
    myRetainedScrollView.reset();
}
TTF_Quit();
SDL_DestroyRenderer(renderer);
SDL_DestroyWindow(window);
SDL_Quit();


}


