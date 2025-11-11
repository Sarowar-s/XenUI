
// SPDX-License-Identifier: Apache-2.0
/*
 *  Copyright (C) 2025 MD S M Sarowar Hossain
 *
 * 
 * 
 * 
 * 
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
// --- DROPDOWN ADDITIONS END ---
int numItems = 15;
bool immediateSwitchState = false;  // For immediate mode switch
bool immediateSwitchState1 = false;  // For immediate mode switch
bool immediateChecked = false;     
bool toggled1 = false;   
bool toggled2 = false;   
bool retainedChecked = false;     
bool retainedChecked2 = false;    

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

// Forward declarations
void setupLabels();
void setupButtons();
void myinput();
void render(SDL_Renderer* renderer);
void setup(SDL_Window* window, SDL_Renderer* renderer);
void setupRadioButtons();
void setupSliders();

void setupLabels() {
    std::string fpsText = "Fps : " + std::to_string(fps);
    
    labels.emplace_back(
        "Fps::60",
        XenUI::PositionParams::Anchored(XenUI::Anchor::CENTER, -100, -30),
        30, // textSize
        SDL_Color{255, 255, 255, 255}
    );

    labels.emplace_back(
        fpsText,
        XenUI::PositionParams::Absolute(100, 100),
        35, // textSize
        SDL_Color{255, 255, 255, 255}
    );
}

void setupButtons() {
    ButtonStyle style1;
    style1.textColor = {255, 255, 255, 255};
    style1.bgColor = {10, 200, 100, 255};

    buttons.emplace_back(
        "Add immediate + retained mode Fps",                                           // text
        XenUI::PositionParams::Anchored(XenUI::Anchor::TOP_LEFT, 50, 370),             // Position
        style1,                                                                        // style
        []() {
            fps++;
            std::string UpdatedText = "Fps : " + std::to_string(fps);
            labels[0].setText(UpdatedText);
            std::cout << "Button 1 Clicked!" << std::endl;
        },
        30 // fontSize
    );

    buttons.emplace_back(
        "Click Me",                                                                    // text
        XenUI::PositionParams::Anchored(XenUI::Anchor::TOP_LEFT, 50, 50),              // Position
        style1,                                                                        // style
        []() {
            FPS++;
            std::string UpdatedText = "FPS; : " + std::to_string(FPS);
            labels[1].setText(UpdatedText);
            std::cout << "Button 1 Clicked!" << std::endl;
        },
        40 // fontSize
    );

    ButtonStyle style2;
    style2.textColor = {255, 255, 255, 255};
    style2.bgColor = {50, 50, 50, 255};

    buttons.emplace_back(
        "Second Button",                                                               // text
        XenUI::PositionParams::Anchored(XenUI::Anchor::BOTTOM_RIGHT, -50, -50),        // Position
        style2,                                                                        // style
        []() { std::cout << "Button 2 Clicked!" << std::endl; }
        // fontSize defaults to 16 if not specified
    );
}

void myinput() {
    XenUI::InputBoxStyle customStyle;
    customStyle.bgColor = {30, 70, 90, 255};    // Example: Opaque dark blue
    customStyle.textColor = {220, 220, 220, 255};
    customStyle.drawBackground = true;
    customStyle.drawBorder     = true;
    customStyle.paddingX       = 8;
    customStyle.paddingY       = 4;
    // ... set other style properties ...

    Inputs.emplace_back(
        XenUI::PositionParams::Absolute(100, 200),
        "Edit Me!",
        300,         // width
        40,          // fontSize
        customStyle, // PASS THE STYLE OBJECT
        false        // isPassword
    );
}

// --- ADDITIONS START ---
void setupRadioButtons() {
    // Retained Mode RadioButtonGroup
    // Create the group on the heap as it manages other objects
   channelGroup = std::make_unique<RadioButtonGroup>(&selectedChannel, [](int newValue) {
        std::cout << "Retained Radio: Selection changed to: " << newValue << std::endl;
        // You can update other UI elements or game state here
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
        "Option A (Channel 0)",
        0, // Value
        XenUI::PositionParams::Absolute(500, 50), // Position of first button
        retainedRadioStyle, // Pass the style
        25 // Font size
    );

    channelGroup->addButton(
        "Option B (Channel 1)",
        1, // Value
        XenUI::PositionParams::Absolute(500, 90), // Position of second button
        retainedRadioStyle, // Pass the style
        25 // Font size
    );

    channelGroup->addButton(
        "Option C (Channel 2)",
        2, // Value
        XenUI::PositionParams::Absolute(500, 130), // Position of third button
        retainedRadioStyle, // Pass the style
        25 // Font size
    );
}
// --- ADDITIONS END ---




void setupCheckbox(){
    // Retained‑mode checkbox:
checkboxes.emplace_back(
    "Enable Feature X",
    XenUI::PositionParams::Absolute(400, 200),
    /*initialState=*/retainedChecked
  );
checkboxes.emplace_back(
    "Enable Feature W",
    XenUI::PositionParams::Absolute(400, 150),
    /*initialState=*/retainedChecked2,
    CheckboxStyle{},
    DEFAULT_CHECKBOX_FONT_SIZE,
    [](bool isChecked){
        std::cout<<"Retained CheckBox w is now "<< isChecked ? "On\n" : "Off\n";
    }
  );
}


void setupSwitches() {
    XenUI::SwitchStyle switchStyle;
    switchStyle.trackColorOff = {180, 180, 180, 255};   // Light grey when off
    switchStyle.trackColorOn = {100, 200, 100, 255};    // Green when on
    switchStyle.thumbColorOff = {255, 255, 255, 255};   // White thumb when off
    switchStyle.thumbColorOn = {255, 255, 255, 255};    // White thumb when on
    switchStyle.hoverTrackColor = {200, 200, 200, 255}; // Slightly darker grey on hover
    switchStyle.hoverThumbColor = {240, 240, 240, 255}; // Slightly darker white on hover
    switchStyle.trackHeight = 30.0f;
    switchStyle.trackWidth = 60.0f;
    switchStyle.thumbPadding = 3.0f;

    switches.emplace_back(
       // textRenderer,  // Your global TextRenderer instance
        XenUI::PositionParams::Absolute(100, 300),  // Position at (100, 300)
        switchStyle,
        [](bool isOn) {  // Callback for state changes
            std::cout << "Retained Switch is now " << (isOn ? "ON" : "OFF") << std::endl;
        },
        false  // Initial state: OFF
    );
}



