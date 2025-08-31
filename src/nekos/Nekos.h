#define LUA_CONST(L, name, value) \
    lua_pushinteger(L, value);    \
    lua_setglobal(L, name);
    
namespace nekos {
    void luaExec(const char* lua, const char* args);
}