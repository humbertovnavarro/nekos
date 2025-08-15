#include "Arduino.h"
#include "WiFi.h"
#include "config/device.h"
#include "nekos/NekosFS.h"
#include "nekos/NekosCoreCommands.h"
#include "nekos/NekosConsole.h"
#include <U8g2lib.h>
#include <Wire.h>
#include "nekos/NekosRegistry.h"
#include "MUIU8g2.h"
#include <Adafruit_NeoPixel.h>
#include "tusb.h"
// Display setup
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

void setup() {

    nekos::fs::init();
    nekos::registerCoreCommands();
    nekos::Console::begin(BAUD);
    nekos::Registry::set(REGKEY_CWD, "/");
    Wire.begin();
    u8g2.begin();
    u8g2.setFont(u8g2_font_pressstart2p_8f);
}

void loop() {
    vTaskDelay(100);
}
