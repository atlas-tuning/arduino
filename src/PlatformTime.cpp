#if defined(ARDUINO)
#include <Arduino.h>
#endif

#include "PlatformTime.h"

long platform_get_micros() {
    #if defined(ARDUINO)
    return micros();
    #else
    return std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    #endif
}