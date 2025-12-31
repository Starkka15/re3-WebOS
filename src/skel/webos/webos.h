#pragma once

// WebOS TouchPad Platform-Specific Header
// For use with HP webOS PDK

#ifdef WEBOS_TOUCHPAD

#include <SDL/SDL.h>
#include <pdl.h>

// Platform-specific structure for WebOS
typedef struct
{
    SDL_Surface* screen;
    RwBool fullScreen;
    bool touchActive;
    RwV2d touchPos;
    RwV2d lastTouchPos;
    RwInt8 joy1id;
    RwUInt32 padButtonState;
    RwUInt32 oldPadButtonState;
} psGlobalTypeWebOS;

// Global accessor macro
#define PSGLOBAL(var) (((psGlobalTypeWebOS *)(RsGlobal.ps))->var)

// WebOS screen dimensions (TouchPad native resolution)
#define WEBOS_SCREEN_WIDTH 1024
#define WEBOS_SCREEN_HEIGHT 768

// Virtual button mapping
enum WebOSVirtualButton
{
    WEBOS_BTN_NONE = 0,
    WEBOS_BTN_FIRE = 1,
    WEBOS_BTN_JUMP = 2,
    WEBOS_BTN_SPRINT = 4,
    WEBOS_BTN_CROUCH = 8,
    WEBOS_BTN_ENTER_EXIT = 16,
    WEBOS_BTN_CAMERA = 32,
    WEBOS_BTN_TARGET = 64,
    WEBOS_BTN_MENU = 128
};

// Virtual stick zones
#define WEBOS_LEFT_STICK_X_MIN 0
#define WEBOS_LEFT_STICK_X_MAX 350
#define WEBOS_RIGHT_STICK_X_MIN 674
#define WEBOS_RIGHT_STICK_X_MAX 1024
#define WEBOS_ACTION_BUTTON_Y_MIN 450

// Function declarations
void HandleTouchInput(SDL_Event* event);
void ProcessVirtualControls();
void RenderVirtualControls();
RwUInt32 DetectVirtualButton(int x, int y);

#endif // WEBOS_TOUCHPAD
