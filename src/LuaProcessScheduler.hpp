#pragma once
#include "Arduino.h"
#include "lua.hpp"
#include "NekosLuaConfig.hpp"
#include "FFat.h"

struct LuaFileProcessParams {
    uint32_t pid;
    fs::File* source;  
};

struct LuaByteCodeProcessParams {
    uint32_t pid;
    fs::File* source;
};

struct LuaCharProcessParams {
    uint32_t pid;
    const char* code;
};

class LuaProcessScheduler {
public:
    static void begin();
    static void end();
    static int run(const char* code);
    static int runFile(const char* filePath);
    static int runByteCode(fs::File* byteCodeFile);
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
