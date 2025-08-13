#include "Arduino.h"
#include "WiFi.h"
#include "Nekos.h"
#include "config/device.h"

constexpr int MAX_LOG_LINES = 8;
constexpr int MAX_LINE_LENGTH = 64;




void setup() {
    nekos::fs::init();
    nekos::registerCoreUtils();
    nekos::Console::begin(BAUD);
}

void loop() {
    vTaskDelay(portTICK_PERIOD_MS * 10);
}