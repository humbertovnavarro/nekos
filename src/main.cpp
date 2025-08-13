#include "Arduino.h"
#include "WiFi.h"
#include "config/device.h"
#include "NekosFS.h"
#include "NekosConsole.h"
#include "NekosCoreUtils.h"
#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE); 

void setup(void) {
    nekos::fs::init();
    nekos::registerCoreUtils();
    nekos::Console::begin(BAUD);
    Wire.begin(); // Initialize I2C communication
    u8g2.begin(); // Initialize the display
    u8g2.setFont(u8g2_font_ncenB14_tr); // Set a font for display
}

void loop(void) {
  u8g2.firstPage(); // Start the drawing process (for full buffer mode)
  do {
    // All drawing commands go inside this do-while loop
    u8g2.drawStr(0, 20, "Hello World!"); // Draw text at position (0, 20)
  } while (u8g2.nextPage()); // Move to the next page of the display buffer
  delay(1000); // Wait for 1 second before redrawing
}