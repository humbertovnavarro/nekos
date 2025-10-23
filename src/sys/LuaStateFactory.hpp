#include "lua.hpp"

#include "modules/os/LuaFreeRTOSModule.hpp"
#include "modules/peripherals/LuaSerialModule.hpp"
#include "modules/os/LuaFileSystemModule.hpp"
#include "modules/peripherals/LuaGPIOModule.hpp"
#include "modules/peripherals/LuaNeoPixelModule.hpp"
#include "modules/peripherals//LuaDisplayModule.hpp"
#include "modules/os/LuaRequireModule.hpp"

inline lua_State* luaStateFactory(lua_State* L) {
    luaL_openlibs(L);
    luaRequireModule.expose(L);
    luaFreeRTOSModule.expose(L);
    luaFileSystemModule.expose(L);
    luaSerialModule.expose(L);
    luaGpioModule.expose(L);
    luaNeoPixelModule.expose(L);
    luaDisplayModule.expose(L);
    return L;
}