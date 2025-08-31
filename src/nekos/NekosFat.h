#pragma once

extern "C" {
    #include "lua.h"
}

namespace nekos {

    // -------------------------------
    // FAT filesystem Lua bindings
    // -------------------------------
    int luaFAT_exists(lua_State* L);
    int luaFAT_remove(lua_State* L);
    int luaFAT_read(lua_State* L);
    int luaFAT_write(lua_State* L);
    int luaFAT_append(lua_State* L);
    int luaFAT_ls(lua_State* L);
    // Registers all FFat bindings under the global table FFat
    void registerFFatBindings(lua_State* L);

}
