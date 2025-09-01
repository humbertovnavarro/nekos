#pragma once
#include <Arduino.h>
#include <map>
#include <list>

extern "C" {
    #include "lua.h"
    #include "lualib.h"
    #include "lauxlib.h"
}

namespace nekos {

// -------------------------
// Cached Lua script entry
// -------------------------
struct LuaCacheEntry {
    String name;
    char* data;
    size_t size;
};

// -------------------------
// Lua Loader with workers
// -------------------------
class LuaScheduler {
public:
    // Initialize cache + worker tasks (spawns one worker per core)
    static void begin(size_t cacheCapacity = 8);

    // Schedule a Lua script for execution on any worker
    static void exec(const String &name, const char* args = nullptr);

    // Check if script exists in cache
    static bool exists(const String &name);

    // Print all cached scripts to Serial
    static void listScripts();

    // Get raw script text from cache
    //   moveToFront=true keeps LRU ordering
    static char* getScript(const String &name, bool moveToFront = true);

private:
    // LRU cache capacity
    static size_t _capacity;
    // Cache storage (front = most recently used)
    static std::list<LuaCacheEntry> _cacheList;
};

} // namespace nekos
