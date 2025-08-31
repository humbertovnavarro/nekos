#include "NekosFreeRTOS.h"
#include <Arduino.h>
#include "FreeRTOS.h"
#include "task.h"

extern "C" {
    #include "lua.h"
    #include "lualib.h"
    #include "lauxlib.h"
}

namespace nekos {

// ----------------- basic bindings -----------------

int luaDelay(lua_State* L) {
    int ms = luaL_checkinteger(L, 1);
    vTaskDelay(pdMS_TO_TICKS(ms));
    return 0;
}

int luaYield(lua_State* L) {
    taskYIELD();
    return 0;
}

int luaMillis(lua_State* L) {
    lua_pushinteger(L, millis());
    return 1;
}

// ----------------- task trampoline -----------------

struct LuaTaskData {
    lua_State* L;
    int funcRef;
};

static void luaTaskTrampoline(void* arg) {
    LuaTaskData* data = static_cast<LuaTaskData*>(arg);
    lua_State* L = data->L;

    lua_rawgeti(L, LUA_REGISTRYINDEX, data->funcRef);

    if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
        const char* err = lua_tostring(L, -1);
        Serial.printf("[LuaTask] Error: %s\n", err);
        lua_pop(L, 1);
    }

    luaL_unref(L, LUA_REGISTRYINDEX, data->funcRef);
    delete data;

    vTaskDelete(nullptr);
}

// ----------------- task functions -----------------

// createTask(name, func, stack, priority)
int luaCreateTask(lua_State* L) {
    const char* name     = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TFUNCTION);
    int stackWords       = luaL_optinteger(L, 3, 4096);
    int priority         = luaL_optinteger(L, 4, 1);

    int funcRef = luaL_ref(L, LUA_REGISTRYINDEX);
    LuaTaskData* data = new LuaTaskData{ L, funcRef };

    TaskHandle_t handle = nullptr;
    BaseType_t res = xTaskCreate(
        luaTaskTrampoline,
        name,
        stackWords,
        data,
        priority,
        &handle
    );

    if (res != pdPASS) {
        luaL_unref(L, LUA_REGISTRYINDEX, funcRef);
        delete data;
        return luaL_error(L, "Failed to create task");
    }

    lua_pushlightuserdata(L, handle); // return handle
    return 1;
}

// deleteTask(handle or nil/self)
int luaDeleteTask(lua_State* L) {
    TaskHandle_t handle = nullptr;
    if (!lua_isnoneornil(L, 1)) {
        handle = (TaskHandle_t)lua_touserdata(L, 1);
    }
    vTaskDelete(handle);
    return 0;
}

// getTaskName(handle or nil/self)
int luaGetTaskName(lua_State* L) {
    TaskHandle_t handle = nullptr;
    if (!lua_isnoneornil(L, 1)) {
        handle = (TaskHandle_t)lua_touserdata(L, 1);
    }
    const char* name = pcTaskGetName(handle);
    if (name) {
        lua_pushstring(L, name);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

// getTaskHandle() -> self handle
int luaGetTaskHandle(lua_State* L) {
    TaskHandle_t handle = xTaskGetCurrentTaskHandle();
    lua_pushlightuserdata(L, handle);
    return 1;
}

// ----------------- register -----------------

void registerFreeRTOSBindings(lua_State* L) {
    lua_newtable(L);

    lua_pushcfunction(L, luaDelay);         lua_setfield(L, -2, "delay");
    lua_pushcfunction(L, luaYield);         lua_setfield(L, -2, "yield");
    lua_pushcfunction(L, luaMillis);        lua_setfield(L, -2, "millis");
    lua_pushcfunction(L, luaCreateTask);    lua_setfield(L, -2, "createTask");
    lua_pushcfunction(L, luaDeleteTask);    lua_setfield(L, -2, "deleteTask");
    lua_pushcfunction(L, luaGetTaskName);   lua_setfield(L, -2, "getTaskName");
    lua_pushcfunction(L, luaGetTaskHandle); lua_setfield(L, -2, "getTaskHandle");

    // Set FreeRTOS global table
    lua_setglobal(L, "FreeRTOS");

    // -----------------------------------------
    // Aliases: map globals to FreeRTOS.*
    // -----------------------------------------
    luaL_dostring(L,
        "delay        = FreeRTOS.delay\n"
        "yield        = FreeRTOS.yield\n"
        "millis       = FreeRTOS.millis\n"
        "createTask   = FreeRTOS.createTask\n"
        "deleteTask   = FreeRTOS.deleteTask\n"
        "getTaskName  = FreeRTOS.getTaskName\n"
        "getTaskHandle= FreeRTOS.getTaskHandle\n"
    );
}



} // namespace nekos
