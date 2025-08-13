#include "Arduino.h"
#include "WiFi.h"
#include "config/device.h"
#include "NekosFS.h"
#include "NekosConsole.h"
#include "NekosCoreUtils.h"
void setup(void)
{
    nekos::fs::init();
    nekos::registerCoreUtils();
    nekos::Console::begin(BAUD);
}

void loop(void) {
    delay(1000);
}