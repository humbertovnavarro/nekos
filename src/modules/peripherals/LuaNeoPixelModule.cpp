#include "Adafruit_NeoPixel.h"
#include "lua.hpp"
#include "LuaNeoPixelModule.hpp"

Adafruit_NeoPixel neopixel(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

int luaNeoSetPixel(lua_State* L) {
    int index = luaL_checkinteger(L, 1);
    int r = luaL_checkinteger(L, 2);
    int g = luaL_checkinteger(L, 3);
    int b = luaL_checkinteger(L, 4);
    if (index >= 0 && index < neopixel.numPixels()) {
        neopixel.setPixelColor(index, neopixel.Color(r, g, b));
    }
    return 0;
}

int luaNeoShow(lua_State* _) {
    neopixel.show();
    return 0;
}

int luaNeoClear(lua_State* _) {
    neopixel.clear();
    return 0;
}

int luaNeoSetBrightness(lua_State* L) {
    int level = luaL_checkinteger(L, 1);
    neopixel.setBrightness(level);
    return 0;
}


void luaOpenNeopixelLibs(lua_State* L) {
    lua_newtable(L);
    lua_pushcfunction(L, luaNeoSetPixel);
    lua_setfield(L, -2, "setPixel");
    lua_pushcfunction(L, luaNeoShow);
    lua_setfield(L, -2, "show");
    lua_pushcfunction(L, luaNeoClear);
    lua_setfield(L, -2, "clear");
    lua_pushcfunction(L, luaNeoSetBrightness);
    lua_setfield(L, -2, "setBrightness");
    lua_setglobal(L, "neopixel");
}
