#include "Arduino.h"
#include "config/device.h"
#include "nekos/NekosFS.h"
#include "nekos/NekosCoreCommands.h"
#include "nekos/NekosConsole.h"
#include "nekos/NekosRegistry.h"

void setup() {
    nekos::fs::init();
    nekos::registerCoreCommands();
    nekos::Console::begin(BAUD);
    nekos::Registry::set(REGKEY_CWD, "/");
}

void loop() {
    vTaskDelay(100);
}
