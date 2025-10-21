#pragma once
#include "Arduino.h"
#include "lua.hpp"
#include "NekosLuaConfig.hpp"

struct LuaFileProcessParams {
    uint32_t pid;
    FILE* source;  
};

struct LuaByteCodeProcessParams {
    uint32_t pid;
    FILE* source;
};

struct LuaCharProcessParams {
    uint32_t pid;
    const char* code;
};

class LuaProcessScheduler {
public:
    static void begin();
    static void end();
    static int run(FILE* luaFile);
    static int run(const char*);
    static int runByteCode(FILE* byteCodeFile);
private:
    static void byteCodeFileExecutor(void* pid);
    static void charCodeExecutor(void* pid);
    static void fileCodeExecutor(void* pid);
    static TaskHandle_t taskHandles[MAX_LUA_PROCESSES];
    static int allocatePid();
    static void freePid(uint32_t pid);
    static void generateTaskName(char* buffer, size_t size, uint32_t pid) {
        snprintf(buffer, size, "LuaTask-%lu", (unsigned long)pid);
    }
    static SemaphoreHandle_t pidMutex;
    static uint8_t getNextCoreIndex();
    static void pushPidToLuaState(lua_State* L, uint32_t pid);
};
