#if defined(WEBOS_TOUCHPAD)

// WebOS TouchPad Platform Implementation
// Uses RW_NULL platform with SDL 1.2 and WebOS PDK

#include <SDL/SDL.h>
#include <pdl.h>
#include <GLES2/gl2.h>
#include <errno.h>
#include <locale.h>
#include <signal.h>
#include <stddef.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "common.h"
#include "crossplatform.h"
#include "platform.h"
#include "skeleton.h"
#include "rwcore.h"

#include "main.h"
#include "FileMgr.h"
#include "Text.h"
#include "Pad.h"
#include "Timer.h"
#include "DMAudio.h"
#include "ControllerConfig.h"
#include "Frontend.h"
#include "Game.h"
#include "PCSave.h"
#include "MemoryCard.h"
#include "Sprite2d.h"
#include "AnimViewer.h"
#include "Font.h"
#include "MemoryMgr.h"

#include "webos.h"
#include "sdl2compat.h"

#define MAX_SUBSYSTEMS (16)

rw::EngineOpenParams openParams;

static RwBool ForegroundApp = TRUE;
static RwBool WindowIconified = FALSE;
static RwBool WindowFocused = TRUE;
static RwBool RwInitialised = FALSE;

static RwSubSystemInfo GsubSysInfo[MAX_SUBSYSTEMS];
static RwInt32 GnumSubSystems = 0;
static RwInt32 GcurSel = 0, GcurSelVM = 0;

static psGlobalTypeWebOS PsGlobal;

size_t _dwMemAvailPhys;
RwUInt32 gGameState;

// Windows-specific stubs for webOS
long _dwOperatingSystemVersion = 0;

void HandleExit(void)
{
    // Windows exit handler - not needed on webOS
}


/*
 * Platform initialization
 */
void _psCreateFolder(const char *path)
{
    struct stat info;
    char fullpath[PATH_MAX];
    realpath(path, fullpath);

    if (lstat(fullpath, &info) != 0) {
        if (errno == ENOENT || (errno != EACCES && !S_ISDIR(info.st_mode))) {
            mkdir(fullpath, 0755);
        }
    }
}

/*
 * Get user files folder for saves
 */
const char *_psGetUserFilesFolder()
{
    static char szUserFiles[256];

    // WebOS user files location
    strcpy(szUserFiles, "/media/internal/.gta3/userfiles");
    _psCreateFolder("/media/internal/.gta3");
    _psCreateFolder(szUserFiles);

    return szUserFiles;
}

/*
 * Get available physical memory
 */
size_t getAvailableMemory()
{
    FILE* fp = fopen("/proc/meminfo", "r");
    if (!fp) return 256 * 1024 * 1024; // Default to 256MB

    char line[256];
    size_t memAvailable = 0;

    while (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, "MemAvailable: %zu kB", &memAvailable) == 1) {
            break;
        }
    }
    fclose(fp);

    if (memAvailable == 0) {
        // Fallback: try MemFree
        fp = fopen("/proc/meminfo", "r");
        while (fgets(line, sizeof(line), fp)) {
            if (sscanf(line, "MemFree: %zu kB", &memAvailable) == 1) {
                break;
            }
        }
        fclose(fp);
    }

    // Convert from KB to bytes
    return memAvailable * 1024;
}

/*
 * Platform initialization
 */
