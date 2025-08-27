#include "Arduino.h"
#include "nekos/NekosFS.h"
#include "nekos/NekosConsole.h"
#include "nekos/NekosCoreCommands.h"
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  nekos::fs::begin();
  nekos::Console::begin();
  nekos::registerCoreCommands();
}

void loop() {
  vTaskDelay(1000);
}

