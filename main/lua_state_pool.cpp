#include "freertos/FreeRTOS.h"

#include "lua_state_pool.h"
#include "lua_utils.h"
#include "lua_oled.hpp"

#include "etl/pool.h"
#include "etl/queue.h"

#include "esp32-hal-log.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

static etl::pool <LuaStateSlot, LUA_POOL_SIZE> s_pool;
static etl::queue<LuaStateSlot*, LUA_POOL_SIZE> s_free;
static SemaphoreHandle_t s_mutex = nullptr;
static bool              s_ready = false;

static const char *TAG = "lua_pool";

void lua_pool_init()
{
    if (s_ready) return;
    s_mutex = xSemaphoreCreateMutex();
    if (!s_mutex) { log_e("%s: mutex create failed", TAG); return; }
    for (int i = 0; i < LUA_POOL_SIZE; ++i) {
        lua_State *L = luaL_newstate();
        if (!L) { log_e("%s: luaL_newstate failed at slot %d", TAG, i); continue; }
        luaL_openlibs(L);
        lua_utils::open(L);
        lua_oled::open(L);
        LuaStateSlot *slot = s_pool.allocate();
        *slot = { i, L, false };
        s_free.push(slot);
    }
    s_ready = true;
    log_i("%s: %d slot(s) ready", TAG, (int)s_free.size());
}

LuaStateSlot *lua_pool_acquire()
{
    for (;;) {
        xSemaphoreTake(s_mutex, portMAX_DELAY);
        if (!s_free.empty()) {
            LuaStateSlot *slot = s_free.front();
            s_free.pop();
            slot->busy = true;
            xSemaphoreGive(s_mutex);
            return slot;
        }
        xSemaphoreGive(s_mutex);
        vTaskDelay(1);
    }
}

void lua_pool_release(LuaStateSlot *slot)
{
    if (!slot) return;
    lua_gc(slot->L, LUA_GCCOLLECT, 0);
    slot->busy = false;
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    s_free.push(slot);
    xSemaphoreGive(s_mutex);
}