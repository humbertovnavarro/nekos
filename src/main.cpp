#include "Arduino.h"
#include "services/core/DisplayService.h"
#include "lib/Console.h"
void setup(void) {
    Console::log("Starting NEKOS");
}

void loop(void) {
    Console::log("Hello from Arduino main Loop");
    Serial.println("Something fucky");
    vTaskDelay(100);
}