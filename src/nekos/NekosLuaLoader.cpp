#include "NekosLuaLoader.h"
#include "Nekos.h" // for luaExec, luaScriptMap
#include "esp_heap_caps.h"

namespace nekos {

// ------------------- Static member definitions -------------------
size_t NekosLuaLoader::_capacity = 8;
std::list<LuaCacheEntry> NekosLuaLoader::_cacheList;

// ------------------- Public -------------------
void NekosLuaLoader::begin(size_t cacheCapacity) {
    if (!luaScriptMap) initLuaScriptMap();
    _capacity = cacheCapacity;
}

void NekosLuaLoader::exec(const String &name, const char* args) {
    char* buf = getScript(name);
    if (!buf) {
        Serial.print("Command '");
        Serial.print(name);
        Serial.println("' not found");
        return;
    }
    luaExec(buf, args);
}

bool NekosLuaLoader::exists(const String &name) {
    return getScript(name, false) != nullptr;
}

void NekosLuaLoader::listScripts() {
    if (!luaScriptMap) return;
    Serial.println("=== Lua Scripts ===");
    for (auto &p : *luaScriptMap) {
        Serial.print("- ");
        Serial.println(p.first);
    }
}

// ------------------- Private -------------------
char* NekosLuaLoader::getScript(const String &name, bool moveToFront) {
    // Check cache
    for (auto it = _cacheList.begin(); it != _cacheList.end(); ++it) {
        if (it->name == name) {
            if (moveToFront) _cacheList.splice(_cacheList.begin(), _cacheList, it);
            return it->data;
        }
    }

    // Not in cache, load from PROGMEM/PSRAM
    if (!luaScriptMap || luaScriptMap->find(name) == luaScriptMap->end()) return nullptr;
    const char* progmemScript = luaScriptMap->at(name);
    size_t len = strlen_P(progmemScript) + 1;

    char* buf = (char*) heap_caps_malloc(len, MALLOC_CAP_SPIRAM);
    if (!buf) buf = (char*) malloc(len);
    if (!buf) return nullptr;

    strcpy_P(buf, progmemScript);

    _cacheList.push_front({name, buf, len});

    if (_cacheList.size() > _capacity) {
        auto &last = _cacheList.back();
        if (last.data) free(last.data);
        _cacheList.pop_back();
    }

    return buf;
}

} // namespace nekos
