#include "Arduino.h"
#include "LuaProcessScheduler.hpp"
#include "neopixel.hpp"
#include "display.hpp"
void setup() {
    Serial.begin(115200);
    neopixel.begin();
    u8g2.begin();
    LuaProcessScheduler::begin();
    LuaProcessScheduler::run("display.clear() display.setFont(\"u8g2_font_ncenB08_tr\") display.drawStr(0, 64, \"Hello Lua!\") display.sendBuffer()");
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000));
    LuaProcessScheduler::run("neopixel.setPixel(0,255,0,0) neopixel.show()");
    vTaskDelay(pdMS_TO_TICKS(1000));
    LuaProcessScheduler::run("neopixel.setPixel(0, 0,255,0) neopixel.show()");
    vTaskDelay(pdMS_TO_TICKS(1000));
    LuaProcessScheduler::run("neopixel.setPixel(0, 0,0,255) neopixel.show()");
    vTaskDelay(pdMS_TO_TICKS(1000));
}
