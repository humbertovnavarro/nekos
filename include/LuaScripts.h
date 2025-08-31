#pragma once
#include <Arduino.h>
#include <map>
#include <pgmspace.h>

static const char help_script[] PROGMEM = "print(\"\\n=== Lua Scripts ===\")\n"
"if bakedScripts then\n"
"    for name, _ in pairs(bakedScripts) do\n"
"        print(\"- \" .. name)\n"
"    end\n"
"else\n"
"    print(\"No baked scripts available\")\n"
"end";

// Lua script map (runtime map pointing to flash strings)
static std::map<String, const char*>* luaScriptMap = nullptr;

static void initLuaScriptMap() {
    if (luaScriptMap) return;
    luaScriptMap = new std::map<String, const char*>();
    (*luaScriptMap)["help"] = help_script;
}
