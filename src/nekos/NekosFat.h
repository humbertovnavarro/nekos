#pragma once

extern "C" {
    #include "lua.h"
}

namespace nekos {
    // -------------------------------
    // FAT filesystem Lua bindings
    // -------------------------------
    int luaFFATexists(lua_State* L);
    int luaFFATremove(lua_State* L);
    int luaFFATread(lua_State* L);
    int luaFFATwrite(lua_State* L);
    int luaFFATappend(lua_State* L);
    int luaFFATls(lua_State* L);
    // Registers all FFat bindings under the global table FFat
    void registerFFatBindings(lua_State* L);

}
