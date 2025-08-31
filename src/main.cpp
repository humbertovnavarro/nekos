#include "Arduino.h"
#include "nekos/Nekos.h"
#include "nekos/NekosConsole.h"
void setup() {
  Serial.print("Starting init.lua");
}

void loop() {
  nekos::Console::poll();
  vTaskDelay(1);
}

