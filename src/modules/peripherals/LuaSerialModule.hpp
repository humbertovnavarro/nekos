#include "Arduino.h"
#include "../LuaModule.hpp"
inline LuaModule serialModule("serial", [](lua_State* L) {
    LuaModule::begin(L);
    LuaModule::addFunction(L, "println", [](lua_State* L) -> int {
        Serial.println(luaL_checkstring(L, 1));
        return 0;
    });
    LuaModule::addFunction(L, "available", [](lua_State* L) -> int {
        lua_pushinteger(L, Serial.available());
        return 1;
    });
    LuaModule::addInteger(L, "BAUD_115200", 115200);
    LuaModule::addString(L, "device", "UART0");
    LuaModule::addBoolean(L, "enabled", true);
});

inline void registerSerialModule() {
    registerLuaModule(serialModule);
}