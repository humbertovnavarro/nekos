#include "lua.hpp"
#include "Esp.h"
#include "esp32-hal.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "sys/LuaModule.hpp"

// ===================================
// Lua System Module (ESP32)
// ===================================
inline LuaModule sysLuaModule("sys", [](lua_State* L) {
    LuaModule::begin(L);

    LuaModule::addFunction(L, "uptime", [](lua_State* L) -> int {
        lua_pushinteger(L, millis());
        return 1;
    });

    LuaModule::addFunction(L, "freeHeap", [](lua_State* L) -> int {
        lua_pushinteger(L, esp_get_free_heap_size());
        return 1;
    });

    LuaModule::addFunction(L, "heapFragmentation", [](lua_State* L) -> int {
        lua_pushnumber(L, heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
        return 1;
    });

    LuaModule::addFunction(L, "cpuCores", [](lua_State* L) -> int {
    #ifdef ESP32
        lua_pushinteger(L, portNUM_PROCESSORS);
    #else
        lua_pushinteger(L, 1);
    #endif
        return 1;
    });

    LuaModule::addFunction(L, "cpuFreqMHz", [](lua_State* L) -> int {
        lua_pushinteger(L, ESP.getCpuFreqMHz());
        return 1;
    });

    LuaModule::addFunction(L, "chipId", [](lua_State* L) -> int {
    #ifdef ESP32
        lua_pushinteger(L, (uint32_t)ESP.getEfuseMac());
    #else
        lua_pushinteger(L, 0);
    #endif
        return 1;
    });

    LuaModule::addFunction(L, "version", [](lua_State* L) -> int {
        lua_pushstring(L, ARDUINO_VARIANT);
        return 1;
    });

    LuaModule::addFunction(L, "flashSize", [](lua_State* L) -> int {
        uint32_t flashSize = 0;
        esp_flash_get_size(nullptr, &flashSize);
        lua_pushinteger(L, flashSize);
        return 1;
    });

    LuaModule::addFunction(L, "sdkVersion", [](lua_State* L) -> int {
        lua_pushstring(L, ESP.getSdkVersion());
        return 1;
    });
});
