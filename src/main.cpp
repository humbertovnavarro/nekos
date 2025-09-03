#include "nekos/NekosLuaScheduler.h"
#include "nekos/NekosHTTP.h"
void setup() {
  nekos::LuaScheduler::begin(32);
  nekos::http::begin();
}

void loop() {
  vTaskDelay(100);
}
