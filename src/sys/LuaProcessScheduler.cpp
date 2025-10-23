#include "Arduino.h"
#include "lua.hpp"
#include "FFat.h"
#include "LuaProcessScheduler.hpp"
#include "LuaModule.hpp"
#include "modules/os/LuaFreeRTOSModule.hpp"
#include "modules/peripherals/LuaSerialModule.hpp"
#include "modules/os/LuaFileSystemModule.hpp"
#include "modules/peripherals/LuaGPIOModule.hpp"
#include "modules/peripherals/LuaNeoPixelModule.hpp"

// ============================================================
// Static Members
// ============================================================
LuaProcess* LuaProcessScheduler::luaProcesses = nullptr;
SemaphoreHandle_t LuaProcessScheduler::schedulerMutex = nullptr;

static bool processUsed[MAX_LUA_PROCESSES] = { false };

void LuaProcessScheduler::begin() {
    registerLuaModule(luafreeRTOSModule);
    registerLuaModule(luaFileSystemModule);
    registerLuaModule(luaGpioModule);
    registerLuaModule(luaNeopixelModule);
    registerLuaModule(luaSerialModule);

    luaProcesses = (LuaProcess*) malloc(sizeof(LuaProcess) * MAX_LUA_PROCESSES);
    memset(luaProcesses, 0, sizeof(LuaProcess) * MAX_LUA_PROCESSES);
    memset(processUsed, 0, sizeof(processUsed));
    Serial.println("[LuaScheduler] Initialized");
}

void LuaProcessScheduler::end() {
    for (int i = 0; i < MAX_LUA_PROCESSES; ++i) {
        if (processUsed[i] && luaProcesses[i].taskHandle) {
            vTaskDelete(luaProcesses[i].taskHandle);
            processUsed[i] = false;
        }
    }
    Serial.println("[LuaScheduler] Shutdown complete");
}


int LuaProcessScheduler::allocatePid() {
    static uint32_t nextPid = 1;
    if (!schedulerMutex) schedulerMutex = xSemaphoreCreateMutex();
    xSemaphoreTake(schedulerMutex, portMAX_DELAY);
    uint32_t pid = nextPid++;
    xSemaphoreGive(schedulerMutex);
    return pid;
}

void LuaProcessScheduler::freePid(uint32_t pid) {
    (void)pid; // No-op
}


uint8_t LuaProcessScheduler::getNextCoreIndex() {
#ifdef ARDUINO_ARCH_ESP32
    static uint8_t next = 0;
    next ^= 1; // toggle
    return next;
#else
    return 0;
#endif
}


static LuaProcess* allocateProcessSlot() {
    taskENTER_CRITICAL(nullptr);
    for (int i = 0; i < MAX_LUA_PROCESSES; ++i) {
        if (!processUsed[i]) {
            processUsed[i] = true;
            LuaProcess* proc = &LuaProcessScheduler::luaProcesses[i];
            memset(proc, 0, sizeof(LuaProcess));
            taskEXIT_CRITICAL(nullptr);
            return proc;
        }
    }
    taskEXIT_CRITICAL(nullptr);
    return nullptr;
}

static void freeProcessSlot(LuaProcess* proc) {
    taskENTER_CRITICAL(nullptr);
    for (int i = 0; i < MAX_LUA_PROCESSES; ++i) {
        if (&LuaProcessScheduler::luaProcesses[i] == proc) {
            processUsed[i] = false;
            break;
        }
    }
    taskEXIT_CRITICAL(nullptr);
}

void LuaProcessScheduler::byteCodeFileExecutor(void* arg) {
    LuaProcess* proc = static_cast<LuaProcess*>(arg);
    if (!proc || !proc->luaCFilePath) {
        Serial.println("[LuaExec] Invalid process or file");
        vTaskDelete(nullptr);
        return;
    }
    File luac = FFat.open(proc->luaCFilePath, "r");
    if (!luac) {
        Serial.printf("[LuaExec %lu] Failed to open %s\n", proc->pid, proc->luaCFilePath);
        freeProcessSlot(proc);
        vTaskDelete(nullptr);
        return;
    }
    lua_State* L = luaL_newstate();
    if (!L) {
        Serial.printf("[LuaExec %lu] Failed to create Lua state\n", proc->pid);
        luac.close();
        freeProcessSlot(proc);
        vTaskDelete(nullptr);
        return;
    }
    luaL_openlibs(L);
    // Expose module system to lua runtime.
    exposeRequire(L);
    lua_pushinteger(L, (lua_Integer)proc->pid);
    lua_setglobal(L, "__PID");

    struct ReaderCtx {
        File* file;
        uint8_t buffer[512];
    } ctx = { &luac };

    auto reader = [](lua_State*, void* data, size_t* size) -> const char* {
        ReaderCtx* ctx = static_cast<ReaderCtx*>(data);
        size_t n = ctx->file->read(ctx->buffer, sizeof(ctx->buffer));
        if (n == 0) {
            *size = 0;
            return nullptr;
        }
        *size = n;
        return (const char*)ctx->buffer;
    };

    char chunkName[32];
    snprintf(chunkName, sizeof(chunkName), "pid_%lu", (unsigned long)proc->pid);

    int loadStatus = lua_load(L, reader, &ctx, chunkName, "b");
    if (loadStatus != LUA_OK) {
        Serial.printf("[LuaExec %lu] Load error: %s\n", proc->pid,
                      lua_tostring(L, -1));
        lua_close(L);
        luac.close();
        freeProcessSlot(proc);
        vTaskDelete(nullptr);
        return;
    }

    int callStatus = lua_pcall(L, 0, LUA_MULTRET, 0);
    if (callStatus != LUA_OK) {
        Serial.printf("[LuaExec %lu] Runtime error: %s\n", proc->pid,
                      lua_tostring(L, -1));
        lua_pop(L, 1);
    } else {
        Serial.printf("[LuaExec %lu] Execution finished successfully\n", proc->pid);
    }

    lua_close(L);
    luac.close();
    freeProcessSlot(proc);
    vTaskDelete(nullptr);
}


int LuaProcessScheduler::run(const char* luaCFilePath, LuaProcessStartOptions opts) {
    LuaProcess* proc = allocateProcessSlot();
    if (!proc) {
        Serial.println("[LuaScheduler] No available process slots");
        return -1;
    }

    proc->pid = allocatePid();
    proc->luaCFilePath = luaCFilePath;
    proc->taskHandle = nullptr;

    char taskName[16];
    generateTaskName(taskName, sizeof(taskName), proc->pid);

    BaseType_t res = xTaskCreatePinnedToCore(
        LuaProcessScheduler::byteCodeFileExecutor,
        taskName,
        opts.stackSize,
        proc,
        opts.priority,
        &proc->taskHandle,
        opts.affinity == -1 ? getNextCoreIndex() : opts.affinity
    );

    if (res != pdPASS) {
        Serial.printf("[LuaScheduler] Failed to start Lua task (pid=%lu)\n", (unsigned long)proc->pid);
        freeProcessSlot(proc);
        freePid(proc->pid);
        return -1;
    }

    Serial.printf("[LuaScheduler] Started Lua task (pid=%lu) for %s\n",
                  (unsigned long)proc->pid, luaCFilePath);
    return proc->pid;
}