// --- SLIDER ADDITIONS START ---
void setupSliders() {
    // --- Retained Mode Sliders ---
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
        XenUI::PositionParams::Absolute(50, 500), // Position at (50, 500)
        250.0f, // Length: 250 pixels wide
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
        XenUI::PositionParams::Anchored(XenUI::Anchor::TOP_RIGHT, -30, 50), // 30px left from top-right, 50px down
        180.0f, // Length: 180 pixels tall
        retainedBrightness, 0.0f, 1.0f, // Value range 0.0 to 1.0
        brightnessSliderStyle,
        [&](float newValue) { // Lambda callback for value changes
            retainedBrightness = newValue;
            std::cout << "Retained Brightness: " << retainedBrightness << std::endl;
        }
    );
}
// --- SLIDER ADDITIONS END ---



// --- DROPDOWN ADDITIONS START ---
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
        XenUI::PositionParams::Absolute(50, 50), // Position at (50, 50)
        200.0f, // Width of the dropdown
        themes,
        selectedThemeIndex, // Initial selection
        themeDropdownStyle,
        [&](int newIndex) { // Callback for selection changes
            selectedThemeIndex = newIndex;
            std::cout << "Selected Theme: " << themes[selectedThemeIndex] << std::endl;
            // Apply theme changes here
        }
    );

    // Retained Mode Dropdown 2: Language Selector
    
    DropdownStyle langDropdownStyle;
    langDropdownStyle.mainButtonBgColor = {70, 50, 50, 255};
    langDropdownStyle.mainButtonTextColor = {255, 255, 255, 255};
    langDropdownStyle.listBgColor = {60, 40, 40, 255};
    langDropdownStyle.listItemHoverBgColor = {90, 70, 70, 255};
    langDropdownStyle.mainButtonFontSize = 16;
    langDropdownStyle.listItemFontSize = 14;
    langDropdownStyle.paddingX = 10;
    langDropdownStyle.paddingY = 8;
    langDropdownStyle.drawBorder = false; // Example: no border

    dropdowns.emplace_back(
        "languageSelector", // Unique ID
        XenUI::PositionParams::Anchored(XenUI::Anchor::TOP_LEFT, 280, 50), // Position next to theme selector
        150.0f, // Width
        languages,
        selectedLanguageIndex, // Initial selection
        langDropdownStyle,
        [&](int newIndex) { // Callback
            selectedLanguageIndex = newIndex;
            std::cout << "Selected Language: " << languages[selectedLanguageIndex] << std::endl;
            // Update language settings
        }
    );
}
// --- DROPDOWN ADDITIONS END ---


// void setupScrollView(SDL_Renderer* renderer) {
//     // 1. Create the ScrollView instance
//     // Position it at (50, 50) with a size of 300x400 pixels
//     myRetainedScrollView = std::make_shared<XenUI::ScrollView>(
//         XenUI::PositionParams::Absolute(50, 50),
//         300, // width of the view
//         400  // height of the view
//     );

//     // 2. Define a style for the buttons inside the scroll view
//     ButtonStyle scrollButtonStyle;
//     scrollButtonStyle.bgColor = {20, 120, 200, 255}; // A nice blue
//     scrollButtonStyle.paddingX = 15;
//     scrollButtonStyle.paddingY = 8;

//     // 3. Create and add buttons to the ScrollView
//     // Note their Y positions are large, making the content taller than the view
//     for (int i = 0; i < 15; ++i) {
//         std::string buttonText = "Scroll Button " + std::to_string(i + 1);
//         int yPos = 10 + (i * 60); // Position buttons vertically, 60 pixels apart

//         myRetainedScrollView->addChild(
//             std::make_shared<Button>(
//                 buttonText,
//                 // Position is relative to the scroll view's content area
//                 XenUI::PositionParams::Absolute(10, yPos),
//                 scrollButtonStyle,
//                 [i]() { std::cout << "Clicked scroll button " << i + 1 << std::endl; },
//                 24 // font size
//             )
//         );
//     }
// }

