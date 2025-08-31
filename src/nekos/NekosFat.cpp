extern "C" {
    #include "lua.h"
    #include "lualib.h"
    #include "lauxlib.h"
}
#include "FFat.h"
#include "NekosFat.h"

namespace nekos {
    // -------------------------------
    // FAT filesystem bindings
    // -------------------------------
    static int luaFFatexists(lua_State* L) {
        const char* path = luaL_checkstring(L, 1);
        lua_pushboolean(L, FFat.exists(path));
        return 1;
    }

    static int luaFFatremove(lua_State* L) {
        const char* path = luaL_checkstring(L, 1);
        lua_pushboolean(L, FFat.remove(path));
        return 1;
    }

    static int luaFFatread(lua_State* L) {
        const char* path = luaL_checkstring(L, 1);
        File f = FFat.open(path, FILE_READ);
        if (!f) { lua_pushnil(L); return 1; }

        String content = f.readString();
        f.close();
        lua_pushstring(L, content.c_str());
        return 1;
    }

    static int luaFFatwrite(lua_State* L) {
        const char* path = luaL_checkstring(L, 1);
        const char* data = luaL_checkstring(L, 2);
        File f = FFat.open(path, FILE_WRITE, true);
        lua_pushboolean(L, f && f.print(data) >= 0);
        if (f) f.close();
        return 1;
    }

    static int luaFFatappend(lua_State* L) {
        const char* path = luaL_checkstring(L, 1);
        const char* data = luaL_checkstring(L, 2);
        File f = FFat.open(path, FILE_APPEND, true);
        lua_pushboolean(L, f && f.print(data) >= 0);
        if (f) f.close();
        return 1;
    }

    static int luaFFatls(lua_State* L) {
        File root = FFat.open("/");
        File file = root.openNextFile();
        lua_newtable(L);
        int index = 1;
        while (file) {
            lua_pushnumber(L, index++);
            lua_pushstring(L, file.name());
            lua_settable(L, -3);
            file = root.openNextFile();
        }
        return 1;
    }
    
    void registerFFatBindings(lua_State* L) {
        lua_newtable(L);
        lua_pushcfunction(L, luaFFatexists);   lua_setfield(L, -2, "exists");
        lua_pushcfunction(L, luaFFatremove);   lua_setfield(L, -2, "remove");
        lua_pushcfunction(L, luaFFatread);     lua_setfield(L, -2, "read");
        lua_pushcfunction(L, luaFFatwrite);    lua_setfield(L, -2, "write");
        lua_pushcfunction(L, luaFFatappend);   lua_setfield(L, -2, "append");
        lua_pushcfunction(L, luaFFatls);       lua_setfield(L, -2, "ls");
        lua_setglobal(L, "FFat");
    }

}