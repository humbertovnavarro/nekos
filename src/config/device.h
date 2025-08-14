#pragma once
#include "Arduino.h"
#define REBOOT_IN_MS 1000
#define NEKOS_STDIO_NUM_CHARS 12
#define NEKOS_STDIO_BUFFER_COUNT 5
#ifdef EMULATOR
    #define HAS_DISPLAY
    #define SSD1306I2C
    #define MENU_SELECT_PIN 40
    #define MENU_NEXT_PIN 39
    #define MENU_PREV_PIN 38
    #define MENU_HOME_PIN 37
    #define HAS_WIFI
#endif

#ifdef WIFI_USB_FELINE
    #define HAS_DISPLAY
    #define SH1106I2C
    #define MENU_SELECT_PIN 3
    #define MENU_NEXT_PIN 5
    #define MENU_PREV_PIN 6
    #define MENU_HOME_PIN 7
    #define BAUD 921600
    #define THREADS 2
    #define NEOPIXEL_PIN 12
    #define HAS_WIFI
#endif

#ifdef HELTEC_LORA_V3
    #define HAS_WIFI
    #define HAS_DISPLAY
    #define SSD1306I2C
    #define GPIO_CLK 18
    #define GPIO_DATA 17
    #define GPIO_RESET 21
    #define SW
    #define MENU_SELECT_PIN 7
    #define MENU_NEXT_PIN 18
    #define MENU_PREV_PIN 9
    #define MENU_HOME_PIN 11
    #define MONO_INPUT
    #define BAUD 115200
#endif