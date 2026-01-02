#if defined(WEBOS_TOUCHPAD)

// WebOS TouchPad Touch Input Handler
// Maps touchscreen input to game controls

#include <SDL/SDL.h>
#include <math.h>

#include "common.h"
#include "platform.h"
#include "Pad.h"
#include "webos.h"
#include "Sprite2d.h"

// Virtual analog stick dead zone
#define STICK_DEADZONE 0.15f
#define STICK_RADIUS 100.0f

// Touch state tracking
struct TouchState {
    bool active;
    int startX, startY;
    int currentX, currentY;
    float normalizedX, normalizedY; // -1 to 1
};

static TouchState leftStick = {false, 0, 0, 0, 0, 0.0f, 0.0f};
static TouchState rightStick = {false, 0, 0, 0, 0, 0.0f, 0.0f};
static RwUInt32 buttonState = 0;
static RwUInt32 pendingRelease = 0;  // Buttons awaiting delayed release
static Uint32 buttonPressTime[8] = {0};  // Timestamp for each button (bit 0-7)

/*
 * Calculate normalized stick position from touch coordinates
 */
void CalculateStickPosition(TouchState* stick)
{
    if (!stick->active) {
        stick->normalizedX = 0.0f;
        stick->normalizedY = 0.0f;
        return;
    }

    // Calculate delta from start position
    float dx = (float)(stick->currentX - stick->startX);
    float dy = (float)(stick->currentY - stick->startY);

    // Calculate distance and angle
    float distance = sqrtf(dx * dx + dy * dy);
    if (distance < STICK_DEADZONE * STICK_RADIUS) {
        stick->normalizedX = 0.0f;
        stick->normalizedY = 0.0f;
        return;
    }

    // Normalize to -1 to 1 range
    stick->normalizedX = dx / STICK_RADIUS;
    stick->normalizedY = dy / STICK_RADIUS;

    // Clamp to unit circle
    if (stick->normalizedX > 1.0f) stick->normalizedX = 1.0f;
    if (stick->normalizedX < -1.0f) stick->normalizedX = -1.0f;
    if (stick->normalizedY > 1.0f) stick->normalizedY = 1.0f;
    if (stick->normalizedY < -1.0f) stick->normalizedY = -1.0f;
}

/*
 * Detect which virtual button was pressed based on touch position
 */
RwUInt32 DetectVirtualButton(int x, int y)
{
    // Action button zone (center bottom)
    if (y > WEBOS_ACTION_BUTTON_Y_MIN && x > WEBOS_LEFT_STICK_X_MAX && x < WEBOS_RIGHT_STICK_X_MIN) {
        // Divide into button regions
        int buttonWidth = (WEBOS_RIGHT_STICK_X_MIN - WEBOS_LEFT_STICK_X_MAX) / 4;
        int relX = x - WEBOS_LEFT_STICK_X_MAX;

        if (relX < buttonWidth) {
            return WEBOS_BTN_JUMP;
        } else if (relX < buttonWidth * 2) {
            return WEBOS_BTN_FIRE;
        } else if (relX < buttonWidth * 3) {
            return WEBOS_BTN_SPRINT;
        } else {
            return WEBOS_BTN_ENTER_EXIT;
        }
    }

    // Top left corner - Menu
    if (x < 100 && y < 100) {
        return WEBOS_BTN_MENU;
    }

    // Top right corner - Camera change
    if (x > WEBOS_SCREEN_WIDTH - 100 && y < 100) {
        return WEBOS_BTN_CAMERA;
    }

    return WEBOS_BTN_NONE;
}

/*
 * Handle touch input events from SDL
 */
