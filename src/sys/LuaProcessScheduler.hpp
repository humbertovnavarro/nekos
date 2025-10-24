#pragma once
#include "Arduino.h"
#include "lua.hpp"
#define MAX_PATH_SIZE 64
#define MAX_LUA_PROCESSES 16
#define LUA_STREAM_BUFFER_CHUNK_SIZE 1024
#define LUA_DEFAULT_STACK_SIZE 4096
#define LUA_DEFAULT_PRIORITY 1
#define NO_AFFINITY -1

struct LuaProcess {
    bool free = true;
    lua_State* L;
    char filePath[64];
    size_t filePathSize;
    uint32_t pid;
    TaskHandle_t taskHandle;
    char streamBuffer[LUA_STREAM_BUFFER_CHUNK_SIZE];
};

struct LuaProcessStartOptions {
    uint32_t stackSize;
    uint32_t priority;
    int32_t affinity = -1;
};

class LuaProcessScheduler {
public:
    static LuaProcess* luaProcesses;
    static SemaphoreHandle_t schedulerMutex;
    static void begin();
    static void end();
    static int run(const char* basePath, LuaProcessStartOptions options = {LUA_DEFAULT_STACK_SIZE, LUA_DEFAULT_PRIORITY, NO_AFFINITY});
    static void runFileTask(void* pid);
    static int allocatePid();
    static void freePid(uint32_t pid);
    static void generateTaskName(char* buffer, size_t size, uint32_t pid) {
        snprintf(buffer, size, "%lu", (unsigned long)pid);
    }
    static uint8_t getNextCoreIndex();
    static void pushPidToLuaState(lua_State* L, uint32_t pid);
};
