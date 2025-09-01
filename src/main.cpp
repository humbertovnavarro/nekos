#include "Arduino.h"
#include "nekos/Nekos.h"
#include "nekos/NekosConsole.h"
#include "nekos/NekosLuaLoader.h"
#include "FFat.h"
void setup() {
  FFat.begin(true, "/", 15, "lua");
  nekos::Console::begin();
}

void loop() {
  nekos::NekosLuaLoader::begin();
  nekos::Console::poll();
  vTaskDelay(10);
}