// New function for setting up the ScrollView and its content
void setupScrollView(SDL_Renderer* renderer) { // Assume renderer is passed in
    // Create the ScrollView (width = 300, height = 400)
    myRetainedScrollView = std::make_unique<ScrollView>(
        

        XenUI::PositionParams::Anchored(XenUI::Anchor::TOP_LEFT, 10, 10, 700, 500)
    );

    // Button style
    ButtonStyle scrollButtonStyle;
    scrollButtonStyle.bgColor = {20, 120, 200, 255};
    scrollButtonStyle.hoverColor = {40, 140, 220, 255};
    scrollButtonStyle.pressedColor = {0, 100, 180, 255};
    scrollButtonStyle.paddingX = 15;
    scrollButtonStyle.paddingY = 8;

    // Content layout constants
    const int contentPaddingLeft = 10;
    const int contentStartY = 10;
    const int rowGap = 50;

    // Add 20 buttons
    for (int i = 0; i < 20; ++i) {
        std::string buttonText = "Scroll Button " + std::to_string(i + 1);
        int yPos = contentStartY + (i * rowGap);

        myRetainedScrollView->addControl(
            std::make_unique<Button>(
                buttonText,
                XenUI::PositionParams::Anchored(XenUI::Anchor::TOP_CENTER, 0, yPos),
                scrollButtonStyle,
                [i]() { std::cout << "Clicked scroll button " << (i + 1) << std::endl; },
                20
            )
        );
    }

    // Now add retained-mode checkboxes **inside the content area** (use same left padding).
    // Put them after the buttons so they are visible as you scroll.
    int checkboxBaseY = contentStartY + (20 * rowGap) + 10; // below the 20 buttons
    DropdownStyle themeDropdownStyle;
    themeDropdownStyle.mainButtonBgColor = {50, 50, 70, 255};
    themeDropdownStyle.mainButtonTextColor = {255, 255, 255, 255};
    themeDropdownStyle.listBgColor = {40, 40, 60, 255};
    themeDropdownStyle.listItemHoverBgColor = {70, 70, 90, 255};
    themeDropdownStyle.mainButtonFontSize = 20;
    themeDropdownStyle.listItemFontSize = 18;
    themeDropdownStyle.paddingX = 15;
    themeDropdownStyle.paddingY = 10;
    myRetainedScrollView->addControl(
        std::make_unique<Dropdown>(
            "themeSelector", // Unique ID
            XenUI::PositionParams::Absolute(contentPaddingLeft, checkboxBaseY + 40 + 50), // Position at (50, 50)
            200.0f, // Width of the dropdown
            themes,
            selectedThemeIndex, // Initial selection
            themeDropdownStyle,
            [&](int newIndex) { // Callback for selection changes
                selectedThemeIndex = newIndex;
                std::cout << "Selected Theme: " << themes[selectedThemeIndex] << std::endl;
                // Apply theme changes here
            }
        )
    );
     int imageYPos = checkboxBaseY + 40 + 40 + 50; // a bit of space after the last checkbox
    myRetainedScrollView->addControl(
        std::make_unique<ImageControl>(
            renderer,
            "Images/Image.png", // The path to your image file
            XenUI::PositionParams::Absolute(contentPaddingLeft, imageYPos),
            120.0f,  // Desired width (e.g., 100 pixels)
            80.0f     // Desired height, 0.0f will preserve aspect ratio
        )
    );
    

    CheckboxStyle cbStyle; // use default style or customize
    myRetainedScrollView->addControl(
        std::make_unique<Checkbox>(
            "Enable Feature X",
            XenUI::PositionParams::Absolute(contentPaddingLeft, checkboxBaseY),
            /*initialState=*/retainedChecked,
            cbStyle,
            DEFAULT_CHECKBOX_FONT_SIZE,
            [](bool isChecked){
                std::cout << "Retained CheckBox X is now " << (isChecked ? "On\n" : "Off\n");
            }
        )
    );



    myRetainedScrollView->addControl(
        std::make_unique<Checkbox>(
            "Enable Feature W",
            XenUI::PositionParams::Absolute(contentPaddingLeft, checkboxBaseY + 40),
            /*initialState=*/retainedChecked2,
            cbStyle,
            DEFAULT_CHECKBOX_FONT_SIZE,
            [](bool isChecked){
                std::cout << "Retained CheckBox W is now " << (isChecked ? "On\n" : "Off\n");
            }
        )
    );

    // Add a retained-mode ImageControl to the ScrollView.
    // This will be drawn as part of the scrolling content.
   
    // Optional: explicitly recalc layout for the scrollview (addControl already updates children,
    // but this is safe and ensures m_contentHeight is accurate).


        XenUI::InputBoxStyle customStyle;
    customStyle.bgColor = {30, 70, 90, 255};    // Example: Opaque dark blue
    customStyle.textColor = {220, 220, 220, 255};
    customStyle.drawBackground = true;
    customStyle.drawBorder     = true;
    customStyle.paddingX       = 8;
    customStyle.paddingY       = 4;
    // ... set other style properties ...
    myRetainedScrollView->addControl(
    std::make_unique<XenUI::InputBox>(
        XenUI::PositionParams::Absolute(contentPaddingLeft, checkboxBaseY + 40 + 40 + 50 + 50),
        "Edit Me!",
        300,         // width
        40,          // fontSize
        customStyle, // PASS THE STYLE OBJECT
        false        // isPassword
    )
    );
    myRetainedScrollView->addControl(
      std::make_unique<Label>(
        "Inside ScrollView 1",
         XenUI::PositionParams::Anchored(XenUI::Anchor::CENTER),
        30, // textSize
        SDL_Color{255, 255, 255, 255}
    )
);
    myRetainedScrollView->addControl(
    std::make_unique<Label>(
        "Inside Scrollview 2",
         XenUI::PositionParams::Absolute(contentPaddingLeft, checkboxBaseY + 300),
        32, // textSize
        SDL_Color{255, 255, 255, 255}
    )
    );
    // Somewhere in your UI setup function

// external or local selected value variable
static int selectedChannel = 0; // or a member variable

// Create the group (use unique_ptr so it can be stored in ScrollView)
auto channelGroup = std::make_unique<RadioButtonGroup>(&selectedChannel,
    [](int newValue) {
        std::cout << "Retained Radio: Selection changed to: " << newValue << std::endl;
        // update app state here if needed
    }
);

// Define a custom style (same as your snippet)
RadioButtonStyle retainedRadioStyle;
retainedRadioStyle.circleColor = {150, 150, 255, 255}; // Light Blue
retainedRadioStyle.selectedColor = {50, 50, 200, 255}; // Dark Blue
retainedRadioStyle.labelColor = {255, 255, 255, 255};
retainedRadioStyle.circleRadius = 12;
retainedRadioStyle.circlePadding = 10;
retainedRadioStyle.innerCirclePadding = 5;

// Add the buttons BEFORE moving into the ScrollView.
// Use positions that are content-space coordinates (relative to scrollview content origin).
channelGroup->addButton(
    "Option A (Channel 0)",
    0,
    XenUI::PositionParams::Absolute(contentPaddingLeft, checkboxBaseY + 300+30),
    retainedRadioStyle,
    25
);

channelGroup->addButton(
    "Option B (Channel 1)",
    1,
    XenUI::PositionParams::Absolute(contentPaddingLeft, checkboxBaseY + 330+30),
    retainedRadioStyle,
    25
);

channelGroup->addButton(
    "Option C (Channel 2)",
    2,
    XenUI::PositionParams::Absolute(contentPaddingLeft , checkboxBaseY + 360+30),
    retainedRadioStyle,
    25
);

// Recalculate group's internal layout so getBounds() etc. are correct.
// Some implementations recalc automatically when you add, but call explicitly to be safe.
channelGroup->recalculateLayout();

// Then add the group to your scroll view
myRetainedScrollView->addControl(std::move(channelGroup));

// Finally re-run layout for scroll view so content height is updated and scrollbars recalc.



myRetainedScrollView->addControl(
    std::make_unique<XenUI::Circle>(
        XenUI::PositionParams::Absolute(contentPaddingLeft, checkboxBaseY +400+50),
        50,
        SDL_Color{20, 100, 200, 255}
    )
);

myRetainedScrollView->addControl(
    std::make_unique<XenUI::Rectangle>(
        XenUI::PositionParams::Absolute(contentPaddingLeft, checkboxBaseY + 550 + 100),
        200, 100,
        SDL_Color{0, 200, 120, 255}
    )
);


    SliderStyle volumeSliderStyle1;
    volumeSliderStyle1.trackColor = {80, 80, 80, 255};
    volumeSliderStyle1.thumbColor = {0, 150, 255, 255}; // Blue thumb
    volumeSliderStyle1.thumbHoverColor = {50, 180, 255, 255};
    volumeSliderStyle1.trackThickness = 10;
    volumeSliderStyle1.thumbSize = 25;
    volumeSliderStyle1.valueTextFontSize = 16;

    myRetainedScrollView->addControl(
   // --- Retained Mode Sliders ---


    std::make_unique<Slider>(
        "retainedVolumeSlider", // Unique ID
        XenUI::Orientation::HORIZONTAL,
        XenUI::PositionParams::Absolute(contentPaddingLeft, checkboxBaseY + 550 + 100+100), // Position at (50, 500)
        250.0f, // Length: 250 pixels wide
        retainedVolume, 0.0f, 1.0f, // Value range 0.0 to 1.0
        volumeSliderStyle1,
        [&](float newValue) { // Lambda callback for value changes
            retainedVolume = newValue;
            std::cout << "Retained Volume: " << retainedVolume << std::endl;
        }
    )
);



    SliderStyle brightnessSliderStyle2;
    brightnessSliderStyle2.trackColor = {60, 60, 60, 255};
    brightnessSliderStyle2.thumbColor = {200, 200, 50, 255}; // Yellow thumb
    brightnessSliderStyle2.thumbHoverColor = {220, 220, 70, 255};
    brightnessSliderStyle2.trackThickness = 8;
    brightnessSliderStyle2.thumbSize = 20;
    brightnessSliderStyle2.valueTextFontSize = 14;

    myRetainedScrollView->addControl(

   std::make_unique<Slider>(
        "retainedBrightnessSlider1", // Unique ID
        XenUI::Orientation::VERTICAL,
        XenUI::PositionParams::Anchored(XenUI::Anchor::CENTER), // 30px left from top-right, 50px down
        180.0f, // Length: 180 pixels tall
        retainedBrightness, 0.0f, 1.0f, // Value range 0.0 to 1.0
        brightnessSliderStyle2,
        [&](float newValue) { // Lambda callback for value changes
            retainedBrightness = newValue;
            std::cout << "Retained Brightness: " << retainedBrightness << std::endl;
        }
    )
);




XenUI::SwitchStyle switchStyle1;
    switchStyle1.trackColorOff = {180, 180, 180, 255};   // Light grey when off
    switchStyle1.trackColorOn = {100, 200, 100, 255};    // Green when on
    switchStyle1.thumbColorOff = {255, 255, 255, 255};   // White thumb when off
    switchStyle1.thumbColorOn = {255, 255, 255, 255};    // White thumb when on
    switchStyle1.hoverTrackColor = {200, 200, 200, 255}; // Slightly darker grey on hover
    switchStyle1.hoverThumbColor = {240, 240, 240, 255}; // Slightly darker white on hover
    switchStyle1.trackHeight = 30.0f;
    switchStyle1.trackWidth = 60.0f;
    switchStyle1.thumbPadding = 3.0f;
     myRetainedScrollView->addControl(

    std::make_unique<XenUI::Switch>(
       // textRenderer,  // Your global TextRenderer instance
        XenUI::PositionParams::Anchored(XenUI::Anchor::TOP_RIGHT, contentPaddingLeft, 25),  // Position at (100, 300)
        switchStyle1,
        [](bool isOn) {  // Callback for state changes
            std::cout << "Retained Switch is now " << (isOn ? "ON" : "OFF") << std::endl;
        },
        false  // Initial state: OFF
    )
);























   SDL_Point winSize = XenUI::GetWindowSize();
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

    // Load an example image from assets (Android) or file path (desktop).
    // On Android, put "images/foo.png" under android-project/app/src/main/assets/images/
    // On desktop, pass a full path, e.g. "assets/images/foo.png"
 // const char* imagePath = "Images/simple_test.bmp"; // or .png
 const char* imagePath = "Images/Image.png"; // Your original image
 if(!imagePath) {
    SDL_Log("Image path incorrect : %s", SDL_GetError());
 }

 SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Attempting to load image: %s", imagePath);
 gImage = new Image(renderer, imagePath); // Calls your Image constructor

 if (!gImage->isLoaded()) {
     // The Image constructor already logs details, but you can add a summary
     SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load image '%s' at startup. Check previous logs from Image constructor.", imagePath);
     // Consider not exiting here, but gImage->render will simply not draw
 } else {
     SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Image '%s' loaded successfully. Dimensions: %d x %d",
         imagePath, gImage->getWidth(), gImage->getHeight());
 }
    setupLabels();
    setupButtons();
    myinput();
    setupRadioButtons();
    setupCheckbox();
    setupSwitches();
    setupSliders();
    setupDropdowns();
    setupScrollView(renderer);

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
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
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
      SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "gImage is not loaded or is null, not rendering.");
    }

    
    // 2) Drawing Retained-Mode shapes
    // XenUI::Rectangle rect(
    //     XenUI::PositionParams::Absolute(50, 50),
    //     200,
    //     50,
    //     SDL_Color{50, 150, 255, 255}
    // );
    // XenUI::Rectangle rect1(
    //     XenUI::PositionParams::Anchored(XenUI::Anchor::BOTTOM_CENTER, 0, 0),
    //     200,
    //     -1,
    //     SDL_Color{50, 150, 255, 255}
    // ); // dynamic height
    // XenUI::Circle circle(
    //     XenUI::PositionParams::Anchored(XenUI::Anchor::TOP_RIGHT, -50, 50),
    //     60,
    //     SDL_Color{255, 80, 80, 255}
    // );
    // rect.draw(renderer);
    // rect1.draw(renderer);
    // circle.draw(renderer);

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







    // === Immediate-Mode Elements ===
   std::string stest = "Volume : " + std::to_string(retainedVolume);
   
    // Immediate-mode labels
    XenUI::Label(stest,
                 XenUI::PositionParams::Absolute(10, 200),
                 30,
                 SDL_Color{200, 200, 50, 255});

    std::string stest1 = "Volume : " + std::to_string(retainedBrightness);

    XenUI::Label(stest1,
                 XenUI::PositionParams::Anchored(XenUI::Anchor::TOP_RIGHT, -10, 10),
                 30,
                 SDL_Color{255, 255, 255, 255});
                 

    // Immediate-mode button
    ButtonStyle blackstyle;
    blackstyle.textColor = {255, 255, 255, 255};
    blackstyle.bgColor   = {0, 0, 0, 255};

    if (XenUI::Button("ok_btn",
                      "Add immediate mode Fps only",
                      XenUI::PositionParams::Absolute(200, 300),
                      renderer,
                      {0.0f, 0.0f},
                      blackstyle,
                      50,
                      true)) // triggerOnPress = true
    {
        fps++;
        std::cout << "OK Button Pressed\n";
    }

    // Update FPS label immediately
    std::string fpsText = "Fps : " + std::to_string(fps);
    XenUI::Label(fpsText,
                 XenUI::PositionParams::Anchored(XenUI::Anchor::TOP_RIGHT, -10, 40),
                 25,
                 SDL_Color{0, 255, 0, 255});

    ButtonStyle redStyle;
    redStyle.bgColor    = {180, 30, 30, 255};
    redStyle.textColor  = {255, 255, 255, 255};
    redStyle.paddingX   = 20;
    redStyle.paddingY   = 10;
        
    // Anchored Exit Button
    if (XenUI::Button("exit_btn",
                      "Exit (Anchored)",
                      XenUI::PositionParams::Anchored(XenUI::Anchor::BOTTOM_CENTER, 0, -30),
                                            renderer,
                      {0.0f, 0.0f},
                      redStyle,
                      50))
    {
        std::cout << "Exit Button Pressed\n";
        exit(0);
    }

    // 6) Example red test rectangle (SDL3 uses SDL_RenderFillRectF for SDL_FRect)
    // --- ADDITIONS START ---
    // Immediate-mode RadioGroup
    std::vector<std::string> immediateOptions = {"Yes", "No", "Maybe"};
    RadioButtonStyle immediateRadioStyle;
    immediateRadioStyle.circleColor = {200, 100, 200, 255}; // Purple-ish
    immediateRadioStyle.selectedColor = {100, 50, 100, 255};
    immediateRadioStyle.labelColor = {255, 255, 255, 255};
    immediateRadioStyle.circleRadius = 8;
    immediateRadioStyle.circlePadding = 5;
    immediateRadioStyle.innerCirclePadding = 3;

    if (XenUI::RadioGroupImmediate(
            "my_immediate_radio_group",
            immediateOptions,
            &immediateSelectedOption,
            XenUI::PositionParams::Anchored(XenUI::Anchor::TOP_LEFT, 50, 450), // Position the group
            immediateRadioStyle,
            18, // Font size for immediate
            30  // Spacing between options
        )) {
        // This block executes IF the selected option changed this frame
        std::cout << "Immediate Radio: Selection changed to option index: " << immediateSelectedOption << std::endl;
        // You can update other UI elements based on immediateSelectedOption here
    }

    // Display the current immediate mode selection
    XenUI::Label("Immediate Selection: " + immediateOptions[immediateSelectedOption],
                 XenUI::PositionParams::Anchored(XenUI::Anchor::TOP_LEFT, 200, 450),
                 20,
                 SDL_Color{255, 255, 0, 255});
    // --- ADDITIONS END ---

    // Immediate‑mode checkbox example
        if (XenUI::Checkbox(
            "chk_immediate",
         "Show FPS Counter",
         &immediateChecked,
            XenUI::PositionParams::Absolute(400, 250)))
        {
        // toggled this frame
        std::cout << "Immediate checkbox is now " 
                 << (immediateChecked ? "on\n" : "off\n");
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
// immediateSwitchStyle.drawBothLabels = true;

if (XenUI::SwitchImmediate(
        "immediate_switch_1",  // Unique ID
        // textRenderer,
        XenUI::PositionParams::Absolute(100, 400),  // Position at (100, 400 адаптировано)
        immediateSwitchStyle,
        &immediateSwitchState,  // State variable (updated automatically)
        false  // Trigger on release
    
    )) {
    std::cout << "Immediate Switch is now " << (immediateSwitchState ? "ON" : "OFF") << std::endl;
}
// Your app's loop or frame
static bool immediateOn = false;  // Your state
if (XenUI::SwitchImmediate("unique_id", XenUI::PositionParams::Absolute(100, 450), immediateSwitchStyle, &immediateOn)) {
    printf("Immediate Switch is now %s\n", immediateOn ? "ON" : "OFF");
    // React to change here if needed
}


// --- SLIDER ADDITIONS START ---
    // Immediate-mode slider

    
    SliderStyle immediateSliderStyle;
    immediateSliderStyle.trackColor = {40, 40, 40, 255};
    immediateSliderStyle.thumbColor = {255, 100, 0, 255}; // Orange thumb
    immediateSliderStyle.thumbHoverColor = {255, 150, 50, 255};
    immediateSliderStyle.trackThickness = 12;
    immediateSliderStyle.thumbSize = 28;
    immediateSliderStyle.valueTextFontSize = 18;
    
    if (XenUI::Slider(
            "immediateAudioSlider1", // Unique ID
            XenUI::Orientation::HORIZONTAL,
            XenUI::PositionParams::Anchored(XenUI::Anchor::BOTTOM_LEFT, 50, -50), // 50px right from bottom-left, 50px up
            350.0f, // Length: 350 pixels wide
            &Level, 0.0f, 1.0f, // Value range 0.0 to 1.0
            immediateSliderStyle
        )) {
        // This block executes ONLY if the slider's value was adjusted this frame
        std::cout << "Immediate Audio Level: " << Level << std::endl;
    }

    std::string audioText = "Audio Level: " + std::to_string(Level);
    XenUI::Label(audioText,
                 XenUI::PositionParams::Anchored(XenUI::Anchor::BOTTOM_LEFT, 50, -80),
                 20,
                 SDL_Color{255, 255, 255, 255});
    if (XenUI::Slider(
            "immediateAudioSlider2", // Unique ID
            XenUI::Orientation::HORIZONTAL,
            XenUI::PositionParams::Anchored(XenUI::Anchor::BOTTOM_LEFT, 50, -100), // 50px right from bottom-left, 50px up
            350.0f, // Length: 350 pixels wide
            &Level3, 0.0f, 1.0f, // Value range 0.0 to 1.0
            immediateSliderStyle
        )) {
        // This block executes ONLY if the slider's value was adjusted this frame
        std::cout << "Immediate Audio Level: " << Level3 << std::endl;
    }

   // std::string audioText = "Audio Level: " + std::to_string(Level);
    XenUI::Label(audioText,
                 XenUI::PositionParams::Anchored(XenUI::Anchor::BOTTOM_LEFT, 50, -80),
                 20,
                 SDL_Color{255, 255, 255, 255});
    // // --- SLIDER ADDITIONS END ---



    // --- DROPDOWN ADDITIONS START ---
    // Immediate-mode Dropdown: Font Selector
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
            "immediateFontSelector", // Unique ID
            XenUI::PositionParams::Anchored(XenUI::Anchor::TOP_RIGHT, -50, 100), // Position
            180.0f, // Width
            fonts,
            &immediateSelectedFontIndex, // Pointer to the variable to modify
            fontDropdownStyle
        )) {
        // This block executes ONLY if the dropdown's selection changed this frame
        std::cout << "Immediate Font Selection: " << fonts[immediateSelectedFontIndex] << std::endl;
        // Apply font change here (e.g., textRenderer.loadFont(fonts[immediateSelectedFontIndex], ...))
    }
    // --- DROPDOWN ADDITIONS END ---


    XenUI::ScrollViewStyle style;
