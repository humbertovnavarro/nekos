#include "Arduino.h"
#include "WiFi.h"
#include <U8g2lib.h>
#include <Wire.h>
#include "MUIU8g2.h"
#include <Adafruit_NeoPixel.h>
#include "nekos/NekosAppRegistry.h"
#include "nekos/NekosFS.h"
#include "nekos/Nekos.h"
void setup() {
    nekos::init();
}

void loop() {
    vTaskDelay(100);
}