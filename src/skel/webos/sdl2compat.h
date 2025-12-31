// SDL1.2 to SDL2 compatibility shim for librw GL3 on webOS
// This allows librw's GL3 backend to work with webOS PDK's SDL 1.2

#pragma once

#ifdef WEBOS_TOUCHPAD

#include <SDL/SDL.h>

// SDL2 window flags mapped to SDL1.2
#define SDL_WINDOW_OPENGL       SDL_OPENGL
#define SDL_WINDOW_FULLSCREEN   SDL_FULLSCREEN
#define SDL_WINDOW_RESIZABLE    SDL_RESIZABLE

// SDL2 types mapped to SDL1.2 equivalents
typedef SDL_Surface* SDL_Window;
typedef void* SDL_GLContext;

// SDL2 display mode structure (simplified for webOS fullscreen)
typedef struct {
    Uint32 format;      // Pixel format
    int w, h;           // Width and height
    int refresh_rate;   // Refresh rate
    void* driverdata;   // Driver-specific data
} SDL_DisplayMode;

// Window position constants (unused in SDL1.2 fullscreen)
#define SDL_WINDOWPOS_UNDEFINED 0

// Context profile enum (for GLES selection)
typedef enum {
    SDL_GL_CONTEXT_PROFILE_CORE = 0,
    SDL_GL_CONTEXT_PROFILE_ES   = 2
} SDL_GLprofile;

// SDL_GL attribute enums
#define SDL_GL_CONTEXT_PROFILE_MASK  SDL_GL_CONTEXT_MAJOR_VERSION  // Reuse existing enum
#define SDL_BITSPERPIXEL(X) (32)  // Always 32bpp on webOS

// Compatibility functions
#ifdef __cplusplus
extern "C" {
#endif

// Global state for SDL1.2 surface
extern SDL_Surface* g_webos_sdl_surface;

// SDL2-style window creation wrapping SDL1.2
static inline SDL_Window SDL_CreateWindow(
    const char* title,
    int x, int y,
    int w, int h,
    Uint32 flags)
{
    (void)x; (void)y; // Unused in fullscreen

    // Use existing surface if already created
    if (g_webos_sdl_surface) {
        return g_webos_sdl_surface;
    }

    // Create new surface with OpenGL ES
    Uint32 sdl_flags = SDL_OPENGLES | SDL_FULLSCREEN;
    g_webos_sdl_surface = SDL_SetVideoMode(w, h, 32, sdl_flags);

    if (g_webos_sdl_surface && title) {
        SDL_WM_SetCaption(title, title);
    }

    return g_webos_sdl_surface;
}

// GL context functions (no-ops since SDL1.2 creates context automatically)
static inline SDL_GLContext SDL_GL_CreateContext(SDL_Window window) {
    (void)window;
    return (SDL_GLContext)1; // Return dummy non-NULL pointer
}

static inline void SDL_GL_DeleteContext(SDL_GLContext context) {
    (void)context;
    // No-op: SDL1.2 manages context automatically
}

// Swap buffers - SDL1.2 equivalent
static inline void SDL_GL_SwapWindow(SDL_Window window) {
    (void)window;
    SDL_GL_SwapBuffers();
}

// Set swap interval - SDL1.2 doesn't support this, but provide stub
static inline int SDL_GL_SetSwapInterval(int interval) {
    (void)interval;
    return 0; // Success
}

// Get window size (return fixed webOS resolution)
static inline void SDL_GetWindowSize(SDL_Window window, int* w, int* h) {
    (void)window;
    if (w) *w = 1024;
    if (h) *h = 768;
}

// Display mode functions (stubs for fullscreen-only webOS)
static inline int SDL_GetNumDisplayModes(int displayIndex) {
    (void)displayIndex;
    return 1; // Only one mode: native fullscreen
}

static inline int SDL_GetDisplayMode(int displayIndex, int modeIndex, SDL_DisplayMode* mode) {
    (void)displayIndex; (void)modeIndex;
    if (mode) {
        mode->w = 1024;
        mode->h = 768;
        mode->refresh_rate = 60;
        mode->driverdata = NULL;
    }
    return 0;
}

static inline int SDL_GetCurrentDisplayMode(int displayIndex, SDL_DisplayMode* mode) {
    return SDL_GetDisplayMode(displayIndex, 0, mode);
}

static inline int SDL_SetWindowDisplayMode(SDL_Window window, const SDL_DisplayMode* mode) {
    (void)window; (void)mode;
    return 0; // Success (no-op on webOS)
}

static inline int SDL_GetNumVideoDisplays(void) {
    return 1; // Single display
}

static inline const char* SDL_GetDisplayName(int displayIndex) {
    (void)displayIndex;
    return "HP TouchPad Display";
}

static inline void SDL_DestroyWindow(SDL_Window window) {
    (void)window;
    // No-op: SDL1.2 manages surface automatically
}

// Note: SDL_GL_GetProcAddress is already provided by WebOS SDL 1.2

#ifdef __cplusplus
}
#endif

#endif // WEBOS_TOUCHPAD
