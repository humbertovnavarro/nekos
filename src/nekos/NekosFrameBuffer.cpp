#include <U8g2lib.h>
#include "Arduino.h"
#include "FreeRTOS.h"

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
SemaphoreHandle_t framebufferMut;

namespace nekos {
    U8G2 takeDisplay() {
        while(xSemaphoreTake(framebufferMut, 1000) != pdTRUE) {}
        return u8g2;
    }

    void releaseDisplay() {
        xSemaphoreGive(framebufferMut);
    }
}
