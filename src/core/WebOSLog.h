#pragma once

#ifdef WEBOS_TOUCHPAD

#include <stdio.h>

// Global persistent log file handle to avoid file descriptor exhaustion
class CWebOSLog {
private:
    static FILE *s_logFile;
    static bool s_initialized;

public:
    static void Initialize() {
        if (!s_initialized) {
            s_logFile = fopen("/media/internal/.gta3/debug.log", "a");
            if (s_logFile) {
                setvbuf(s_logFile, NULL, _IOLBF, 0); // Line buffered
            }
            s_initialized = true;
        }
    }

    static void Shutdown() {
        if (s_logFile) {
            fclose(s_logFile);
            s_logFile = nullptr;
        }
        s_initialized = false;
    }

    static void Write(const char *format, ...) {
        if (!s_logFile) return;

        va_list args;
        va_start(args, format);
        vfprintf(s_logFile, format, args);
        va_end(args);
    }
};

#endif // WEBOS_TOUCHPAD