style.bgColor               = {30, 30,  60, 255};
style.borderColor           = {80, 80, 110, 255};
style.scrollbarBgColor      = {20, 20,  20, 255};
style.scrollbarThumbColor   = {100,100,100,255};
style.scrollbarThumbHoverColor  = {140,140,140,255};
style.scrollbarThumbGrabbedColor = {180,180,180,255};
style.scrollbarWidth        = 14;
 // In your render loop, per‐frame:
// In your render loop, per-frame:

//Position params for immedate mode scrollview 
XenUI::PositionParams p = XenUI::PositionParams::Anchored(XenUI::Anchor::CENTER, 0, 0, 400, 300);
SDL_FRect   view    = { 1000, 50, 250, 400 };
SDL_FPoint content = { 0, 2000 };
static SDL_FPoint scroll;  // persistent scroll state

int viewW = static_cast<int>(view.w);
int viewH = static_cast<int>(view.h);
// Pass current SDL_Event into BeginScrollView:
SDL_FPoint ofs = XenUI::BeginScrollView("settings_list", p, viewW, viewH, content, renderer, evt, style);

int contentClipW = (content.y > viewH) ? (viewW - style.scrollbarWidth) : viewW;
int contentClipH = viewH;

// Now draw all your immediate-mode widgets with that viewOffset:
for (int i = 0; i < numItems; ++i) {
    // e.g. a button or label positioned inside the scrolling area:
    XenUI::Button(
      "btn"+std::to_string(i),
      "Item "+std::to_string(i),
      XenUI::PositionParams::Absolute(int(10 + ofs.x),
                                      int(10 + i*50 + ofs.y)),
      renderer,

      {0,0},
      blackstyle,
      18,
      contentClipW, 
      contentClipH
    );
}
XenUI::Button("ok_btnn",
                      "Add",
                      XenUI::PositionParams::Anchored(XenUI::Anchor::TOP_CENTER, ofs.x, ofs.y),
                      renderer,
                      {0.0f, 0.0f},
                      blackstyle,
                      30,
                      true,
                    contentClipW, 
                    contentClipH); // triggerOnPress = true
    