void HandleTouchInput(SDL_Event* event)
{
    int x, y;

#ifdef WEBOS_TOUCHPAD
    static int touchEventCount = 0;
    static FILE *touchLog = NULL;
    if (touchEventCount < 200) {  // Log first 200 touch events
        touchLog = NULL /* logging disabled */;
    }
#endif

    switch (event->type) {
        case SDL_MOUSEBUTTONDOWN: {
            x = event->button.x;
            y = event->button.y;

#ifdef WEBOS_TOUCHPAD
            if (touchEventCount++ < 200 && touchLog) {
                fprintf(touchLog, "Touch DOWN at (%d, %d)", x, y);

                // Check button detection zones
                if (y > WEBOS_ACTION_BUTTON_Y_MIN && x > WEBOS_LEFT_STICK_X_MAX && x < WEBOS_RIGHT_STICK_X_MIN) {
                    fprintf(touchLog, " -> IN BUTTON ZONE");
                    RwUInt32 detectedButton = DetectVirtualButton(x, y);
                    fprintf(touchLog, " -> Detected button: 0x%X", detectedButton);
                }
                fprintf(touchLog, "\n");
                fflush(touchLog);
            }
#endif

            // Check for buttons FIRST (before sticks) to prevent overlap issues
            RwUInt32 button = DetectVirtualButton(x, y);
            if (button != WEBOS_BTN_NONE) {
                buttonState |= button;
                // Record press time for this button
                for (int i = 0; i < 8; i++) {
                    if (button & (1 << i)) {
                        buttonPressTime[i] = SDL_GetTicks();
                    }
                }
            }
            // Left stick zone (only if not a button)
            else if (x >= WEBOS_LEFT_STICK_X_MIN && x <= WEBOS_LEFT_STICK_X_MAX) {
                leftStick.active = true;
                leftStick.startX = x;
                leftStick.startY = y;
                leftStick.currentX = x;
                leftStick.currentY = y;
            }
            // Right stick zone
            else if (x >= WEBOS_RIGHT_STICK_X_MIN && x <= WEBOS_RIGHT_STICK_X_MAX) {
                rightStick.active = true;
                rightStick.startX = x;
                rightStick.startY = y;
                rightStick.currentX = x;
                rightStick.currentY = y;
            }
            break;
        }

        case SDL_MOUSEBUTTONUP:
            x = event->button.x;
            y = event->button.y;

            // Check which stick was released
            if (leftStick.active && x >= WEBOS_LEFT_STICK_X_MIN && x <= WEBOS_LEFT_STICK_X_MAX) {
                leftStick.active = false;
                leftStick.normalizedX = 0.0f;
                leftStick.normalizedY = 0.0f;
            } else if (rightStick.active && x >= WEBOS_RIGHT_STICK_X_MIN && x <= WEBOS_RIGHT_STICK_X_MAX) {
                rightStick.active = false;
                rightStick.normalizedX = 0.0f;
                rightStick.normalizedY = 0.0f;
            } else {
                // Mark button for delayed release (don't clear immediately)
                RwUInt32 button = DetectVirtualButton(x, y);
                if (button != WEBOS_BTN_NONE) {
                    pendingRelease |= button;

#ifdef WEBOS_TOUCHPAD
                    FILE *log = NULL; // fopen("/media/internal/.gta3/debug.log", "a");
                    if (log) {
                        fprintf(log, "Touch UP: button 0x%X marked for delayed release\n", button);
                        fflush(log);
                        fclose(log);
                    }
#endif
                }
            }
            break;

        case SDL_MOUSEMOTION:
            x = event->motion.x;
            y = event->motion.y;

            // Update active stick positions
            if (leftStick.active) {
                leftStick.currentX = x;
                leftStick.currentY = y;
                CalculateStickPosition(&leftStick);
            }
            if (rightStick.active) {
                rightStick.currentX = x;
                rightStick.currentY = y;
                CalculateStickPosition(&rightStick);
            }
            break;
    }

#ifdef WEBOS_TOUCHPAD
    if (touchLog) {
        fclose(touchLog);
        touchLog = NULL;
    }
#endif
}

/*
 * Process virtual controls and update game pad state
 */