RwBool psInitialize(void)
{
    FILE *log = NULL; // fopen("/media/internal/.gta3/debug.log", "a");
    if (log) { fprintf(log, "psInitialize() called\n"); fflush(log); }

    debug("Initializing WebOS platform...\n");

    PsGlobal.screen = nil;
    PsGlobal.fullScreen = TRUE;
    PsGlobal.touchActive = false;
    PsGlobal.touchPos.x = 0.0f;
    PsGlobal.touchPos.y = 0.0f;
    PsGlobal.lastTouchPos.x = 0.0f;
    PsGlobal.lastTouchPos.y = 0.0f;
    PsGlobal.joy1id = -1;
    PsGlobal.padButtonState = 0;
    PsGlobal.oldPadButtonState = 0;

    // Initialize PDL (Palm Device Layer)
    if (log) { fprintf(log, "Calling PDL_Init()...\n"); fflush(log); }
    if (PDL_Init(0) != 0) {
        if (log) { fprintf(log, "ERROR: PDL_Init failed!\n"); fclose(log); }
        debug("PDL_Init failed\n");
        return FALSE;
    }
    if (log) { fprintf(log, "PDL_Init succeeded\n"); fflush(log); }

    // Initialize SDL
    if (log) { fprintf(log, "Calling SDL_Init()...\n"); fflush(log); }
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_AUDIO) < 0) {
        if (log) { fprintf(log, "ERROR: SDL_Init failed: %s\n", SDL_GetError()); fclose(log); }
        debug("SDL_Init failed: %s\n", SDL_GetError());
        return FALSE;
    }
    if (log) { fprintf(log, "SDL_Init succeeded\n"); fflush(log); }

    // Set OpenGL attributes for ES 2.0 (webOS only supports GLES 2.0 contexts)
    // Note: GL1 backend will need to emulate fixed-function or use compatibility extensions
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    // Create OpenGL ES surface - must be done before RenderWare initialization
    // FIX: Use SDL_OPENGL instead of SDL_OPENGLES - SDL_OPENGLES doesn't set the SDL_OPENGL flag
    if (log) { fprintf(log, "Creating SDL video surface with SDL_OPENGL flag...\n"); fflush(log); }
    PsGlobal.screen = SDL_SetVideoMode(
        WEBOS_SCREEN_WIDTH,
        WEBOS_SCREEN_HEIGHT,
        32,
        SDL_OPENGL | SDL_FULLSCREEN
    );

    if (!PsGlobal.screen) {
        if (log) { fprintf(log, "ERROR: SDL_SetVideoMode failed: %s\n", SDL_GetError()); fclose(log); }
        debug("SDL_SetVideoMode failed: %s\n", SDL_GetError());
        return FALSE;
    }
    if (log) {
        fprintf(log, "SDL video surface created successfully\n");
        fprintf(log, "  Surface dimensions: %dx%d\n", PsGlobal.screen->w, PsGlobal.screen->h);
        fprintf(log, "  Surface flags: 0x%08X\n", PsGlobal.screen->flags);
        fprintf(log, "  SDL_OPENGL flag: %s\n", (PsGlobal.screen->flags & SDL_OPENGL) ? "SET" : "NOT SET");
        fprintf(log, "  SDL_FULLSCREEN flag: %s\n", (PsGlobal.screen->flags & SDL_FULLSCREEN) ? "SET" : "NOT SET");
        fflush(log);
    }

    // Set global surface for SDL2 compatibility shim
    g_webos_sdl_surface = PsGlobal.screen;

    // Initialize OpenGL ES 2.0 using PDL
    // Note: webOS only supports GLES 2.0 contexts, not GLES 1.1
    if (log) { fprintf(log, "Calling PDL_LoadOGL(PDL_OGL_2_0)...\n"); fflush(log); }
    PDL_Err err = PDL_LoadOGL(PDL_OGL_2_0);
    if (err != PDL_NOERROR) {
        if (log) { fprintf(log, "ERROR: PDL_LoadOGL failed: %s\n", PDL_GetError()); fclose(log); }
        debug("PDL_LoadOGL failed: %s\n", PDL_GetError());
        return FALSE;
    }
    if (log) { fprintf(log, "PDL_LoadOGL succeeded - OpenGL ES 2.0 loaded\n"); fflush(log); }

    // Verify OpenGL context is working
    log = NULL /* logging disabled */;
    if (log) {
        fprintf(log, "Testing OpenGL context...\n");

        // Try to get OpenGL version
        const GLubyte* version = glGetString(GL_VERSION);
        GLenum gl_error = glGetError();
        if (gl_error != GL_NO_ERROR) {
            fprintf(log, "ERROR: glGetString(GL_VERSION) failed with error: 0x%x\n", gl_error);
            fprintf(log, "OpenGL context is NOT working!\n");
            fclose(log);
            return FALSE;
        }

        if (version) {
            fprintf(log, "OpenGL Version: %s\n", version);
        } else {
            fprintf(log, "ERROR: glGetString returned NULL\n");
            fclose(log);
            return FALSE;
        }

        const GLubyte* vendor = glGetString(GL_VENDOR);
        const GLubyte* renderer = glGetString(GL_RENDERER);
        if (vendor) fprintf(log, "OpenGL Vendor: %s\n", vendor);
        if (renderer) fprintf(log, "OpenGL Renderer: %s\n", renderer);

        fprintf(log, "OpenGL context test successful!\n");
        fflush(log);
        fclose(log);
    }
    log = NULL;

    // Set RsGlobal screen dimensions for the game engine
    RsGlobal.maximumWidth = WEBOS_SCREEN_WIDTH;
    RsGlobal.maximumHeight = WEBOS_SCREEN_HEIGHT;
    RsGlobal.width = WEBOS_SCREEN_WIDTH;
    RsGlobal.height = WEBOS_SCREEN_HEIGHT;
    if (log) { fprintf(log, "RsGlobal dimensions set to %dx%d\n", RsGlobal.width, RsGlobal.height); fflush(log); }

    // Set window title and hide cursor
    SDL_WM_SetCaption("GTA III re3", "GTA III");
    SDL_ShowCursor(SDL_DISABLE);

    // Set orientation for webOS display system
    PDL_SetOrientation(PDL_ORIENTATION_0);
    if (log) { fprintf(log, "PDL_SetOrientation(PDL_ORIENTATION_0) called\n"); fflush(log); }

    // Get available memory
    _dwMemAvailPhys = getAvailableMemory();
    debug("Available memory: %zu MB\n", _dwMemAvailPhys / (1024 * 1024));
    if (log) { fprintf(log, "Available memory: %zu MB\n", _dwMemAvailPhys / (1024 * 1024)); fflush(log); }

    // Initialize joystick if available
    if (SDL_NumJoysticks() > 0) {
        SDL_JoystickOpen(0);
        PsGlobal.joy1id = 0;
        debug("Joystick detected: %s\n", SDL_JoystickName(0));
        if (log) { fprintf(log, "Joystick detected: %s\n", SDL_JoystickName(0)); fflush(log); }
    }

    if (log) { fprintf(log, "psInitialize() completed successfully\n"); fclose(log); }
    return TRUE;
}

