#pragma once

#include <Arduino.h>
#include <cstdio>
#include <cstring>
#include "stdin.hpp"

extern "C" {
    #include "lua.h"
}

#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

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

struct LuaFileReader {
    FILE *fp             = nullptr;
    char  buf[LUA_CHUNK_SIZE] = {};
};

struct LuaJob {
    char      path[64] = {};
    Arguments *args      = nullptr;
    bool              ok       = false;
    SemaphoreHandle_t done_sem = nullptr;
};

esp_err_t lua_fs_mount(bool format_on_fail = false);
void lua_fs_unmount();
void lua_full_path(const char *name, char *out, size_t sz);
const char *lua_file_reader_cb(lua_State *L, void *ud, size_t *sz);
void lua_log_error(lua_State *L);
void lua_push_args(lua_State *L, const char *path, const Arguments &blk);
bool lua_run_file(lua_State *L, const char *path, const Arguments &blk);
void lua_list_scripts(void (*cb)(const char *path));
void runner_task(void *);
bool lua_init();
LuaJob *lua_submit(const char *path, Arguments *blk);
bool lua_wait(LuaJob *job);
UBaseType_t lua_queue_waiting();
bool lua_run_file_sync(const char        *path,
                              int                argc = 0,
                              const char *const *argv = nullptr);