void ProcessVirtualControls()
{
    // Get the main game pad
    CPad* pad = CPad::GetPad(0);
    if (!pad) return;

    // Process delayed button releases (minimum 50ms hold time)
    if (pendingRelease != 0) {
        Uint32 currentTime = SDL_GetTicks();
        for (int i = 0; i < 8; i++) {
            RwUInt32 buttonBit = (1 << i);
            if (pendingRelease & buttonBit) {
                // Check if button has been held for at least 50ms
                if (currentTime - buttonPressTime[i] >= 50) {
                    buttonState &= ~buttonBit;
                    pendingRelease &= ~buttonBit;

#ifdef WEBOS_TOUCHPAD
                    FILE *log = NULL; // fopen("/media/internal/.gta3/debug.log", "a");
                    if (log) {
                        fprintf(log, "ProcessVirtualControls: Released button 0x%X after %u ms\n",
                            buttonBit, currentTime - buttonPressTime[i]);
                        fflush(log);
                        fclose(log);
                    }
#endif
                }
            }
        }
    }

#ifdef WEBOS_TOUCHPAD
    static int controlLogCount = 0;
    static RwUInt32 lastButtonState = 0;

    // Log when buttonState changes or every 60 frames
    if (buttonState != lastButtonState || (controlLogCount++ % 60 == 0 && buttonState != 0)) {
        FILE *log = NULL; // fopen("/media/internal/.gta3/debug.log", "a");
        if (log) {
            fprintf(log, "ProcessVirtualControls: buttonState=0x%X (was 0x%X), leftStick(%d,%d), rightStick(%d,%d)\n",
                buttonState, lastButtonState,
                leftStick.active ? 1 : 0, leftStick.active ? (int)(leftStick.normalizedX * 100) : 0,
                rightStick.active ? 1 : 0, rightStick.active ? (int)(rightStick.normalizedX * 100) : 0);
            fflush(log);
            fclose(log);
        }
        lastButtonState = buttonState;
    }
#endif

    // Update stick positions
    CalculateStickPosition(&leftStick);
    CalculateStickPosition(&rightStick);

    // Left stick controls movement
    if (leftStick.active) {
        // Map to pad axes (-128 to 127)
        pad->NewState.LeftStickX = (short)(leftStick.normalizedX * 127.0f);
        pad->NewState.LeftStickY = (short)(-leftStick.normalizedY * 127.0f); // Invert Y

        // Also emulate DPad from stick for menu navigation
        pad->NewState.DPadUp = (leftStick.normalizedY < -0.5f) ? 255 : 0;
        pad->NewState.DPadDown = (leftStick.normalizedY > 0.5f) ? 255 : 0;
        pad->NewState.DPadLeft = (leftStick.normalizedX < -0.5f) ? 255 : 0;
        pad->NewState.DPadRight = (leftStick.normalizedX > 0.5f) ? 255 : 0;
    } else {
        pad->NewState.LeftStickX = 0;
        pad->NewState.LeftStickY = 0;
        pad->NewState.DPadUp = 0;
        pad->NewState.DPadDown = 0;
        pad->NewState.DPadLeft = 0;
        pad->NewState.DPadRight = 0;
    }

    // Right stick controls camera
    if (rightStick.active) {
        pad->NewState.RightStickX = (short)(rightStick.normalizedX * 127.0f);
        pad->NewState.RightStickY = (short)(-rightStick.normalizedY * 127.0f); // Invert Y
    } else {
        pad->NewState.RightStickX = 0;
        pad->NewState.RightStickY = 0;
    }

    // Map virtual buttons to pad buttons
    pad->NewState.Cross = (buttonState & WEBOS_BTN_FIRE) ? 255 : 0;
    pad->NewState.Square = (buttonState & WEBOS_BTN_JUMP) ? 255 : 0;
    pad->NewState.Circle = (buttonState & WEBOS_BTN_SPRINT) ? 255 : 0;
    pad->NewState.Triangle = (buttonState & WEBOS_BTN_ENTER_EXIT) ? 255 : 0;
    pad->NewState.Select = (buttonState & WEBOS_BTN_CAMERA) ? 255 : 0;
    pad->NewState.Start = (buttonState & WEBOS_BTN_MENU) ? 255 : 0;

#ifdef WEBOS_TOUCHPAD
    static int padLogCount = 0;
    if(padLogCount++ % 30 == 0 && (pad->NewState.DPadUp || pad->NewState.DPadDown || pad->NewState.Cross)) {
        FILE *log = NULL; // fopen("/media/internal/.gta3/debug.log", "a");
        if(log) {
            fprintf(log, "ProcessVirtualControls: DPadUp=%d, DPadDown=%d, Cross=%d, OldCross=%d\n",
                pad->NewState.DPadUp, pad->NewState.DPadDown, pad->NewState.Cross, pad->OldState.Cross);
            fflush(log);
            fclose(log);
        }
    }
#endif

    // L/R buttons (could be mapped to additional touch zones)
    pad->NewState.LeftShoulder1 = (buttonState & WEBOS_BTN_TARGET) ? 255 : 0;
    pad->NewState.RightShoulder1 = (buttonState & WEBOS_BTN_CROUCH) ? 255 : 0;
}

/*
 * Render virtual controls overlay (optional - for visual feedback)
 */
