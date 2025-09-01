#define LUA_CONST(L, name, value) \
    lua_pushinteger(L, value);    \
    lua_setglobal(L, name);
    
extern "C" {
    #include "lua.h"
    #include "lualib.h"
    #include "lauxlib.h"
}    
namespace nekos {
    void luaExec(const char* lua, const char* args);
    void luaRegisterBindings(lua_State* L);
    bool luaRunReplLine(lua_State* L, const char* line);
}