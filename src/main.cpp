#include "Arduino.h"
#include "nekos/NekosCoreCommands.h"
#include "nekos/NekosConsole.h"
#include "nekos/NekosRegistry.h"

void setup() {
    nekos::registerCoreCommands();
    nekos::Console::begin();
    nekos::Registry::set(REGKEY_CWD, "/");
}

void loop() {
    vTaskDelay(100);
}