// Draw an image inside the scroll view using the new helper function
// and passing the view offset.
XenUI::DrawImage(
    "scroll_image", // unique cache key for the image
    renderer,
    "Images/Image.png", // path to your image
    XenUI::PositionParams::Absolute(10, int(10 + numItems * 50 + 60 + 30)),
    ofs, // The view offset is crucial for scrolling
    120.0f, // desired width
    80.0f // desired height
);

if (XenUI::Checkbox("imm_chk_1", "ImOption 1", &toggled1,
    XenUI::PositionParams::Absolute(10, int(10 + numItems*50 + 30)),
    CheckboxStyle{}, 20, ofs))
{
    std::cout << "Immediate Option 1 is now " << (toggled1 ? "On\n" : "Off\n");
}

if (XenUI::Checkbox("imm_chk_2", "ImOption 2", &toggled2,
    XenUI::PositionParams::Absolute(10, int(10 + numItems*50 +30+30)),
    CheckboxStyle{}, 20, ofs))
{
    std::cout << "Immediate Option 2 is now " << (toggled2 ? "On\n" : "Off\n");
}




    //immediate label in scrollview area 
    XenUI::Label("Immediate label and more",
                 XenUI::PositionParams::Anchored(XenUI::Anchor::CENTER),
                 30,
                 SDL_Color{200, 200, 50, 255},
                 contentClipW, 
                 contentClipH,
                ofs);

        if (XenUI::RadioGroupImmediate(
            "my_immediate_radio_group",
            immediateOptions,
            &immediateSelectedOption,
            XenUI::PositionParams::Absolute(10, int(10 + numItems * 50 + 200 + 50)), // Position the group
            immediateRadioStyle,
            18, // Font size for immediate
            30,  // Spacing between options
            ofs
        )) {
        // This block executes IF the selected option changed this frame
        std::cout << "Immediate Radio: Selection changed to option index: " << immediateSelectedOption << std::endl;
        // You can update other UI elements based on immediateSelectedOption here
    }



