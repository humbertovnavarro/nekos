#include "Arduino.h"
#include "lib/Console.h"

void setup(void) {
    Console::init();
    Console::log("Starting NEKOS V0.01 alpha");
}

#ifdef DISPLAY
    #include "services/DisplayService.h"
#endif

void loop(void) {
    vTaskDelay(100); 
}