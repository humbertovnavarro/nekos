#pragma once
#include <functional>
#include <string>
#include <unordered_map>
#include "lua.hpp"

using LuaModuleFactory = void(*)(lua_State*);

class LuaModule {
public:
    const char* name;
    LuaModuleFactory moduleFactory;
    LuaModule(const char* n, LuaModuleFactory f);
    static void begin(lua_State* L);
    static void addFunction(lua_State* L, const char* name, lua_CFunction func);
    static void addString(lua_State* L, const char* name, const char* value);
    static void addInteger(lua_State* L, const char* name, lua_Integer value);
    static void addBoolean(lua_State* L, const char* name, bool value);
    static void addNumber(lua_State* L, const char* name, lua_Number value);
};

extern std::unordered_map<std::string, std::function<void(lua_State* L)>> luaModuleMap;
void registerLuaModule(const LuaModule& mod);
int luaRequire(lua_State* L);
void exposeRequire(lua_State* L);
