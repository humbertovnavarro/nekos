#pragma once
#include "Arduino.h"
#include "sys/LuaModule.hpp"

// ============================
// GPIO Lua Module
// ============================

LuaModule luaGpioModule("gpio", [](lua_State* L) {
    LuaModule::begin(L);

    // gpio.mode(pin, mode)
    // mode = "input", "output", "input_pullup"
    LuaModule::addFunction(L, "mode", [](lua_State* L) -> int {
        int pin = luaL_checkinteger(L, 1);
        const char* mode = luaL_checkstring(L, 2);

        if (strcmp(mode, "input") == 0)
            pinMode(pin, INPUT);
        else if (strcmp(mode, "output") == 0)
            pinMode(pin, OUTPUT);
        else if (strcmp(mode, "input_pullup") == 0)
            pinMode(pin, INPUT_PULLUP);
        else
            Serial.println("Unknown GPIO mode");

        return 0;
    });

    // gpio.write(pin, value)
    LuaModule::addFunction(L, "write", [](lua_State* L) -> int {
        int pin = luaL_checkinteger(L, 1);
        int value = lua_toboolean(L, 2);
        digitalWrite(pin, value ? HIGH : LOW);
        return 0;
    });

    // value = gpio.read(pin)
    LuaModule::addFunction(L, "read", [](lua_State* L) -> int {
        int pin = luaL_checkinteger(L, 1);
        int value = digitalRead(pin);
        lua_pushboolean(L, value == HIGH);
        return 1;
    });

    // gpio.toggle(pin)
    LuaModule::addFunction(L, "toggle", [](lua_State* L) -> int {
        int pin = luaL_checkinteger(L, 1);
        int current = digitalRead(pin);
        digitalWrite(pin, !current);
        return 0;
    });

    return 1;
});
