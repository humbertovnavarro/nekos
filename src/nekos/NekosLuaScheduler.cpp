// LuaScheduler.cpp
#include "NekosLuaScheduler.h"
#include "Arduino.h"

extern "C" {
    #include "lua.h"
    #include "lualib.h"
    #include "lauxlib.h"
}

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "LuaScripts.h"

namespace nekos {

// Forward declaration to your existing runner:
extern void luaExec(const char* script, const char* args);

// ----------- static storage -----------
size_t LuaScheduler::_capacity = 0;
std::list<LuaCacheEntry> LuaScheduler::_cacheList;

// worker infra
static constexpr size_t JOB_QUEUE_LEN = 32;
static constexpr uint32_t WORKER_STACK_WORDS = 8192; // generous stack for Lua calls
static constexpr UBaseType_t WORKER_PRIO = 1;

static QueueHandle_t s_jobQueue = nullptr;
static SemaphoreHandle_t s_cacheMutex = nullptr;

#ifndef MAX_CORES
#define MAX_CORES 4
#endif
static TaskHandle_t s_workers[MAX_CORES] = {nullptr};

// ------------ job representation ------------
struct LuaJob {
    String name;
    String args;
};

// ---------- helpers: cache lock ----------
class CacheLock {
public:
    CacheLock(SemaphoreHandle_t m) : m_(m) { if (m_) xSemaphoreTake(m_, portMAX_DELAY); }
    ~CacheLock() { if (m_) xSemaphoreGive(m_); }
private:
    SemaphoreHandle_t m_;
};

// ---------- worker task ----------
static void luaWorkerTask(void* pv) {
    const int core = xPortGetCoreID();
    Serial.printf("[LuaWorker] started on core %d\n", core);
    for (;;) {
        LuaJob* job = nullptr;
        if (xQueueReceive(s_jobQueue, &job, portMAX_DELAY) == pdTRUE && job) {
            // Fetch script text from cache (thread-safe)
            char* script = LuaScheduler::getScript(job->name, /*moveToFront*/true);
            if (!script) {
                Serial.printf("[LuaWorker] script not found: %s\n", job->name.c_str());
                delete job;
                continue;
            }

            // Execute Lua
            const char* args = job->args.length() ? job->args.c_str() : nullptr;
            luaExec(script, args);
            delete job;
        }
    }
}

// ---------- public API ----------
void LuaScheduler::begin(size_t cacheCapacity) {
    initLuaScriptMap();
    _capacity = cacheCapacity;

    if (!s_cacheMutex) {
        s_cacheMutex = xSemaphoreCreateMutex();
    }
    if (!s_jobQueue) {
        s_jobQueue = xQueueCreate(JOB_QUEUE_LEN, sizeof(LuaJob*));
    }

    // Create one worker per core
    int cores = 1;
    #ifdef portNUM_PROCESSORS
        cores = portNUM_PROCESSORS;
    #endif
    if (cores > MAX_CORES) cores = MAX_CORES;

    for (int i = 0; i < cores; ++i) {
        if (s_workers[i]) continue; // already created
        BaseType_t ok = xTaskCreatePinnedToCore(
            luaWorkerTask,
            (String("LuaWorker-") + i).c_str(),
            WORKER_STACK_WORDS,
            nullptr,
            WORKER_PRIO,
            &s_workers[i],
            i  // pin to this core
        );
        if (ok != pdPASS) {
            Serial.printf("[LuaScheduler] Failed to start worker on core %d\n", i);
        }
    }

    Serial.printf("[LuaScheduler] init: cache=%u entries, workers=%d, queue=%u\n",
                  (unsigned)_capacity, cores, (unsigned)JOB_QUEUE_LEN);
}

void LuaScheduler::exec(const String &name, const char* args) {
    if (!s_jobQueue) {
        Serial.println("[LuaScheduler] exec() before begin(); auto-initializing.");
        begin(_capacity ? _capacity : 8);
    }

    LuaJob* job = new LuaJob{ name, String(args ? args : "") };

    // Try to queue quickly; if full, drop oldest to make room (non-blocking producer)
    if (xQueueSend(s_jobQueue, &job, 0) != pdTRUE) {
        // Drop one
        LuaJob* old = nullptr;
        if (xQueueReceive(s_jobQueue, &old, 0) == pdTRUE && old) {
            delete old;
        }
        // Try again
        if (xQueueSend(s_jobQueue, &job, 0) != pdTRUE) {
            Serial.printf("[LuaScheduler] job queue full; dropping '%s'\n", name.c_str());
            delete job;
        }
    }
}

bool LuaScheduler::exists(const String &name) {
    CacheLock lock(s_cacheMutex);
    for (auto it = _cacheList.begin(); it != _cacheList.end(); ++it) {
        if (it->name == name) {
            return true;
        }
    }
    auto it = luaScriptMap->find(name);
    if (it != luaScriptMap->end() && it->second) {
        return true;
    }

    return false;
}

// Returns pointer to script text (owned by cache). Optionally moves to front (LRU).
char* LuaScheduler::getScript(const String &name, bool moveToFront) {
    CacheLock lock(s_cacheMutex);
    for (auto it = _cacheList.begin(); it != _cacheList.end(); ++it) {
        if (it->name == name) {
            if (moveToFront && it != _cacheList.begin()) {
                _cacheList.splice(_cacheList.begin(), _cacheList, it);
                return _cacheList.begin()->data;
            }
            return it->data;
        }
    }
}


} // namespace nekos
