#include "Arduino.h"
#include "WiFi.h"
#include "config/device.h"
#include "nekos/NekosFS.h"
#include "nekos/NekosCoreCommands.h"
#include "nekos/NekosConsole.h"
#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include "nekos/NekosRegistry.h"
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE); 

void setup(void) {
    nekos::fs::init();
    nekos::registerCoreCommands();
    nekos::Console::begin(BAUD);
    nekos::Registry::set(REGKEY_CWD, "/");
    Wire.begin();
    u8g2.begin();
    u8g2.setFont(u8g2_font_pressstart2p_8f);
}

void loop(void) {
    static size_t startIndex = 0;
    const int maxLines = 5;
    const int lineHeight = 10;
    auto& commands = nekos::Console::commands.commandMap;
    size_t totalCommands = commands.size();
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_ncenB08_tr);
        int y = 10;
        u8g2.drawStr(0, y, "115200 on /dev/ttyACM0");
        y += lineHeight;

        size_t shown = 0;
        size_t index = startIndex;

        for (; shown < maxLines && index < totalCommands; shown++, index++) {
            auto it = std::next(commands.begin(), index);
            u8g2.drawStr(0, y, it->first.c_str());
            y += lineHeight;
        }
    } while (u8g2.nextPage());
    // Update startIndex for next batch
    startIndex += maxLines;
    if (startIndex >= nekos::Console::commands.commandMap.size()) {
        startIndex = 0; // wrap around
    }

    delay(5000); // wait 5 seconds before showing next batch
}