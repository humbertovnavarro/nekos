#pragma once
#include "sys/LuaModule.hpp"
#include "drivers/u8g2.hpp"
LuaModule luaDisplayModule("display", [](lua_State* L) {
    LuaModule::begin(L);

    LuaModule::addFunction(L, "clear", [](lua_State* L) -> int {
        u8g2.clearBuffer();
        return 0;
    });

    LuaModule::addFunction(L, "drawStr", [](lua_State* L) -> int {
        int x = luaL_checkinteger(L, 1);
        int y = luaL_checkinteger(L, 2);
        const char* str = luaL_checkstring(L, 3);
        u8g2.drawStr(x, y, str);
        return 0;
    });

    LuaModule::addFunction(L, "sendBuffer", [](lua_State* L) -> int {
        u8g2.sendBuffer();
        return 0;
    });

    LuaModule::addFunction(L, "setFont", [](lua_State* L) -> int {
        const char* fontName = luaL_checkstring(L, 1);
        if (strcmp(fontName, "u8g2_font_ncenB08_tr") == 0)
            u8g2.setFont(u8g2_font_ncenB08_tr);
        else if (strcmp(fontName, "u8g2_font_6x10_tr") == 0)
            u8g2.setFont(u8g2_font_6x10_tr);
        else
            Serial.println("Unknown font for Lua");
        return 0;
    });

    LuaModule::addFunction(L, "drawBox", [](lua_State* L) -> int {
        int x = luaL_checkinteger(L, 1);
        int y = luaL_checkinteger(L, 2);
        int w = luaL_checkinteger(L, 3);
        int h = luaL_checkinteger(L, 4);
        u8g2.drawBox(x, y, w, h);
        return 0;
    });

    LuaModule::addFunction(L, "drawFrame", [](lua_State* L) -> int {
        int x = luaL_checkinteger(L, 1);
        int y = luaL_checkinteger(L, 2);
        int w = luaL_checkinteger(L, 3);
        int h = luaL_checkinteger(L, 4);
        u8g2.drawFrame(x, y, w, h);
        return 0;
    });
    return 1;
});