/*
 * Platform termination
 */
void psTerminate(void)
{
    debug("Terminating WebOS platform...\n");

    if (PsGlobal.joy1id >= 0) {
        SDL_JoystickClose(SDL_JoystickOpen(PsGlobal.joy1id));
    }

    SDL_Quit();
    PDL_Quit();
}

/*
 * Create main window and OpenGL context
 */
RwBool _psSetVideoMode(RwInt32 subSystem, RwInt32 videoMode)
{
    debug("Setting video mode: %dx%d\n", WEBOS_SCREEN_WIDTH, WEBOS_SCREEN_HEIGHT);

    // Create fullscreen OpenGL ES surface
    PsGlobal.screen = SDL_SetVideoMode(
        WEBOS_SCREEN_WIDTH,
        WEBOS_SCREEN_HEIGHT,
        32,
        SDL_OPENGLES | SDL_FULLSCREEN
    );

    if (!PsGlobal.screen) {
        debug("SDL_SetVideoMode failed: %s\n", SDL_GetError());
        return FALSE;
    }

    // Set RsGlobal screen dimensions for the game engine
    RsGlobal.maximumWidth = WEBOS_SCREEN_WIDTH;
    RsGlobal.maximumHeight = WEBOS_SCREEN_HEIGHT;
    RsGlobal.width = WEBOS_SCREEN_WIDTH;
    RsGlobal.height = WEBOS_SCREEN_HEIGHT;

    // Set window title
    SDL_WM_SetCaption("GTA III re3", "GTA III");

    // Hide cursor for touch-only interface
    SDL_ShowCursor(SDL_DISABLE);

    debug("Video mode set successfully: %dx%d\n", RsGlobal.width, RsGlobal.height);
    return TRUE;
}

/*
 * Select device
 */
RwBool psSelectDevice()
{
    // WebOS only has one device (OpenGL ES)
    GnumSubSystems = 1;
    GcurSel = 0;

    return TRUE;
}

/*
 * Camera show raster (swap buffers)
 */
void psCameraShowRaster(RwCamera *camera)
{
    static bool firstCall = true;
    static int frameCount = 0;

    if (firstCall) {
        // Check OpenGL viewport settings
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);

        FILE *log = NULL; // fopen("/media/internal/.gta3/debug.log", "a");
        if (log) {
            fprintf(log, "psCameraShowRaster: GL_VIEWPORT = [%d, %d, %d, %d]\n",
                viewport[0], viewport[1], viewport[2], viewport[3]);

            // Check scissor test
            GLboolean scissorEnabled = glIsEnabled(GL_SCISSOR_TEST);
            fprintf(log, "psCameraShowRaster: GL_SCISSOR_TEST = %s\n",
                scissorEnabled ? "ENABLED" : "DISABLED");

            if (scissorEnabled) {
                GLint scissor[4];
                glGetIntegerv(GL_SCISSOR_BOX, scissor);
                fprintf(log, "psCameraShowRaster: GL_SCISSOR_BOX = [%d, %d, %d, %d]\n",
                    scissor[0], scissor[1], scissor[2], scissor[3]);
            }

            fflush(log); fclose(log);
        }
        firstCall = false;
    }

#ifdef WEBOS_TOUCHPAD
    // Render virtual controls overlay before swapping buffers
    RenderVirtualControls();
#endif

    // Swap buffers using SDL
    SDL_GL_SwapBuffers();

#ifdef WEBOS_TOUCHPAD
    // FPS counter for performance diagnosis
    static Uint32 fpsLastTime = 0;
    static int fpsFrameCount = 0;
    static float currentFPS = 0.0f;

    fpsFrameCount++;
    Uint32 currentTime = SDL_GetTicks();
    Uint32 elapsed = currentTime - fpsLastTime;

    if (elapsed >= 1000) { // Update FPS every second
        currentFPS = (fpsFrameCount * 1000.0f) / elapsed;
        FILE* log = fopen("/media/internal/.gta3/debug.log", "a");
        if (log) {
            fprintf(log, "FPS: %.2f (frames=%d, time=%ums)\n", currentFPS, fpsFrameCount, elapsed);
            fclose(log);
        }
        fpsFrameCount = 0;
        fpsLastTime = currentTime;
    }
