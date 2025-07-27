// Do not put your application code here. It goes in services/yourapp.cpp. Look at services/core for examples.
#include "Arduino.h"
#include "lib/Console.h"

void setup(void) {
    Console::init();
    Console::log("Starting NEKOS V0.01 alpha");
}

#ifdef HAS_DISPLAY
    #include "services/core/DisplayService.h"
#endif

void loop(void) {
    vTaskDelay(100); 
}