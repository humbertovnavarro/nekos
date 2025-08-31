#include "NekosFreeRTOS.h"
#include <Arduino.h>
#include "FreeRTOS.h"
#include "task.h"

extern "C" {
    #include "lua.h"
    #include "lualib.h"
    #include "lauxlib.h"
}

namespace nekos {

// delay(ms)
int luaDelay(lua_State* L) {
    int ms = luaL_checkinteger(L, 1);
    vTaskDelay(pdMS_TO_TICKS(ms));
    return 0;
}

// yield()
int luaYield(lua_State* L) {
    taskYIELD();
    return 0;
}

// millis()
int luaMillis(lua_State* L) {
    lua_pushinteger(L, millis());
    return 1;
}

// register bindings
void registerFreeRTOSBindings(lua_State* L) {
    lua_newtable(L);
    lua_pushcfunction(L, luaDelay);   lua_setfield(L, -2, "delay");
    lua_pushcfunction(L, luaYield);   lua_setfield(L, -2, "yield");
    lua_pushcfunction(L, luaMillis);  lua_setfield(L, -2, "millis");
    lua_setglobal(L, "FreeRTOS");
}

} // namespace nekos
