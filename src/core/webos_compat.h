#ifdef WEBOS_TOUCHPAD

// WebOS TouchPad compatibility layer
// Similar to psp2_compat.h for PS Vita

// Audio sample rate optimized for WebOS
#define DIGITALRATE 48000  // Best rate for WebOS hardware

// WebOS-specific path handling
// realpath is already provided by stdlib.h in the webOS toolchain
// No need to forward declare it

// Memory allocation helpers for limited RAM environment
// WebOS TouchPad has 512MB-1GB RAM depending on model
#define WEBOS_HEAP_SIZE (256 * 1024 * 1024)  // 256MB for game heap

// Platform-specific optimizations
#ifdef __ARM_NEON__
// NEON-specific optimizations can be enabled here
// The Cortex-A8 in WebOS has NEON support
#define USE_NEON_OPTIMIZATIONS
#endif

// WebOS PDL and SDL compatibility
#include <SDL/SDL.h>
#include <pdl.h>

#endif // WEBOS_TOUCHPAD
