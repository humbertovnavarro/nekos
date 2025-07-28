#pragma once
#ifdef EMULATOR
    #define SSD1306I2C
    #define MENU_SELECT_PIN 40
    #define MENU_NEXT_PIN 39
    #define MENU_PREV_PIN 38
    #define MENU_HOME_PIN 37
#endif

#ifdef WIFI_USB_NUGGET
    #define HAS_DISPLAY
    #define SH1106I2C
    #define MENU_SELECT_PIN 7
    #define MENU_NEXT_PIN 18
    #define MENU_PREV_PIN 9
    #define MENU_HOME_PIN 11
#endif

#ifdef HELTEC_LORA_V3
    #define HAS_DISPLAY
    #define SSD1306I2C
#endif