#include "Arduino.h"
// Quick macros for simple shims
namespace nekos {
    #ifdef ESP32
        #define REBOOT() esp_restart();
    #else
        #define REBOOT() abort();
    #endif

}
