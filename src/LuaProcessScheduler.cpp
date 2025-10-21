#include "LuaProcessScheduler.hpp"
#include "Arduino.h"
#include "LuaNekosLibs.hpp"
// ================================
// Static members
// ================================
TaskHandle_t LuaProcessScheduler::taskHandles[MAX_LUA_PROCESSES] = { nullptr };
SemaphoreHandle_t LuaProcessScheduler::pidMutex;
// ================================
// Initialization / Shutdown
// ================================
void LuaProcessScheduler::begin() {
    for (int i = 0; i < MAX_LUA_PROCESSES; i++) {
        taskHandles[i] = nullptr;
    }
    LuaProcessScheduler::pidMutex = xSemaphoreCreateMutex();
}

void LuaProcessScheduler::end() {
    for (int i = 0; i < MAX_LUA_PROCESSES; i++) {
        if (taskHandles[i]) {
            vTaskDelete(taskHandles[i]);
            taskHandles[i] = nullptr;
        }
    }
}

// ================================
// PID management
// ================================
int LuaProcessScheduler::allocatePid() {
    int pid = -1;
    if (pidMutex) xSemaphoreTake(pidMutex, portMAX_DELAY);
    for (int i = 0; i < MAX_LUA_PROCESSES; i++) {
        if (taskHandles[i] == nullptr) {
            pid = i;
            taskHandles[i] = (TaskHandle_t)0x1; // temporary mark as taken
            break;
        }
    }
    if (pidMutex) xSemaphoreGive(pidMutex);
    return pid;
}


void LuaProcessScheduler::freePid(uint32_t pid) {
    if (pid >= MAX_LUA_PROCESSES) return;
    if (pidMutex) xSemaphoreTake(pidMutex, portMAX_DELAY);
    taskHandles[pid] = nullptr;
    if (pidMutex) xSemaphoreGive(pidMutex);
}

// ================================
// Round-robin core selection
// ================================
uint8_t LuaProcessScheduler::getNextCoreIndex() {
    static uint8_t lastCore = 0;
    uint8_t core = lastCore;
    lastCore = (lastCore + 1) % portNUM_PROCESSORS;
    return core;
}

// ================================
// Job submission
// ================================
int LuaProcessScheduler::run(FILE* luaFile) {
    if (!luaFile) return -1;
    int pid = allocatePid();
    if (pid < 0) return -1;
    LuaFileProcessParams* p = new LuaFileProcessParams{ (uint32_t)pid, luaFile };
    char taskName[16];
    generateTaskName(taskName, sizeof(taskName), pid);
    BaseType_t res = xTaskCreatePinnedToCore(
        fileCodeExecutor,
        taskName,
        4096,
        p,
        1,
        &taskHandles[pid],
        getNextCoreIndex()
    );
    if (res != pdPASS) {
        delete p;
        freePid(pid);
        return -1;
    }
    return pid;
}

int LuaProcessScheduler::runByteCode(FILE* byteCodeFile) {
    if (!byteCodeFile) return -1;
    int pid = allocatePid();
    if (pid < 0) return -1;
    LuaByteCodeProcessParams* p = new LuaByteCodeProcessParams{ (uint32_t)pid, byteCodeFile };
    char taskName[16];
    generateTaskName(taskName, sizeof(taskName), pid);
    BaseType_t res = xTaskCreatePinnedToCore(
        byteCodeFileExecutor,
        taskName,
        4096,
        p,
        1,
        &taskHandles[pid],
        getNextCoreIndex()
    );
    if (res != pdPASS) {
        delete p;
        freePid(pid);
        return -1;
    }
    return pid;
}

int LuaProcessScheduler::run(const char* code) {
    if (!code) return -1;
    int pid = allocatePid();
    if (pid < 0) return -1;
    LuaCharProcessParams* p = new LuaCharProcessParams{ (uint32_t)pid, code };
    char taskName[16];
    generateTaskName(taskName, sizeof(taskName), pid);
    BaseType_t res = xTaskCreatePinnedToCore(
        charCodeExecutor,
        taskName,
        4096,
        p,
        1,
        &taskHandles[pid],
        getNextCoreIndex()
    );
    if (res != pdPASS) {
        delete p;
        freePid(pid);
        return -1;
    }
    return pid;
}

// ================================
// Executors
// ================================
void LuaProcessScheduler::fileCodeExecutor(void* param) {
    auto* p = reinterpret_cast<LuaFileProcessParams*>(param);
    if (!p) {
        vTaskDelete(nullptr);
        return;
    }
    fseek(p->source, 0, SEEK_END);
    long size = ftell(p->source);
    fseek(p->source, 0, SEEK_SET);
    char* buffer = new char[size + 1];
    fread(buffer, 1, size, p->source);
    buffer[size] = '\0';
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    LuaOpenNekosLibs(L);
    pushPidToLuaState(L, p->pid);
    int result = luaL_loadstring(L, buffer);
    if (result == LUA_OK) result = lua_pcall(L, 0, LUA_MULTRET, 0);
    if (result != LUA_OK) {
        Serial.printf("Lua file error (PID=%u): %s\n", p->pid, lua_tostring(L, -1));
        lua_pop(L, 1);
    } else {
        Serial.printf("Lua file executed successfully (PID=%u)\n", p->pid);
    }
    lua_close(L);
    delete[] buffer;
    freePid(p->pid);
    delete p;
    vTaskDelete(nullptr);
}

void LuaProcessScheduler::byteCodeFileExecutor(void* param) {
    auto* p = reinterpret_cast<LuaByteCodeProcessParams*>(param);
    if (!p) {
        vTaskDelete(nullptr);
        return;
    }
    fseek(p->source, 0, SEEK_END);
    long size = ftell(p->source);
    fseek(p->source, 0, SEEK_SET);
    char* buffer = new char[size];
    fread(buffer, 1, size, p->source);
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    LuaOpenNekosLibs(L);
    pushPidToLuaState(L, p->pid);
    int result = luaL_loadbuffer(L, buffer, size, "bytecode_chunk");
    if (result == LUA_OK) result = lua_pcall(L, 0, LUA_MULTRET, 0);
    if (result != LUA_OK) {
        Serial.printf("Lua bytecode error (PID=%lu): %s\n", p->pid, lua_tostring(L, -1));
        lua_pop(L, 1);
    } else {
        Serial.printf("Lua bytecode executed successfully (PID=%lu)\n", p->pid);
    }
    lua_close(L);
    delete[] buffer;
    freePid(p->pid);
    delete p;
    vTaskDelete(nullptr);
}

void LuaProcessScheduler::charCodeExecutor(void* param) {
    auto* p = reinterpret_cast<LuaCharProcessParams*>(param);
    if (!p) {
        vTaskDelete(nullptr);
        return;
    }
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    LuaOpenNekosLibs(L);
    pushPidToLuaState(L, p->pid);
    int result = luaL_loadstring(L, p->code);
    if (result == LUA_OK) result = lua_pcall(L, 0, LUA_MULTRET, 0);
    if (result != LUA_OK) {
        Serial.printf("Lua code error (PID=%u): %s\n", p->pid, lua_tostring(L, -1));
        lua_pop(L, 1);
    } else {
        Serial.printf("Lua code executed successfully (PID=%u)\n", p->pid);
    }
    lua_close(L);
    freePid(p->pid);
    delete p;
    vTaskDelete(nullptr);
}

void LuaProcessScheduler::pushPidToLuaState(lua_State* L, uint32_t pid) {
    if (!L) return;
    lua_pushinteger(L, pid);
    lua_setglobal(L, "_PID");
}