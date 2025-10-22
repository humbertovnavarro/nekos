#include "FreeRTOS.h"
#include "freertos/task.h"
#include "../LuaModule.hpp"

inline LuaModule freertosModule("freertos", [](lua_State* L) {
        LuaModule::begin(L);

        LuaModule::addFunction(L, "delay", [](lua_State* L) -> int {
            int ms = luaL_checkinteger(L, 1);
            vTaskDelay(pdMS_TO_TICKS(ms));
            return 0;
        });

        LuaModule::addFunction(L, "delayTicks", [](lua_State* L) -> int {
            int ticks = luaL_checkinteger(L, 1);
            vTaskDelay(ticks);
            return 0;
        });
});


inline void registerFreeRTOSModule() {
    registerLuaModule(freertosModule);
}