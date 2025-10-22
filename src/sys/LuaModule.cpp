#include "LuaModule.hpp"

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

// ===================== Module registry =====================
std::unordered_map<std::string, std::function<void(lua_State* L)>> luaModuleMap;

void registerLuaModule(const LuaModule& mod) {
    luaModuleMap[mod.name] = mod.moduleFactory;
}

int luaRequire(lua_State* L) {
    const char* name = luaL_checkstring(L, 1);
    auto it = luaModuleMap.find(name);
    if (it != luaModuleMap.end()) {
        it->second(L);
        return 1;
    }
    return luaL_error(L, "Module '%s' not found", name);
}

void exposeRequire(lua_State* L) {
    lua_pushcfunction(L, luaRequire);
    lua_setglobal(L, "require");
}