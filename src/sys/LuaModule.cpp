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

const char* luaSearchPaths[] = {"/lib/", "/modules/", "/"};
const size_t luaSearchPathsCount = sizeof(luaSearchPaths) / sizeof(luaSearchPaths[0]);

int luaRequire(lua_State* L) {
    const char* name = luaL_checkstring(L, 1);
    auto it = luaModuleMap.find(name);
    if (it != luaModuleMap.end()) {
        it->second(L);
        return 1;
    }
    char path[128];
    for (size_t i = 0; i < luaSearchPathsCount; i++) {
        snprintf(path, sizeof(path), "%s%s.luac", luaSearchPaths[i], name);
        if (FFat.exists(path)) {
            File f = FFat.open(path, "r");
            if (!f) {
                return luaL_error(L, "Failed to open module file '%s'", path);
            }
            size_t size = f.size();
            char* buffer = new char[size];
            f.readBytes(buffer, size);
            f.close();
            if (luaL_loadbuffer(L, buffer, size, name) != LUA_OK) {
                delete[] buffer;
                return lua_error(L);
            }
            delete[] buffer;
            if (lua_pcall(L, 0, 1, 0) != LUA_OK) {
                return lua_error(L);
            }
            return 1;
        }
    }
    return luaL_error(L, "Module '%s' not found", name);
}

void exposeRequire(lua_State* L) {
    lua_pushcfunction(L, luaRequire);
    lua_setglobal(L, "require");
}