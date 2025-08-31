#include "Arduino.h"
#include "nekos/Nekos.h"
#include "nekos/NekosConsole.h"
#include "nekos/NekosLuaLoader.h"
void setup() {
}

void loop() {
  nekos::NekosLuaLoader::begin();
  nekos::Console::poll();
  vTaskDelay(1);
}