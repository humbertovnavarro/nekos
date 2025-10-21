#include "sys.hpp"
#include "Esp.h"
#include "esp32-hal.h"
#include "esp_flash.h"
#include "esp_system.h"

// ================================
// System info functions
// ================================
static int luaSysUptime(lua_State* L) {
    lua_pushinteger(L, millis());
    return 1;
}

static int luaSysFreeHeap(lua_State* L) {
    lua_pushinteger(L, esp_get_free_heap_size());
    return 1;
}

static int luaSysHeapFragmentation(lua_State* L) {
    lua_pushnumber(L, heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
    return 1;
}

static int luaSysCpuCores(lua_State* L) {
#ifdef ESP32
    lua_pushinteger(L, portNUM_PROCESSORS);
#else
    lua_pushinteger(L, 1);
#endif
    return 1;
}

static int luaSysCpuFreq(lua_State* L) {
    lua_pushinteger(L, ESP.getCpuFreqMHz());
    return 1;
}

static int luaSysChipId(lua_State* L) {
#ifdef ESP32
    lua_pushinteger(L, (uint32_t)ESP.getEfuseMac());
#else
    lua_pushinteger(L, 0);
#endif
    return 1;
}

static int luaSysVersion(lua_State* L) {
    lua_pushstring(L, ARDUINO_VARIANT);
    return 1;
}

static int luaSysFlashSize(lua_State* L) {
    uint32_t flashSize = 0;
    esp_flash_get_size(nullptr, &flashSize);
    lua_pushinteger(L, flashSize);
    return 1;
}

static int luaSysSdkVersion(lua_State* L) {
    lua_pushstring(L, ESP.getSdkVersion());
    return 1;
}

// ================================
// Register functions
// ================================
void luaOpenSysLibs(lua_State* L) {
    lua_newtable(L);

    lua_pushcfunction(L, luaSysUptime);
    lua_setfield(L, -2, "uptime");

    lua_pushcfunction(L, luaSysFreeHeap);
    lua_setfield(L, -2, "freeHeap");

    lua_pushcfunction(L, luaSysHeapFragmentation);
    lua_setfield(L, -2, "heapFragmentation");

    lua_pushcfunction(L, luaSysCpuCores);
    lua_setfield(L, -2, "cpuCores");

    lua_pushcfunction(L, luaSysCpuFreq);
    lua_setfield(L, -2, "cpuFreqMHz");

    lua_pushcfunction(L, luaSysChipId);
    lua_setfield(L, -2, "chipId");

    lua_pushcfunction(L, luaSysVersion);
    lua_setfield(L, -2, "version");

    lua_pushcfunction(L, luaSysFlashSize);
    lua_setfield(L, -2, "flashSize");

    lua_pushcfunction(L, luaSysSdkVersion);
    lua_setfield(L, -2, "sdkVersion");

    lua_setglobal(L, "sys");
}
