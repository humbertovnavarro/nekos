#pragma once

extern "C" {
    #include "lua.h"
}

namespace nekos {
    // Arduino Serial <-> Lua bindings
    int luaSerialBegin(lua_State* L);      // Serial.begin(baud)
    int luaSerialPrint(lua_State* L);      // Serial.print(str)
    int luaSerialPrintln(lua_State* L);    // Serial.println(str)
    int luaSerialAvailable(lua_State* L);  // Serial.available()
    int luaSerialRead(lua_State* L);       // Serial.read()
    int luaSerialReadLine(lua_State* L);   // Serial.readLine()
    int luaSerialWrite(lua_State* L);      // Serial.write(byte)
    int luaSerialPrintf(lua_State* L);
    void registerSerialBindings(lua_State* L);
}
