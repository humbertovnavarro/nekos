#include "Arduino.h"
#include "nekos/NekosConsole.h"
#include "FFat.h"
void setup() {
  FFat.begin(true, "/", 10, "storage");
  nekos::Console::begin();
}

void loop() {
  vTaskDelay(1000);
}

