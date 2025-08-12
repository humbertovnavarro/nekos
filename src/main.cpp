#include "Arduino.h"
#include "WiFi.h"
#include "Nekos.h"

void setup() {
    nekos::fs::init();
    nekos::registerCoreUtils();
    nekos::Console::begin();
}

void loop() {
    nekos::Console::poll();
}