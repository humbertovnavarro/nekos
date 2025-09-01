extern "C" {
    #include "lua.h"
    #include "lualib.h"
    #include "lauxlib.h"
}
#include "LuaScripts.h"
#include "NekosMeta.h"

namespace nekos {

    int luaGetScripts(lua_State* L) {
        if (!luaScriptMap) {
            initLuaScriptMap();
        }
        lua_newtable(L);
        int i = 1;
        for (const auto& kv : *luaScriptMap) {
            lua_pushstring(L, kv.first.c_str());
            lua_rawseti(L, -2, i++);
        }
        return 1;
    }

    void luaRegisterNekosMetaBindings(lua_State* L) {
        lua_pushcfunction(L, luaGetScripts);
        lua_setglobal(L, "getScripts");
    }
}