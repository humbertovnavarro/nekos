#include "LuaModule.hpp"
#include "FFat.h"
#include <vector>
#include <functional>
#include <string>

// ===================== LuaModule constructors =====================
LuaModule::LuaModule(const char* n, LuaModuleFactory f)
    : name(n), moduleFactory(f) {}

// ===================== Table helpers =====================
void LuaModule::begin(lua_State* L) {
    lua_newtable(L);
}

void LuaModule::addFunction(lua_State* L, const char* name, lua_CFunction func) {
    lua_pushcfunction(L, func);
    lua_setfield(L, -2, name);
}

void LuaModule::addString(lua_State* L, const char* name, const char* value) {
    lua_pushstring(L, value);
    lua_setfield(L, -2, name);
}

void LuaModule::addInteger(lua_State* L, const char* name, lua_Integer value) {
    lua_pushinteger(L, value);
    lua_setfield(L, -2, name);
}

void LuaModule::addBoolean(lua_State* L, const char* name, bool value) {
    lua_pushboolean(L, value);
    lua_setfield(L, -2, name);
}

void LuaModule::addNumber(lua_State* L, const char* name, lua_Number value) {
    lua_pushnumber(L, value);
    lua_setfield(L, -2, name);
}