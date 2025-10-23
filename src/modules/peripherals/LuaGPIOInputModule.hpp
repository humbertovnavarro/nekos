#pragma once
#include "Arduino.h"
#include "lua.hpp"
#include "sys/LuaModule.hpp"
#include "drivers/input.hpp"

LuaModule luaInputModule("input", [](lua_State* L) {
    LuaModule::begin(L);
    LuaModule::addFunction(L, "read", [](lua_State* L) -> int {
        Input dir = getInputDirection();
        const char* result = "none";
        switch (dir) {
            case UP:    result = "up"; break;
            case DOWN:  result = "down"; break;
            case LEFT:  result = "left"; break;
            case RIGHT: result = "right"; break;
            default:    result = "none"; break;
        }
        lua_pushstring(L, result);
        return 1;
    });

    return 1;
});
