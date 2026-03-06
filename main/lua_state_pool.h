#pragma once
extern "C" {
#include "lua.h"
}
#ifndef LUA_POOL_SIZE
  #define LUA_POOL_SIZE 4
#endif

struct LuaStateSlot {
    int        index = -1;
    lua_State *L     = nullptr;
    bool       busy  = false;
};

void          lua_pool_init();
LuaStateSlot *lua_pool_acquire();
void          lua_pool_release(LuaStateSlot *slot);