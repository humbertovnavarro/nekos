#include "lua.hpp"
#include "Adafruit_NeoPixel.h"
#define GREEN         0,0,5,0
#define RED           0,5,0,0
#define BLUE          0,0,0,5
#define WHITE         0,2,2,2
#define YELLOW        0,2,2,0
#define OFF           0,0,0,0
#define BRIGHT_GREEN  0,0,20,0
#define BRIGHT_RED    0,20,0,0
#define BRIGHT_BLUE   0,0,0,20
#define BRIGHT_WHITE  0,35,35,35
#define FLASHLIGHT    0,255,255,255
#define LED_PIN 12
#define LED_COUNT 1
#define SCL 35
#define SDA 33
extern Adafruit_NeoPixel neopixel;
void luaOpenNeopixelLibs(lua_State* L);