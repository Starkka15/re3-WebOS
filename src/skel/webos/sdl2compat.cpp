// SDL1.2 to SDL2 compatibility shim implementation

#include "sdl2compat.h"

#ifdef WEBOS_TOUCHPAD

// Global SDL1.2 surface used by compatibility layer
SDL_Surface* g_webos_sdl_surface = nullptr;

#endif // WEBOS_TOUCHPAD
