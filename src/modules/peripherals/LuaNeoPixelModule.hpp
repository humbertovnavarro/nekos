#pragma once
#include "lua.hpp"
#include "sys/LuaModule.hpp"
#include "drivers/neopixel.hpp"
// =======================================
// Lua Module Definition
// =======================================
LuaModule luaNeoPixelModule("neopixel", [](lua_State* L) {
    LuaModule::begin(L);
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

    return 1;
});