#endif
}

/*
 * Camera begin update
 */
RwBool psCameraBeginUpdate(RwCamera *camera)
{
    if (!RwCameraBeginUpdate(Scene.camera)) {
        ForegroundApp = FALSE;
        return FALSE;
    }
    return TRUE;
}

/*
 * Timer functions
 */
double psTimer(void)
{
    return SDL_GetTicks();  // Return milliseconds, not seconds!
}

/*
 * Keyboard state (stubbed for touchscreen)
 */
void psKeyboardUpdate()
{
    // Keyboard input handled by touch interface
}

/*
 * Mouse state (used for touch input)
 */
void psMouseUpdate()
{
    // Mouse emulated by touch input
}

/*
 * Handle SDL events
 */
void HandleSDLEvents()
{
    static bool firstCall = true;
    static int eventCount = 0;

    if (firstCall) {
        FILE *log = NULL; // fopen("/media/internal/.gta3/debug.log", "a");
        if (log) { fprintf(log, "HandleSDLEvents: First call, about to declare event\n"); fflush(log); fclose(log); }
    }

    SDL_Event event;

    if (firstCall) {
        FILE *log = NULL; // fopen("/media/internal/.gta3/debug.log", "a");
        if (log) { fprintf(log, "HandleSDLEvents: Event declared, about to call SDL_PollEvent\n"); fflush(log); fclose(log); }
    }

    while (SDL_PollEvent(&event)) {
        // Log first 200 events + ALL mouse/joystick button events
        if (eventCount < 200 || event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP ||
            event.type == SDL_JOYBUTTONDOWN || event.type == SDL_JOYBUTTONUP) {
            FILE *log = NULL; // fopen("/media/internal/.gta3/debug.log", "a");
            if (log) {
                fprintf(log, "HandleSDLEvents: Event #%d, type=%d ", eventCount, event.type);
                switch(event.type) {
                    case SDL_MOUSEBUTTONDOWN: fprintf(log, "(MOUSEBUTTONDOWN)\n"); break;
                    case SDL_MOUSEBUTTONUP: fprintf(log, "(MOUSEBUTTONUP)\n"); break;
                    case SDL_MOUSEMOTION: fprintf(log, "(MOUSEMOTION)\n"); break;
                    case SDL_QUIT: fprintf(log, "(QUIT)\n"); break;
                    case SDL_ACTIVEEVENT: fprintf(log, "(ACTIVEEVENT)\n"); break;
                    case SDL_VIDEOEXPOSE: fprintf(log, "(VIDEOEXPOSE)\n"); break;
                    case SDL_JOYAXISMOTION:
                        fprintf(log, "(JOYAXISMOTION axis=%d value=%d)\n", event.jaxis.axis, event.jaxis.value);
                        break;
                    case SDL_JOYBUTTONDOWN:
                        fprintf(log, "(JOYBUTTONDOWN button=%d)\n", event.jbutton.button);
                        break;
                    case SDL_JOYBUTTONUP:
                        fprintf(log, "(JOYBUTTONUP button=%d)\n", event.jbutton.button);
                        break;
                    default: fprintf(log, "(UNKNOWN type %d)\n", event.type); break;
                }
                fflush(log); fclose(log);
            }
            eventCount++;
        }

        if (firstCall) {
            FILE *log = NULL; // fopen("/media/internal/.gta3/debug.log", "a");
            if (log) { fprintf(log, "HandleSDLEvents: SDL_PollEvent returned event, type=%d\n", event.type); fflush(log); fclose(log); }
        }

        switch (event.type) {
            case SDL_QUIT:
                RsEventHandler(rsQUITAPP, nil);
                break;

            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
            case SDL_MOUSEMOTION:
                HandleTouchInput(&event);
                break;

#ifdef WEBOS_TOUCHPAD
            case SDL_JOYAXISMOTION:
            case SDL_JOYBUTTONDOWN:
            case SDL_JOYBUTTONUP:
                // WebOS delivers touch as joystick events - convert to mouse events
                {
                    static int lastX = -1, lastY = -1;
                    static bool touchActive = false;
                    static int restingX = -30000, restingY = -2000; // Approximate resting position
                    static int lastAxisX = -30000, lastAxisY = -2000;
                    static int stableFrames = 0;
                    static Uint32 lastEventTime = 0;

                    if (event.type == SDL_JOYAXISMOTION) {
                        // Axis 0 = X, Axis 1 = Y (values from -32768 to 32767)
                        static int axisX = -30000, axisY = -2000;

                        if (event.jaxis.axis == 0) {
                            axisX = event.jaxis.value;
                        } else if (event.jaxis.axis == 1) {
                            axisY = event.jaxis.value;
                        }

                        Uint32 currentTime = SDL_GetTicks();

                        // Detect touch release: ONLY by timeout (no stable frames - too unreliable)
                        if (touchActive) {
                            if (currentTime - lastEventTime > 150) {
                                // No events for 150ms - touch released
                                FILE *log = NULL; // fopen("/media/internal/.gta3/debug.log", "a");
                                if (log) {
                                    fprintf(log, "JOYSTICK: Timeout triggered! currentTime=%u, lastEventTime=%u, diff=%u ms → Sending MOUSEBUTTONUP\n",
                                        currentTime, lastEventTime, currentTime - lastEventTime);
                                    fflush(log);
                                    fclose(log);
                                }
                                SDL_Event mouseEvent;
                                mouseEvent.type = SDL_MOUSEBUTTONUP;
                                mouseEvent.button.button = SDL_BUTTON_LEFT;
                                mouseEvent.button.x = lastX;
                                mouseEvent.button.y = lastY;
                                HandleTouchInput(&mouseEvent);
                                touchActive = false;
                                stableFrames = 0;
                            }
                        }

                        lastEventTime = currentTime;
                        lastAxisX = axisX;
                        lastAxisY = axisY;

                        // Convert joystick coordinates to screen coordinates
                        int screenX = (axisX + 32768) * WEBOS_SCREEN_WIDTH / 65536;
                        int screenY = (axisY + 32768) * WEBOS_SCREEN_HEIGHT / 65536;

                        // Clamp to screen bounds
                        if (screenX < 0) screenX = 0;
                        if (screenX >= WEBOS_SCREEN_WIDTH) screenX = WEBOS_SCREEN_WIDTH - 1;
                        if (screenY < 0) screenY = 0;
                        if (screenY >= WEBOS_SCREEN_HEIGHT) screenY = WEBOS_SCREEN_HEIGHT - 1;

                        // Synthesize mouse events
                        SDL_Event mouseEvent;

                        // First touch = significant movement from resting position
                        int deltaX = abs(axisX - restingX);
                        int deltaY = abs(axisY - restingY);

                        if (!touchActive && (deltaX > 2000 || deltaY > 2000)) {
                            FILE *log = NULL; // fopen("/media/internal/.gta3/debug.log", "a");
                            if (log) {
                                fprintf(log, "JOYSTICK: Touch detected! axisX=%d, axisY=%d, screen=(%d,%d), deltaX=%d, deltaY=%d → Sending MOUSEBUTTONDOWN\n",
                                    axisX, axisY, screenX, screenY, deltaX, deltaY);
                                fflush(log);
                                fclose(log);
                            }
                            mouseEvent.type = SDL_MOUSEBUTTONDOWN;
                            mouseEvent.button.button = SDL_BUTTON_LEFT;
                            mouseEvent.button.x = screenX;
                            mouseEvent.button.y = screenY;
                            HandleTouchInput(&mouseEvent);
                            touchActive = true;
                            lastX = screenX;
                            lastY = screenY;
                            stableFrames = 0;
                        }
                        // Touch is active and moving
                        else if (touchActive && (screenX != lastX || screenY != lastY)) {
                            mouseEvent.type = SDL_MOUSEMOTION;
                            mouseEvent.motion.x = screenX;
                            mouseEvent.motion.y = screenY;
                            HandleTouchInput(&mouseEvent);
                            lastX = screenX;
                            lastY = screenY;
                        }
                    }
                }
                break;
#endif

            case SDL_ACTIVEEVENT:
                if (event.active.state & SDL_APPACTIVE) {
                    if (event.active.gain) {
                        ForegroundApp = TRUE;
                        WindowFocused = TRUE;
                    } else {
                        ForegroundApp = FALSE;
                        WindowFocused = FALSE;
                    }
                }
                break;

            case SDL_VIDEOEXPOSE:
                if (firstCall) {
                    FILE *log = NULL; // fopen("/media/internal/.gta3/debug.log", "a");
                    if (log) { fprintf(log, "HandleSDLEvents: Got SDL_VIDEOEXPOSE, calling RsEventHandler(rsCAMERASIZE)\n"); fflush(log); fclose(log); }
                }
                // Create proper RwRect structure (not just a pointer to maximumWidth!)
                RwRect r;
                r.x = 0;
                r.y = 0;
                r.w = RsGlobal.maximumWidth;
                r.h = RsGlobal.maximumHeight;
                if (RsEventHandler(rsCAMERASIZE, &r) == rsEVENTERROR) {
                    if (firstCall) {
                        FILE *log = NULL; // fopen("/media/internal/.gta3/debug.log", "a");
                        if (log) { fprintf(log, "HandleSDLEvents: rsCAMERASIZE returned ERROR\n"); fflush(log); fclose(log); }
                    }
                    return;
                }
                if (firstCall) {
                    FILE *log = NULL; // fopen("/media/internal/.gta3/debug.log", "a");
                    if (log) { fprintf(log, "HandleSDLEvents: rsCAMERASIZE succeeded\n"); fflush(log); fclose(log); }
                }
                break;
        }
    }

    if (firstCall) {
        FILE *log = NULL; // fopen("/media/internal/.gta3/debug.log", "a");
        if (log) { fprintf(log, "HandleSDLEvents: Completed successfully (no more events)\n"); fflush(log); fclose(log); }
        firstCall = false;
    }
}

