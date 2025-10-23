#include "Adafruit_NeoPixel.h"
#include "lua.hpp"
#include "sys/LuaModule.hpp"

// =======================================
// NeoPixel Setup
// =======================================
#ifndef LED_PIN
#define LED_PIN 4
#endif

#ifndef LED_COUNT
#define LED_COUNT 8
#endif

static Adafruit_NeoPixel neopixel(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// =======================================
// Lua Module Definition
// =======================================
inline LuaModule luaNeopixelModule("neopixel", [](lua_State* L) {
    LuaModule::begin(L);

    // Initialize the LED strip
    neopixel.begin();
    neopixel.show(); // Clear all LEDs on start

    LuaModule::addFunction(L, "setPixel", [](lua_State* L) -> int {
        int index = luaL_checkinteger(L, 1);
        int r = luaL_checkinteger(L, 2);
        int g = luaL_checkinteger(L, 3);
        int b = luaL_checkinteger(L, 4);

        if (index >= 0 && index < neopixel.numPixels()) {
            neopixel.setPixelColor(index, neopixel.Color(r, g, b));
        }
        return 0;
    });

    LuaModule::addFunction(L, "show", [](lua_State* L) -> int {
        neopixel.show();
        return 0;
    });

    LuaModule::addFunction(L, "clear", [](lua_State* L) -> int {
        neopixel.clear();
        return 0;
    });

    LuaModule::addFunction(L, "setBrightness", [](lua_State* L) -> int {
        int level = luaL_checkinteger(L, 1);
        neopixel.setBrightness(level);
        return 0;
    });

    LuaModule::addFunction(L, "numPixels", [](lua_State* L) -> int {
        lua_pushinteger(L, neopixel.numPixels());
        return 1;
    });
});