SliderStyle immediateSliderStyle1;
    immediateSliderStyle1.trackColor = {40, 40, 40, 255};
    immediateSliderStyle1.thumbColor = {255, 100, 0, 255}; // Orange thumb
    immediateSliderStyle1.thumbHoverColor = {255, 150, 50, 255};
    immediateSliderStyle1.trackThickness = 12;
    immediateSliderStyle1.thumbSize = 28;
    immediateSliderStyle1.valueTextFontSize = 18;
    
    if (XenUI::Slider(
            "immediateAudioSlider3", // Unique ID
            XenUI::Orientation::HORIZONTAL,
            XenUI::PositionParams::Absolute(10, int(10 + numItems * 50 + 200 + 50+50+50)), // 50px right from bottom-left, 50px up
            350.0f, // Length: 350 pixels wide
            &Level4, 0.0f, 1.0f, // Value range 0.0 to 1.0
            immediateSliderStyle1, 
            ofs
        )) {
        // This block executes ONLY if the slider's value was adjusted this frame
        std::cout << "Immediate Audio Level: " << Level4 << std::endl;
    }

// SliderStyle immediateSliderStyle1;
//     immediateSliderStyle1.trackColor = {40, 40, 40, 255};
//     immediateSliderStyle1.thumbColor = {255, 100, 0, 255}; // Orange thumb
//     immediateSliderStyle1.thumbHoverColor = {255, 150, 50, 255};
//     immediateSliderStyle1.trackThickness = 12;
//     immediateSliderStyle1.thumbSize = 28;
//     immediateSliderStyle1.valueTextFontSize = 18;
    
    if (XenUI::Slider(
            "immediateAudioSlider4", // Unique ID
            XenUI::Orientation::HORIZONTAL,
            XenUI::PositionParams::Absolute(10, int(10 + numItems * 50 + 200 + 50+50+50+100)), // 50px right from bottom-left, 50px up
            350.0f, // Length: 350 pixels wide
            &Level1, 0.0f, 1.0f, // Value range 0.0 to 1.0
            immediateSliderStyle1, 
            ofs
        )) {
        // This block executes ONLY if the slider's value was adjusted this frame
        std::cout << "Immediate Audio Level: " << Level1 << std::endl;
    }