/*
 * Main event loop
 */
RsEventStatus psMainLoop()
{
    static bool firstCall = true;

    if (firstCall) {
        FILE *log = NULL; // fopen("/media/internal/.gta3/debug.log", "a");
        if (log) { fprintf(log, "psMainLoop: First call - about to handle SDL events\n"); fflush(log); fclose(log); }
    }

    // Handle SDL events
    HandleSDLEvents();

    if (firstCall) {
        FILE *log = NULL; // fopen("/media/internal/.gta3/debug.log", "a");
        if (log) { fprintf(log, "psMainLoop: HandleSDLEvents() succeeded\n"); fflush(log); fclose(log); }
    }

    // NOTE: ProcessVirtualControls() is now called from FrontendIdle/Idle BEFORE CPad::UpdatePads()
    // to ensure pad state is fresh when UpdatePads reads it

    // Update PDL
    PDL_CallJS("update", NULL, 0);

    if (firstCall) {
        FILE *log = NULL; // fopen("/media/internal/.gta3/debug.log", "a");
        if (log) { fprintf(log, "psMainLoop: PDL_CallJS() succeeded, about to return\n"); fflush(log); fclose(log); }
        firstCall = false;
    }

    return rsEVENTPROCESSED;
}

/*
 * Install file system
 */
