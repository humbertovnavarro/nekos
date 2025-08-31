#include <Arduino.h>
#include "NekosSerial.h"
extern "C" {
    #include "lua.h"
    #include "lualib.h"
    #include "lauxlib.h"
}
namespace nekos {

int luaSerialBegin(lua_State* L) {
    int baud = luaL_checkinteger(L, 1);
    Serial.begin(baud);
    return 0;
}

int luaSerialPrint(lua_State* L) {
    const char* str = luaL_checkstring(L, 1);
    Serial.print(str);
    return 0;
}

int luaSerialPrintln(lua_State* L) {
    const char* str = luaL_checkstring(L, 1);
    Serial.println(str);
    return 0;
}

int luaSerialAvailable(lua_State* L) {
    lua_pushinteger(L, Serial.available());
    return 1;
}

int luaSerialRead(lua_State* L) {
    if (Serial.available() > 0) {
        lua_pushinteger(L, Serial.read());
    } else {
        lua_pushnil(L);
    }
    return 1;
}

int luaSerialWrite(lua_State* L) {
    int byte = luaL_checkinteger(L, 1);
    Serial.write((uint8_t)byte);
    return 0;
}

// ----------------- New: printf -----------------
int luaSerialPrintf(lua_State* L) {
    // Forward arguments to Lua's string.format
    lua_getglobal(L, "string");        // push string lib
    lua_getfield(L, -1, "format");     // push string.format
    lua_remove(L, -2);                 // remove string table

    int argc = lua_gettop(L) - 1;      // exclude self
    lua_insert(L, 1);                  // put format at bottom
    lua_call(L, argc, 1);              // call string.format(...)

    const char* str = lua_tostring(L, -1);
    if (str) {
        Serial.printf("%s", str);
    }
    lua_pop(L, 1);
    return 0;
}
// -----------------------------------------------

void registerSerialBindings(lua_State* L) {
    lua_newtable(L);

    lua_pushcfunction(L, luaSerialBegin);
    lua_setfield(L, -2, "begin");

    lua_pushcfunction(L, luaSerialPrint);
    lua_setfield(L, -2, "print");

    lua_pushcfunction(L, luaSerialPrintln);
    lua_setfield(L, -2, "println");

    lua_pushcfunction(L, luaSerialAvailable);
    lua_setfield(L, -2, "available");

    lua_pushcfunction(L, luaSerialRead);
    lua_setfield(L, -2, "read");

    lua_pushcfunction(L, luaSerialWrite);
    lua_setfield(L, -2, "write");

    lua_pushcfunction(L, luaSerialPrintf);
    lua_setfield(L, -2, "printf");

    lua_setglobal(L, "Serial");  // expose as global Serial

    // ----------------- override global print -----------------
    lua_pushcfunction(L, [](lua_State* L) -> int {
        int n = lua_gettop(L);
        for (int i = 1; i <= n; i++) {
            size_t len;
            const char* s = luaL_tolstring(L, i, &len);
            if (i > 1) Serial.print("\t");
            Serial.print(s);
            lua_pop(L, 1); // remove luaL_tolstring result
        }
        Serial.println();
        return 0;
    });
    lua_setglobal(L, "print");
}

} // namespace nekos
