#include "Arduino.h"
#include "services/core/DisplayService.h"
void setup(void) {
    Serial.begin(115200);
}

void loop(void) {
    vTaskDelay(1);
}