RwBool psInstallFileSystem(void)
{
    // Set data directory
    char dataPath[PATH_MAX];
    strcpy(dataPath, "/media/internal/.gta3/data");

    debug("Data path: %s\n", dataPath);

    // This will be called by the game to set up file paths
    return TRUE;
}

/*
 * Mouse position (emulated via touch)
 */
RwV2d *psMouseGetPos()
{
    return &PsGlobal.touchPos;
}

/*
 * Pad state
 */
CPad *psPadGetPad(int padNum)
{
    return Pads;
}

/*
 * Path operations
 */
bool psIsKeyDown(int key)
{
    return false; // Touch-only interface
}

/*
 * Global structure access
 */
void psGlobalInit()
{
    RsGlobal.ps = &PsGlobal;
}

/*
 * Install file system paths
 */
void psPathnameCreate(char *buffer)
{
    // No-op for WebOS
}

void psPathnameDestroy(char *buffer)
{
    // No-op for WebOS
}

/*
 * RenderWare initialization
 */
void _psSelectScreenVM(RwInt32 videoMode)
{
    // No-op for WebOS
}

/*
 * Fullscreen toggle (always fullscreen on WebOS)
 */
void _psToggleFullScreen()
{
    // WebOS is always fullscreen
}

/*
 * Get number of video modes
 */
int _psGetNumVideos()
{
    return 1; // Only one mode on WebOS
}

/*
 * Get native video mode
 */
RwInt32 psGetVideoMode()
{
    return 0;
}

/*
 * Close application
 */
void psClose()
{
    SDL_Event quit_event;
    quit_event.type = SDL_QUIT;
    SDL_PushEvent(&quit_event);
}

/*
 * Signal handler for clean termination
 */
void terminateHandler(int sig, siginfo_t *info, void *ucontext)
{
    RsGlobal.quit = TRUE;
}

/*
 * Post-RenderWare initialization
 */
void psPostRWinit(void)
{
    // Initialize input after RenderWare is ready
    // Clear pad state
    CPad::GetPad(0)->Clear(true);
    CPad::GetPad(1)->Clear(true);
}

/*
 * Missing ps* functions
 */
RwImage *psGrabScreen(RwCamera *camera)
{
    // Screenshot functionality - not implemented for webOS
    return nil;
}

void psMouseSetPos(RwV2d *pos)
{
    // Mouse positioning - not applicable for touch interface
}

RwMemoryFunctions *psGetMemoryFunctions(void)
{
    return nil;
}

RwBool psNativeTextureSupport(void)
{
    // Native texture support handled by librw
    return TRUE;
}

