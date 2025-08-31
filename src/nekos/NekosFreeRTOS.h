#pragma once

extern "C" {
    #include "lua.h"
}

namespace nekos {
    // FreeRTOS Lua bindings
    int luaDelay(lua_State* L);        // delay in ms
    int luaYield(lua_State* L);        // yield to scheduler
    int luaMillis(lua_State* L);       // get system uptime
    int luaCreateTask(lua_State* L);   // create new task running a Lua function
    void registerFreeRTOSBindings(lua_State* L);
}
