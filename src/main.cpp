#include "Arduino.h"
#include "drivers/input.hpp"
#include "drivers/neopixel.hpp"
#include "drivers/u8g2.hpp"
#include "drivers/usbmsc.hpp"
#include "sys/LuaCompiler.hpp"
#include "sys/LuaProcessScheduler.hpp"

void initializeDrivers() {
    Serial.begin();
    u8g2.begin();
    neopixel.begin();
    inputBegin();
    setupUSBMSC();
}

void prepareLuaEnvironment() {
    enumerateAndCompileLuaFiles("/");
    LuaProcessScheduler::begin();
}

void setup() {
    initializeDrivers();
    prepareLuaEnvironment();
    LuaProcessScheduler::run("/boot.luac", {
        .stackSize = 16384,
        .priority = 1,
        .affinity = 0
    });
}

void loop() {
    unsigned long start = micros();
    LuaProcessScheduler::run("/tests.luac");
    unsigned long end = micros();
    unsigned long duration = end - start;
    Serial.printf("[LuaExec] Process time: %luus\n", duration);
    vTaskDelay(500 / portTICK_PERIOD_MS);
}