RwChar **_psGetVideoModeList(void)
{
    static RwChar *modeList[2];
    static RwChar mode[32];
    sprintf(mode, "%dx%dx32", WEBOS_SCREEN_WIDTH, WEBOS_SCREEN_HEIGHT);
    modeList[0] = mode;
    modeList[1] = nil;
    return modeList;
}

int _psGetNumVideModes(void)
{
    return 1;
}

void _InputTranslateShiftKeyUpDown(RsKeyCodes* rs)
{
    // Windows DirectInput keyboard translation - not used on webOS
}

/*
 * Main entry point
 */
int main(int argc, char *argv[])
{
    RwInt32 i;

    // Debug logging
    FILE *debugLog = fopen("/media/internal/.gta3/debug.log", "w");
    if (debugLog) {
        fprintf(debugLog, "=== RE3 DEBUG LOG ===\n");
        fprintf(debugLog, "Main() started\n");
        fflush(debugLog);
    }

#ifdef USE_CUSTOM_ALLOCATOR
    InitMemoryMgr();
    if (debugLog) { fprintf(debugLog, "Memory manager initialized\n"); fflush(debugLog); }
#endif

    // Set up signal handlers
    struct sigaction act;
    act.sa_sigaction = terminateHandler;
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGTERM, &act, NULL);
    if (debugLog) { fprintf(debugLog, "Signal handlers set up\n"); fflush(debugLog); }

    // Initialize platform independent data
    if (debugLog) { fprintf(debugLog, "Calling RsEventHandler(rsINITIALIZE)...\n"); fflush(debugLog); }
    if (RsEventHandler(rsINITIALIZE, nil) == rsEVENTERROR) {
        if (debugLog) { fprintf(debugLog, "ERROR: rsINITIALIZE failed!\n"); fclose(debugLog); }
        return FALSE;
    }
    if (debugLog) { fprintf(debugLog, "rsINITIALIZE succeeded\n"); fflush(debugLog); }

    // Parse command line parameters
    for (i = 1; i < argc; i++) {
        RsEventHandler(rsPREINITCOMMANDLINE, argv[i]);
    }

    // Set up RenderWare initialization parameters
    openParams.width = WEBOS_SCREEN_WIDTH;
    openParams.height = WEBOS_SCREEN_HEIGHT;
    openParams.windowtitle = RsGlobal.appName;

    ControlsManager.MakeControllerActionsBlank();
    ControlsManager.InitDefaultControlConfiguration();
    if (debugLog) { fprintf(debugLog, "Controls initialized\n"); fflush(debugLog); }

    // Initialize RenderWare
    if (debugLog) { fprintf(debugLog, "Calling RsEventHandler(rsRWINITIALIZE)...\n"); fflush(debugLog); }
    RsEventStatus rwInitResult = RsEventHandler(rsRWINITIALIZE, &openParams);
    if (debugLog) { fprintf(debugLog, "rsRWINITIALIZE returned: %d (rsEVENTERROR=%d, rsEVENTPROCESSED=%d)\n", rwInitResult, rsEVENTERROR, rsEVENTPROCESSED); fflush(debugLog); }
    if (rsEVENTERROR == rwInitResult) {
        if (debugLog) { fprintf(debugLog, "ERROR: rsRWINITIALIZE failed!\n"); fclose(debugLog); }
        RsEventHandler(rsTERMINATE, nil);
        return 0;
    }
    if (debugLog) { fprintf(debugLog, "RenderWare initialized successfully\n"); fflush(debugLog); }
    RwInitialised = TRUE;

    // Post-RW initialization
    if (debugLog) { fprintf(debugLog, "About to call psPostRWinit()...\n"); fflush(debugLog); }
    psPostRWinit();
    if (debugLog) { fprintf(debugLog, "Post-RW init complete\n"); fflush(debugLog); }

    // Parse post-init command line parameters
    if (debugLog) { fprintf(debugLog, "Parsing command line parameters...\n"); fflush(debugLog); }
    for (i = 1; i < argc; i++) {
        RsEventHandler(rsCOMMANDLINE, argv[i]);
    }
    if (debugLog) { fprintf(debugLog, "Command line parsing complete\n"); fflush(debugLog); }

    // Initialize game state - start with frontend menu, NOT gameplay
    gGameState = GS_START_UP;
#ifdef WEBOS_TOUCHPAD
    FILE *initLog = NULL /* logging disabled */;
    if (initLog) { fprintf(initLog, "Initialized gGameState = GS_START_UP\n"); fflush(initLog); fclose(initLog); }
