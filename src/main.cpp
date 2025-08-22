#include "Arduino.h"
#include "config/device.h"
#include "nekos/NekosFS.h"
#include "nekos/NekosCoreCommands.h"
#include "nekos/NekosConsole.h"
#include "nekos/NekosRegistry.h"
#include "nekos/NekosLua.h"

void setup() {
    nekos::fs::init();
    nekos::registerCoreCommands();
    nekos::Console::begin(BAUD);
    nekos::Registry::set(REGKEY_CWD, "/");
    if(nekos::fs::fileExists("boot.lua")) {
        String bootScript = nekos::fs::readFile("boot.lua");
        nekos::luaExec(bootScript.c_str());
    }
}

void loop() {
    vTaskDelay(100);
}
