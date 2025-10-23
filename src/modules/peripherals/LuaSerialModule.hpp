#include "Arduino.h"
#include "sys/LuaModule.hpp"

inline LuaModule luaSerialModule("serial", [](lua_State* L) {
    LuaModule::begin(L);

    // ----------------------------
    // serial.println(str)
    // ----------------------------
    LuaModule::addFunction(L, "println", [](lua_State* L) -> int {
        Serial.println(luaL_checkstring(L, 1));
        return 0;
    });

    // ----------------------------
    // serial.print(value)
    // ----------------------------
    LuaModule::addFunction(L, "print", [](lua_State* L) -> int {
        if (lua_isstring(L, 1)) {
            Serial.print(luaL_checkstring(L, 1));
        } else if (lua_isnumber(L, 1)) {
            double n = luaL_checknumber(L, 1);
            Serial.print(n);
        } else if (lua_isboolean(L, 1)) {
            bool b = lua_toboolean(L, 1);
            Serial.print(b ? 1 : 0);
        } else {
            return 1;
        }
        return 0;
    });

    // ----------------------------
    // serial.available() -> int
    // ----------------------------
    LuaModule::addFunction(L, "available", [](lua_State* L) -> int {
        lua_pushinteger(L, Serial.available());
        return 1;
    });

    // ----------------------------
    // serial.read() -> int or nil
    // ----------------------------
    LuaModule::addFunction(L, "read", [](lua_State* L) -> int {
        if (Serial.available()) {
            int c = Serial.read();
            lua_pushinteger(L, c);
        } else {
            lua_pushnil(L);
        }
        return 1;
    });

    // ----------------------------
    // serial.readLine() -> string or nil
    // ----------------------------
    LuaModule::addFunction(L, "readLine", [](lua_State* L) -> int {
        if (!Serial.available()) {
            lua_pushnil(L);
            return 1;
        }
        String line = Serial.readStringUntil('\n');
        line.trim(); // remove \r and whitespace
        lua_pushlstring(L, line.c_str(), line.length());
        return 1;
    });

    // ----------------------------
    // serial.readString([len]) -> string or nil
    // ----------------------------
    LuaModule::addFunction(L, "readString", [](lua_State* L) -> int {
        int maxLen = luaL_optinteger(L, 1, 0); // 0 = read all available
        if (!Serial.available()) {
            lua_pushnil(L);
            return 1;
        }

        String s;
        if (maxLen > 0) {
            s = Serial.readStringUntil('\n');
            if (s.length() > maxLen) s = s.substring(0, maxLen);
        } else {
            s = Serial.readString();
        }

        lua_pushlstring(L, s.c_str(), s.length());
        return 1;
    });
});
