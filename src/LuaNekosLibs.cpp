#include "lua.hpp"
#include "LuaNekosLibs.hpp"
#include "display.hpp"
#include "sys.hpp"
#include "neopixel.hpp"
void LuaOpenNekosLibs(lua_State* L) {
    lua_pushcfunction(L, [](lua_State* L) -> int {
        int n = lua_gettop(L);
        for (int i = 1; i <= n; ++i) {
            if (lua_isstring(L, i)) Serial.print(lua_tostring(L, i));
            else if (lua_isnumber(L, i)) Serial.print(lua_tonumber(L, i));
            else if (lua_isboolean(L, i)) Serial.print(lua_toboolean(L, i) ? "true" : "false");
            else Serial.print(luaL_typename(L, i));
            if (i < n) Serial.print("\t");
        }
        Serial.println();
        return 0;
    });
    lua_setglobal(L, "print");
    luaOpenDisplayLibs(L);
    luaOpenSysLibs(L);
    luaOpenNeopixelLibs(L);
}
