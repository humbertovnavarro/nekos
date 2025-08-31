#pragma once
#include <Arduino.h>
#include <map>
#include <list>
#include "LuaScripts.h"

namespace nekos {

struct LuaCacheEntry {
    String name;
    char* data;
    size_t size;
};

class NekosLuaLoader {
public:
    static void begin(size_t cacheCapacity = 8);
    static void exec(const String &name, const char* args = nullptr);
    static bool exists(const String &name);
    static void listScripts();
    static char* getScript(const String &name, bool moveToFront = true);

private:
    static size_t _capacity;
    static std::list<LuaCacheEntry> _cacheList;

};

} // namespace nekos
