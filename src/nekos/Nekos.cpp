extern "C" {
    #include "lua.h"
    #include "lualib.h"
    #include "lauxlib.h"
}
#include "NekosSerial.h"
#include "Arduino.h"
#include "LuaScripts.h"
#include "Nekos.h"
#include "NekosLuaLoader.h"
#include "NekosFreeRTOS.h"
namespace nekos {

    // -------------------------------
    // Lua: scripts[name] = true
    // -------------------------------
    static int luaGetScripts(lua_State* L) {
        lua_newtable(L);
        if (luaScriptMap) {
            for (auto& kv : *luaScriptMap) {
                lua_pushboolean(L, true);
                lua_setfield(L, -2, kv.first.c_str());
            }
        }
        return 1;
    }

    // -------------------------------
    // Lua: getScript(name) -> source text
    // -------------------------------
    static int luaGetScript(lua_State* L) {
        const char* name = luaL_checkstring(L, 1);
        char* script = NekosLuaLoader::getScript(name, false);
        if (!script) {
            lua_pushnil(L);
            return 1;
        }
        lua_pushstring(L, script);
        return 1;
    }

    // -------------------------------
    // Lua: runScript(name, args?) -> true/false
    // -------------------------------
    static int luaRunScript(lua_State* L) {
        const char* name = luaL_checkstring(L, 1);
        const char* args = luaL_optstring(L, 2, nullptr);

        char* script = NekosLuaLoader::getScript(name, true);
        if (!script) {
            lua_pushboolean(L, false);
            return 1;
        }

        luaExec(script, args);
        lua_pushboolean(L, true);
        return 1;
    }

    // -------------------------------
    // Register Lua bindings
    // -------------------------------
    void registerLuaBindings(lua_State* L) {
        // Serial bindings
        registerSerialBindings(L);
        registerFreeRTOSBindings(L);
        // scripts = { "help" = true, ... }
        lua_pushcfunction(L, luaGetScripts);
        lua_call(L, 0, 1);
        lua_setglobal(L, "scripts");

        // getScript(name)
        lua_register(L, "getScript", luaGetScript);

        // runScript(name, args?)
        lua_register(L, "runScript", luaRunScript);
    }

    // -------------------------------
    // Lua execution wrapper
    // -------------------------------
    void luaExec(const char* script, const char* args = nullptr) {
        if (!script) return;
        lua_State* L = luaL_newstate();
        luaL_openlibs(L);
        registerLuaBindings(L);

        // Push args as a global variable
        if (args) {
            lua_pushstring(L, args);
            lua_setglobal(L, "arg");  // Lua can now access `arg`
        }

        if (luaL_dostring(L, script) != LUA_OK) {
            const char* err = lua_tostring(L, -1);
            Serial.printf("Lua Error: %s\n", err ? err : "Unknown error");
            lua_pop(L, 1);
        }

        lua_close(L);
    }

} // namespace nekos
