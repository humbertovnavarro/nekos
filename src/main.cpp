#include "Arduino.h"
#include "config/device.h"
#include "nekos/NekosFS.h"
#include "nekos/NekosCoreCommands.h"
#include "nekos/NekosConsole.h"
#include "nekos/NekosLua.h"

void setup() {
    nekos::Console::begin(BAUD);
    delay(10000);
    nekos::fs::init(true);
    nekos::registerCoreCommands();
    if(nekos::fs::fileExists("boot.lua")) {
        String bootScript = nekos::fs::readFile("boot.lua");
        nekos::luaExec(bootScript.c_str());
    }
}

void loop() {
    vTaskDelay(100);
}
