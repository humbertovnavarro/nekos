// Do not put your application code here. It goes in services/yourapp.cpp. Look at services/core for examples.
#include "Arduino.h"
void setup(void) {
}

#ifdef HAS_DISPLAY
    #include "services/core/DisplayService.h"
#endif

void loop(void) {
    vTaskStartScheduler();
}