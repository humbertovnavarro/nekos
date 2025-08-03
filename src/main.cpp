#include "Arduino.h"
#include "lib/FrameBuffer.h"
#include "services/core/Menu.h"
#include "lib/Tasks.h"

void setup(void) {
    Serial.println("Starting NEKOS");
    framebuffer_init();
    START_TASK(menu, 0, 8192);
}

void loop(void) {
    vTaskDelay(100);
}