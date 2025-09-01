extern "C" {
    #include "lua.h"
    #include "lualib.h"
    #include "lauxlib.h"
}
namespace nekos {
    void luaRegisterNekosMetaBindings(lua_State* L);
}