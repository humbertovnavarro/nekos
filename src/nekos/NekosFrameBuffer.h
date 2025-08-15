#include <U8g2lib.h>
#include "Arduino.h"
#include "FreeRTOS.h"
namespace nekos {
    SemaphoreHandle_t framebufferMut;
    U8G2 takeDisplay();
    void releaseDisplay();
}
