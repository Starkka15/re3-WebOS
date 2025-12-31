#include "common.h"
#include "WebOSLog.h"

#ifdef WEBOS_TOUCHPAD

FILE *CWebOSLog::s_logFile = nullptr;
bool CWebOSLog::s_initialized = false;

#endif // WEBOS_TOUCHPAD
