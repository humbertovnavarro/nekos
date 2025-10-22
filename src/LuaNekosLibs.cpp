#include "Arduino.h"
#include "freertos/portmacro.h"
#include "lua.hpp"
#include "projdefs.h"
#include "display.hpp"
#include "neopixel.hpp"
#include "sys.hpp"

void LuaOpenNekosLibs(lua_State* L) {
    lua_pushcfunction(L, [](lua_State* L) -> int {
        int ms = luaL_checkinteger(L, 1);
        vTaskDelay(pdMS_TO_TICKS(ms));
        return 0;
    });
    lua_setglobal(L, "delay");
    lua_pushcfunction(L, [](lua_State* L) -> int {
        TickType_t ticks = static_cast<TickType_t>(luaL_checkinteger(L, 1));
        vTaskDelay(ticks);
        return 0;
    });
    lua_setglobal(L, "delayTicks");
    luaOpenDisplayLibs(L);
    luaOpenSysLibs(L);
    luaOpenNeopixelLibs(L);
}
