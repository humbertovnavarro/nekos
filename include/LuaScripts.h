// Auto-generated Lua scripts in PROGMEM

#pragma once
#include <Arduino.h>
#include <map>
#include <pgmspace.h>
#include "esp_heap_caps.h"


// Lua script map (runtime map pointing to flash strings)
static std::map<String, const char*>* luaScriptMap = nullptr;

static void initLuaScriptMap() {
    if (luaScriptMap) return;
    luaScriptMap = (std::map<String, const char*>*) heap_caps_malloc(
        sizeof(std::map<String, const char*>), MALLOC_CAP_SPIRAM
    );
    if (!luaScriptMap) {
        luaScriptMap = new std::map<String, const char*>(); // fallback to normal heap
    } else {
        new (luaScriptMap) std::map<String, const char*>(); // placement new in PSRAM
    }
}