// int contentClipW = (content.y > viewH) ? (viewW - style.scrollbarWidth) : viewW;
// int contentClipH = viewH;

    if (XenUI::SwitchImmediate(
        "immediate_switch_2",  // Unique ID
        // textRenderer,
        XenUI::PositionParams::Anchored(XenUI::Anchor::TOP_RIGHT),  // Position at (100, 400 адаптировано)
        immediateSwitchStyle,
        &immediateSwitchState1,  // State variable (updated automatically)
        false , // Trigger on release
        contentClipW,     // parentWidth — important for anchors
        contentClipH, 
        ofs
    )) {
    std::cout << "Immediate Switch is now " << (immediateSwitchState1 ? "ON" : "OFF") << std::endl;
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
    // First, let the scroll‐view get first crack. If it handles the event,
    // we redraw and skip other widgets this frame.
    // if (myRetainedScrollView && myRetainedScrollView->handleEvent(event)) {
    //     needsRedraw = true;
    //     break;  // Skip other event handling for this event
    // }
// Pass 'window' into the ScrollView so it can forward it to children.
// If your ScrollView::handleEvent signature is (const SDL_Event&), change it
// to: bool handleEvent(const SDL_Event&, SDL_Window*, const SDL_FPoint& viewOffset);
// and forward the viewOffset you draw with (here we use {0,0} at top-level).
// NEW: pass the SDL_Window* and top-level viewOffset {0,0}
if (myRetainedScrollView && myRetainedScrollView->handleEvent(event, window, {0.0f, 0.0f})) {
    needsRedraw = true;
    break;
}



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




/* Game Mode
    best for game loops 



bool running       = true;
SDL_Event event;

// For precise timing (60 FPS cap)
constexpr float TARGET_FPS = 60.0f;
constexpr Uint32 FRAME_MS  = static_cast<Uint32>(1000.0f / TARGET_FPS);

Uint64 lastCounter = SDL_GetPerformanceCounter();
double invFreq     = 1.0 / static_cast<double>(SDL_GetPerformanceFrequency());

while (running) {
    // 1) Poll all pending events
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            running = false;
            break;
        }

        // Handle desktop fullscreen toggle
        if (event.type == SDL_EVENT_KEY_DOWN &&
            event.key.key == SDLK_F11) {
            Uint32 flags = SDL_GetWindowFlags(window);
            bool isFull = (flags & SDL_WINDOW_FULLSCREEN) != 0;
            SDL_SetWindowFullscreen(
                window,
                isFull ? 0 : SDL_WINDOW_FULLSCREEN
            );
        }

        // Recalc positions on window resize
        if (event.type == SDL_EVENT_WINDOW_RESIZED) {
            for (auto& btn : buttons) btn.recalculatePosition();
            for (auto& lbl : labels)  lbl.recalculatePosition();
            for (auto& box : Inputs)  box.recalculatePosition();
        }

        // Pass events to widgets (buttons, input boxes, etc.)
        for (auto& btn : buttons) {
            btn.handleEvent(event);
        }
        for (auto& box : Inputs) {
            box.handleEvent(event, window);
        }
    }

    // 2) Compute deltaTime
    Uint64 nowCounter = SDL_GetPerformanceCounter();
    double deltaSec = (nowCounter - lastCounter) * invFreq;
    lastCounter = nowCounter;

    // 3) Update all widgets and game logic
    for (auto& box : Inputs) {
        box.update(static_cast<float>(deltaSec));
    }
    // ... update button animations, moving sprites, physics, etc. ...

    // 4) Always render each frame
    render(renderer);

    // 5) Cap to ~60 FPS
    Uint64 frameCounterDiff = SDL_GetPerformanceCounter() - nowCounter;
    Uint32 frameTimeMs =
        static_cast<Uint32>(frameCounterDiff * invFreq * 1000.0);
    if (frameTimeMs < FRAME_MS) {
        SDL_Delay(FRAME_MS - frameTimeMs);
    }
}

// Cleanup (unchanged)
textRenderer.clearCache();
if (gImage) { delete gImage; gImage = nullptr; }
TTF_Quit();
SDL_DestroyRenderer(renderer);
SDL_DestroyWindow(window);
SDL_Quit();

*/



/* Idle Mode
    best for idle apps, forms, text etc 



bool running     = true;
SDL_Event event;

// “Dirty” flag: set to true whenever UI state changes
bool needsRedraw = true;

// Track time to update widgets (e.g. caret blink) even if no user input
auto lastTime    = std::chrono::high_resolution_clock::now();

while (running) {
    // 1) Wait for an event (or time out after 500 ms for widget updates)
    if (SDL_WaitEventTimeout(&event, 500)) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
                running = false;
                break;

            case SDL_EVENT_KEY_DOWN:
                if (event.key.key == SDLK_F11) {
                    // Toggle fullscreen on desktop
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
                // Recalculate positions for anchored widgets
                for (auto& btn : buttons) btn.recalculatePosition();
                for (auto& lbl : labels)  lbl.recalculatePosition();
                for (auto& box : Inputs)  box.recalculatePosition();
                needsRedraw = true;
                break;

            default:
                // Pass remaining events to retained‐mode widgets
                for (auto& btn : buttons) {
                    if (btn.handleEvent(event)) {
                        needsRedraw = true;
                    }
                }
                break;
        }

        // Always let input boxes process events (text input, caret, etc.)
        for (auto& box : Inputs) {
            if (box.handleEvent(event, window)) {
                needsRedraw = true;
            }
        }

        if (event.type == SDL_EVENT_TEXT_EDITING) {
            SDL_Log(
                "App: SDL_EVENT_TEXT_EDITING --- text: '%s', start: %d, length: %d",
                event.edit.text ? event.edit.text : "NULL",
                event.edit.start,
                event.edit.length
            );
        }

        // Something happened, ensure we’ll redraw
        needsRedraw = true;
    }
    // If SDL_WaitEventTimeout timed out (no events in 500 ms), fall through to updates

    // 2) Compute deltaTime and update widgets (caret blink, short animations)
    {
        auto now = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> dt = now - lastTime;
        lastTime = now;
        float deltaTime = dt.count();

        // Update input boxes; if update() returns true, widget state changed
        for (auto& box : Inputs) {
            if (box.update(deltaTime)) {
                needsRedraw = true;
            }
        }

        // ... you could update button‐hover animations or other short UI animations here ...
    }

    // 3) Only redraw when needed
    if (needsRedraw) {
        render(renderer);
        needsRedraw = false;
    }
    // Otherwise, loop back into SDL_WaitEventTimeout and let the CPU sleep
}

// Cleanup (unchanged)
textRenderer.clearCache();
if (gImage) { delete gImage; gImage = nullptr; }
TTF_Quit();
SDL_DestroyRenderer(renderer);
SDL_DestroyWindow(window);
SDL_Quit();

*/



/* Fast FPS loop

    //it is not ideal for idle apps, because it usages more battery power. but can be used in fast rendering app, like game



    // bool running = true;
    // SDL_Event event;
    // while (running) {
    //     while (SDL_PollEvent(&event)) {
    //         switch (event.type) {
    //             case SDL_EVENT_QUIT:
    //                 running = false;
    //                 break;

    //             case SDL_EVENT_KEY_DOWN:
    //                 if (event.key.key == SDLK_F11) {
    //                     // Toggle fullscreen (desktop)
    //                     Uint32 flags = SDL_GetWindowFlags(window);
    //                     bool is_fullscreen = (flags & SDL_WINDOW_FULLSCREEN) != 0;
    //                     SDL_SetWindowFullscreen(window,
    //                                             is_fullscreen ? 0 : SDL_WINDOW_FULLSCREEN);
    //                     std::cout << (is_fullscreen
    //                                       ? "Exited fullscreen mode.\n"
    //                                       : "Entered fullscreen (desktop) mode.\n");
    //                 }
    //                 break;

    //             case SDL_EVENT_WINDOW_RESIZED:
    //                 // Recalculate positions for anchored widgets
    //                 for (auto& btn : buttons)   btn.recalculatePosition();
    //                 for (auto& lbl : labels)    lbl.recalculatePosition();
    //                 for (auto& box : Inputs)    box.recalculatePosition();
    //                 break;

    //             default:
    //                 // Pass all other events to retained-mode widgets
    //                 for (auto& btn : buttons) btn.handleEvent(event);
    //                 break;
    //         }

    //         for (auto& box : Inputs) {
    //             box.handleEvent(event, window);
    //         }

    //         if (event.type == SDL_EVENT_TEXT_EDITING) {
    //             SDL_Log("App: SDL_EVENT_TEXT_EDITING --- text: '%s', start: %d, length: %d",
    //                     event.edit.text ? event.edit.text : "NULL",
    //                     event.edit.start,
    //                     event.edit.length);
    //         }
    //     }

    //     // Delta-time for updates
    //     static auto lastTime = std::chrono::high_resolution_clock::now();
    //     auto now      = std::chrono::high_resolution_clock::now();
    //     std::chrono::duration<float> dt = now - lastTime;
    //     lastTime = now;
    //     float deltaTime = dt.count();

    //     // Update input boxes
    //     for (auto& box : Inputs) {
    //         box.update(deltaTime);
    //     }

    //     // Render everything
    //     render(renderer);

    //     // Cap ~60 FPS
    //     SDL_Delay(16);
    // }

    // // Cleanup
    // textRenderer.clearCache();
    // if (gImage) {
    //     delete gImage;
    //     gImage = nullptr;
    // }
    // TTF_Quit();        // Shutdown SDL_ttf
    // SDL_DestroyRenderer(renderer);
    // SDL_DestroyWindow(window);
    // SDL_Quit();

    // return 0;
}

*/




