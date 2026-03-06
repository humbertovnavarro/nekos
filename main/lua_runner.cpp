#include <Arduino.h>
#include <cstdio>
#include <cstring>
#include "lua_runner.hpp"
#include "stdin.hpp"
#include "sys/dirent.h"

extern "C" {
    #include "lua.h"
}

#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include "esp_spiffs.h"
#include "lua_state_pool.h"

#ifndef LUA_SPIFFS_BASE
  #define LUA_SPIFFS_BASE       "/lua"
#endif
#ifndef LUA_SPIFFS_PARTITION
  #define LUA_SPIFFS_PARTITION  "lua"
#endif
#ifndef LUA_CHUNK_SIZE
  #define LUA_CHUNK_SIZE        512
#endif
#ifndef LUA_RUNNER_COUNT
  #define LUA_RUNNER_COUNT      3
#endif
#ifndef LUA_QUEUE_DEPTH
  #define LUA_QUEUE_DEPTH       16
#endif
#ifndef LUA_RUNNER_STACK
  #define LUA_RUNNER_STACK      24576
#endif
#ifndef LUA_RUNNER_PRIORITY
  #define LUA_RUNNER_PRIORITY   2
#endif

const char* TAG_LUA = "lua";

esp_err_t lua_fs_mount(bool format_on_fail)
{
    esp_vfs_spiffs_conf_t cfg = {
        .base_path              = LUA_SPIFFS_BASE,
        .partition_label        = LUA_SPIFFS_PARTITION,
        .max_files              = 8,
        .format_if_mount_failed = format_on_fail,
    };
    esp_err_t err = esp_vfs_spiffs_register(&cfg);
    if (err != ESP_OK)
        log_e("%s: mount failed: %s", TAG_LUA, esp_err_to_name(err));
    return err;
}

void lua_fs_unmount()
{
    esp_vfs_spiffs_unregister(LUA_SPIFFS_PARTITION);
}

void lua_full_path(const char *name, char *out, size_t sz)
{
    if (strncmp(name, LUA_SPIFFS_BASE, strlen(LUA_SPIFFS_BASE)) == 0)
        strncpy(out, name, sz - 1);
    else
        snprintf(out, sz, "%s/%s", LUA_SPIFFS_BASE, name);
    out[sz - 1] = '\0';
}

const char *lua_file_reader_cb(lua_State *L, void *ud, size_t *sz)
{
    (void)L;
    LuaFileReader *r = static_cast<LuaFileReader *>(ud);
    *sz = fread(r->buf, 1, sizeof(r->buf), r->fp);
    return (*sz > 0) ? r->buf : nullptr;
}

void lua_log_error(lua_State *L)
{
    const char *msg = lua_tostring(L, -1);
    log_e("%s: %s", TAG_LUA, msg ? msg : "(non-string error)");
    lua_pop(L, 1);
}

void lua_push_args(lua_State *L, const char *path, const Arguments &blk)
{
    lua_newtable(L);
    lua_pushstring(L, path);
    lua_rawseti(L, -2, 0);
    for (int i = 0; i < blk.argc(); ++i) {
        lua_pushstring(L, blk.arg(i));
        lua_rawseti(L, -2, i + 1);
    }
    lua_setglobal(L, "arg");
}

bool lua_run_file(lua_State *L, const char *path, const Arguments &args)
{
    char full[256];
    lua_full_path(path, full, sizeof(full));
    LuaFileReader r{};
    r.fp = fopen(full, "r");
    if (!r.fp) {
        log_e("%s: cannot open %s", TAG_LUA, full);
        return false;
    }
    int err = lua_load(L, lua_file_reader_cb, &r, full, "t");
    fclose(r.fp);
    if (err != LUA_OK) { lua_log_error(L); return false; }
    lua_push_args(L, full, args);
    err = lua_pcall(L, 0, LUA_MULTRET, 0);
    if (err != LUA_OK) { lua_log_error(L); return false; }
    return true;
}

void lua_list_scripts(void (*cb)(const char *path))
{
    DIR *dir = opendir(LUA_SPIFFS_BASE);
    if (!dir) { log_e("%s: cannot open %s", TAG_LUA, LUA_SPIFFS_BASE); return; }
    struct dirent *ent;
    while ((ent = readdir(dir)) != nullptr) {
        const char *name = ent->d_name;
        size_t      len  = strlen(name);
        if (len > 4 && strcmp(name + len - 4, ".lua") == 0) {
            char full[261];
            snprintf(full, sizeof(full), "%s/%s", LUA_SPIFFS_BASE, name);
            cb(full);
        }
    }
    closedir(dir);
}

static QueueHandle_t s_queue                   = nullptr;
static TaskHandle_t  s_tasks[LUA_RUNNER_COUNT] = {};
static bool          s_running                 = false;

void runner_task(void * /*arg*/)
{
    for (;;) {
        LuaJob *job = nullptr;
        if (xQueueReceive(s_queue, &job, portMAX_DELAY) != pdTRUE || !job)
            continue;

        LuaStateSlot *lslot = lua_pool_acquire();
        lua_push_args(lslot->L, job->path, *job->args);
        
        if (lslot) {
            job->ok = lua_run_file(lslot->L, job->path, *job->args);
            lua_pool_release(lslot);
        } else {
            log_e("%s: no free Lua state for '%s'", TAG_LUA, job->path);
            job->ok = false;
        }
        StdinArgPool::free(job->args);
        job->args = nullptr;

        if (job->done_sem)
            xSemaphoreGive(job->done_sem);
        else
            delete job;
    }
}

bool lua_init()
{
    if (s_running) return true;

    s_queue = xQueueCreate(LUA_QUEUE_DEPTH, sizeof(LuaJob *));
    if (!s_queue) { log_e("%s: queue create failed", TAG_LUA); return false; }

    for (int i = 0; i < LUA_RUNNER_COUNT; ++i) {
        char name[16];
        snprintf(name, sizeof(name), "lua_run%d", i);
        BaseType_t r = xTaskCreate(runner_task, name, LUA_RUNNER_STACK,
                                   nullptr, LUA_RUNNER_PRIORITY, &s_tasks[i]);
        if (r != pdPASS) {
            log_e("%s: failed to create runner task %d", TAG_LUA, i);
            return false;
        }
    }
    s_running = true;
    log_i("%s: %d runner(s) started, queue depth %d",
          TAG_LUA, LUA_RUNNER_COUNT, LUA_QUEUE_DEPTH);
    return true;
}

LuaJob *lua_submit(const char *path, Arguments *args)
{
    auto *job = new LuaJob{};
    lua_full_path(path, job->path, sizeof(job->path));
    job->args = args;

    if (xQueueSend(s_queue, &job, portMAX_DELAY) != pdTRUE) {
        log_e("%s: queue send failed for '%s'", TAG_LUA, job->path);
        delete job;
        return nullptr;   // caller frees blk
    }
    return job;
}

bool lua_wait(LuaJob *job)
{
    if (!job || !job->done_sem) return false;
    xSemaphoreTake(job->done_sem, portMAX_DELAY);
    bool ok = job->ok;
    delete job;
    return ok;
}

UBaseType_t lua_queue_waiting()
{
    return s_queue
        ? uxQueueMessagesWaiting(s_queue)
        : 0;
}