#endif

    // Main game loop with state machine
    if (debugLog) { fprintf(debugLog, "Entering main game loop...\n"); fflush(debugLog); fclose(debugLog); debugLog = NULL; }
    int loopCount = 0;
    while (TRUE) {
        RsEventStatus status = psMainLoop();
        if (status == rsEVENTERROR) {
            break;
        }

        if (ForegroundApp) {
            switch (gGameState) {
                case GS_START_UP:
                {
                    gGameState = GS_INIT_ONCE;
                    break;
                }

                case GS_INIT_ONCE:
                {
                    LoadingScreen(nil, nil, "loadsc0");
                    if (!CGame::InitialiseOnceAfterRW())
                        RsGlobal.quit = TRUE;
                    gGameState = GS_INIT_FRONTEND;
#ifdef WEBOS_TOUCHPAD
                    if (loopCount < 3) {
                        FILE *log = NULL; // fopen("/media/internal/.gta3/debug.log", "a");
                        if (log) { fprintf(log, "GS_INIT_ONCE completed, transitioning to GS_INIT_FRONTEND\n"); fflush(log); fclose(log); }
                    }
#endif
                    break;
                }

                case GS_INIT_FRONTEND:
                {
                    LoadingScreen(nil, nil, "loadsc0");
                    FrontEndMenuManager.m_bGameNotLoaded = true;
                    CMenuManager::m_bStartUpFrontEndRequested = true;
                    gGameState = GS_FRONTEND;
#ifdef WEBOS_TOUCHPAD
                    if (loopCount < 3) {
                        FILE *log = NULL; // fopen("/media/internal/.gta3/debug.log", "a");
                        if (log) { fprintf(log, "GS_INIT_FRONTEND completed, m_bGameNotLoaded=true, transitioning to GS_FRONTEND\n"); fflush(log); fclose(log); }
                    }
#endif
                    break;
                }

                case GS_FRONTEND:
                {
                    if (!WindowIconified)
                        RsEventHandler(rsFRONTENDIDLE, nil);

                    if (!FrontEndMenuManager.m_bMenuActive || FrontEndMenuManager.m_bWantToLoad) {
                        gGameState = GS_INIT_PLAYING_GAME;
#ifdef WEBOS_TOUCHPAD
                        FILE *log = NULL; // fopen("/media/internal/.gta3/debug.log", "a");
                        if (log) { fprintf(log, "User started game, transitioning to GS_INIT_PLAYING_GAME\n"); fflush(log); fclose(log); }
#endif
                    }

                    if (FrontEndMenuManager.m_bWantToLoad) {
                        InitialiseGame();
                        FrontEndMenuManager.m_bGameNotLoaded = false;
                        gGameState = GS_PLAYING_GAME;
#ifdef WEBOS_TOUCHPAD
                        FILE *log2 = NULL /* logging disabled */;
                        if (log2) { fprintf(log2, "InitialiseGame() called, m_bGameNotLoaded=false, transitioning to GS_PLAYING_GAME\n"); fflush(log2); fclose(log2); }
#endif
                    }
                    break;
                }

                case GS_INIT_PLAYING_GAME:
                {
                    InitialiseGame();
                    FrontEndMenuManager.m_bGameNotLoaded = false;
                    gGameState = GS_PLAYING_GAME;
#ifdef WEBOS_TOUCHPAD
                    FILE *log = NULL; // fopen("/media/internal/.gta3/debug.log", "a");
                    if (log) { fprintf(log, "GS_INIT_PLAYING_GAME: InitialiseGame() called, transitioning to GS_PLAYING_GAME\n"); fflush(log); fclose(log); }
#endif
                    break;
                }

                case GS_PLAYING_GAME:
                {
#ifdef WEBOS_TOUCHPAD
                    static int playingFrameCount = 0;
                    if (playingFrameCount < 5) {
                        FILE *log = NULL; // fopen("/media/internal/.gta3/debug.log", "a");
                        if (log) { fprintf(log, "GS_PLAYING_GAME: Frame %d, RwInitialised=%d, about to call rsIDLE\n", playingFrameCount, RwInitialised); fflush(log); fclose(log); }
                    }
#endif
                    if (RwInitialised)
                        RsEventHandler(rsIDLE, (void *)TRUE);
#ifdef WEBOS_TOUCHPAD
                    if (playingFrameCount < 5) {
                        FILE *log = NULL; // fopen("/media/internal/.gta3/debug.log", "a");
                        if (log) { fprintf(log, "GS_PLAYING_GAME: Frame %d, rsIDLE completed\n", playingFrameCount); fflush(log); fclose(log); }
                        playingFrameCount++;
                    }
#endif
                    break;
                }
            }
        }

        if (RsGlobal.quit) {
            break;
        }

        loopCount++;
    }

    // Cleanup
    RsEventHandler(rsTERMINATE, nil);

    return 0;
}

// Stub for CapturePad - not used on webOS touchscreen device
void CapturePad(RwInt32 padID) {
    (void)padID;
    // No-op: webOS uses touchscreen input, not gamepad
}

#endif // WEBOS_TOUCHPAD
