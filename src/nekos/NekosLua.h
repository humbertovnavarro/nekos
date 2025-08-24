#define LUA_CONST(L, name, value) \
    lua_pushinteger(L, value);    \
    lua_setglobal(L, name);
    
namespace nekos {
    String luaExec(const char* lua);
}