void RenderVirtualControls()
{
    // Draw semi-transparent overlay for virtual controls

    // Left stick area - draw circle
    if (leftStick.active) {
        // Draw stick base (green when active)
        CSprite2d::DrawRect(CRect(leftStick.startX - 80, leftStick.startY - 80,
                                  leftStick.startX + 80, leftStick.startY + 80),
                           CRGBA(0, 255, 0, 80));
        // Draw stick position
        CSprite2d::DrawRect(CRect(leftStick.currentX - 30, leftStick.currentY - 30,
                                  leftStick.currentX + 30, leftStick.currentY + 30),
                           CRGBA(0, 255, 0, 150));
    } else {
        // Draw stick base (gray when inactive)
        CSprite2d::DrawRect(CRect(50, WEBOS_SCREEN_HEIGHT - 200, 210, WEBOS_SCREEN_HEIGHT - 40),
                           CRGBA(128, 128, 128, 60));
    }

    // Right stick area
    if (rightStick.active) {
        CSprite2d::DrawRect(CRect(rightStick.startX - 80, rightStick.startY - 80,
                                  rightStick.startX + 80, rightStick.startY + 80),
                           CRGBA(0, 200, 255, 80));
        CSprite2d::DrawRect(CRect(rightStick.currentX - 30, rightStick.currentY - 30,
                                  rightStick.currentX + 30, rightStick.currentY + 30),
                           CRGBA(0, 200, 255, 150));
    } else {
        CSprite2d::DrawRect(CRect(WEBOS_SCREEN_WIDTH - 210, WEBOS_SCREEN_HEIGHT - 200,
                                  WEBOS_SCREEN_WIDTH - 50, WEBOS_SCREEN_HEIGHT - 40),
                           CRGBA(128, 128, 128, 60));
    }

    // Action buttons in center
    int buttonY = WEBOS_SCREEN_HEIGHT - 150;
    int buttonWidth = (WEBOS_RIGHT_STICK_X_MIN - WEBOS_LEFT_STICK_X_MAX) / 4;
    int baseX = WEBOS_LEFT_STICK_X_MAX + 20;

    // Jump button (Square)
    CRGBA jumpColor = (buttonState & WEBOS_BTN_JUMP) ? CRGBA(255, 255, 0, 150) : CRGBA(200, 200, 0, 60);
    CSprite2d::DrawRect(CRect(baseX, buttonY, baseX + buttonWidth - 10, buttonY + 80), jumpColor);

    // Fire button (Cross) - most important for menu
    CRGBA fireColor = (buttonState & WEBOS_BTN_FIRE) ? CRGBA(255, 0, 0, 150) : CRGBA(255, 0, 0, 80);
    CSprite2d::DrawRect(CRect(baseX + buttonWidth, buttonY, baseX + buttonWidth * 2 - 10, buttonY + 80), fireColor);

    // Sprint button (Circle)
    CRGBA sprintColor = (buttonState & WEBOS_BTN_SPRINT) ? CRGBA(0, 255, 0, 150) : CRGBA(0, 200, 0, 60);
    CSprite2d::DrawRect(CRect(baseX + buttonWidth * 2, buttonY, baseX + buttonWidth * 3 - 10, buttonY + 80), sprintColor);

    // Enter/Exit button (Triangle)
    CRGBA triangleColor = (buttonState & WEBOS_BTN_ENTER_EXIT) ? CRGBA(0, 150, 255, 150) : CRGBA(0, 100, 200, 60);
    CSprite2d::DrawRect(CRect(baseX + buttonWidth * 3, buttonY, baseX + buttonWidth * 4 - 10, buttonY + 80), triangleColor);

    // Menu button (top left)
    CRGBA menuColor = (buttonState & WEBOS_BTN_MENU) ? CRGBA(255, 255, 255, 150) : CRGBA(200, 200, 200, 60);
    CSprite2d::DrawRect(CRect(10, 10, 100, 100), menuColor);

    // Camera button (top right)
    CRGBA cameraColor = (buttonState & WEBOS_BTN_CAMERA) ? CRGBA(255, 255, 255, 150) : CRGBA(200, 200, 200, 60);
    CSprite2d::DrawRect(CRect(WEBOS_SCREEN_WIDTH - 100, 10, WEBOS_SCREEN_WIDTH - 10, 100), cameraColor);
}

#endif // WEBOS_TOUCHPAD
