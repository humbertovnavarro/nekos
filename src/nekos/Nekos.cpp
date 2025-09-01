extern "C" {
    #include "lua.h"
    #include "lualib.h"
    #include "lauxlib.h"
}

#include "NekosSerial.h"
#include "Arduino.h"
#include "LuaScripts.h"
#include "Nekos.h"
#include "NekosLuaScheduler.h"
#include "vector"
#include "NekosTime.h"
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
        char* script = LuaScheduler::getScript(name, false);
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

        char* script = LuaScheduler::getScript(name, true);
        if (!script) {
            lua_pushboolean(L, false);
            return 1;
        }
        LuaScheduler::exec(script, args);
        lua_pushboolean(L, true);
        return 1;
    }

    void luaRegisterBindings(lua_State* L) {
        // ------------------- Serial bindings -------------------
        // e.g., Serial.read(), Serial.write(), etc.
        luaRegisterSerialBindings(L);
        luaRegisterTimeBindings(L);
        // ------------------- Scripts table -------------------
        // Creates Lua table `scripts` containing available script names
        lua_pushcfunction(L, luaGetScripts); // C function returning table
        lua_call(L, 0, 1);                   // call with 0 args, expect 1 return
        lua_setglobal(L, "scripts");         // set as global `scripts`

        // ------------------- getScript(name) -------------------
        // Lua function: returns content of a script by name
        lua_register(L, "getScript", luaGetScript);

        // ------------------- runScript(name, args?) -------------------
        // Lua function: executes a script by name with optional arguments
        lua_register(L, "runScript", luaRunScript);
    }

    void luaExec(const char* script, const char* argline = nullptr) {
        if (!script) return;

        lua_State* L = luaL_newstate();
        luaL_openlibs(L);
        luaRegisterBindings(L);

        // Wrap the script in a function
        String wrapped = "return function(...)\n";
        wrapped += script;
        wrapped += "\nend";

        if (luaL_loadstring(L, wrapped.c_str()) != LUA_OK) {
            const char* err = lua_tostring(L, -1);
            Serial.printf("Lua Load Error: %s\n", err ? err : "Unknown error");
            lua_pop(L, 1);
            lua_close(L);
            return;
        }

        // Call the loader to get the function
        if (lua_pcall(L, 0, 1, 0) != LUA_OK) {
            const char* err = lua_tostring(L, -1);
            Serial.printf("Lua Runtime Error: %s\n", err ? err : "Unknown error");
            lua_pop(L, 1);
            lua_close(L);
            return;
        }

        // Now the wrapped function is on top
        int nargs = 0;

        if (argline && *argline) {
            const char* ptr = argline;
            while (*ptr) {
                while (*ptr == ' ') ptr++;
                if (*ptr == '\0') break;

                String token;
                if (*ptr == '"') {
                    ptr++; // skip opening quote
                    while (*ptr && *ptr != '"') {
                        token += *ptr;
                        ptr++;
                    }
                    if (*ptr == '"') ptr++; // skip closing quote
                } else {
                    while (*ptr && *ptr != ' ') {
                        token += *ptr;
                        ptr++;
                    }
                }

                lua_pushstring(L, token.c_str());
                nargs++;
            }
        }

        // Call the wrapped function with nargs arguments
        if (lua_pcall(L, nargs, LUA_MULTRET, 0) != LUA_OK) {
            const char* err = lua_tostring(L, -1);
            Serial.printf("Lua Runtime Error: %s\n", err ? err : "Unknown error");
            lua_pop(L, 1);
        }

        lua_close(L);
    }

    bool luaRunReplLine(lua_State* L, const char* line) {
        int status;

        // Avoid dynamic String allocation: fixed buffer
        char expr[256];
        snprintf(expr, sizeof(expr), "return %s", line);

        // 1. Try "return <line>" first
        status = luaL_loadstring(L, expr);
        if (status != LUA_OK) {
            // Try normal statement
            status = luaL_loadstring(L, line);
            if (status != LUA_OK) {
                const char* err = lua_tostring(L, -1);
                Serial.printf("Lua Syntax Error: %s\n", err ? err : "Unknown error");
                lua_pop(L, 1);
                return false;
            }
        }

        // Execute the chunk
        status = lua_pcall(L, 0, LUA_MULTRET, 0);
        if (status != LUA_OK) {
            const char* err = lua_tostring(L, -1);
            Serial.printf("Lua Runtime Error: %s\n", err ? err : "Unknown error");
            lua_pop(L, 1);
            return false;
        }

        // Print returned values safely
        int nresults = lua_gettop(L);           // number of return values
        for (int i = 1; i <= nresults; ++i) {
            if (lua_isstring(L, i)) Serial.print(lua_tostring(L, i));
            else if (lua_isnumber(L, i)) Serial.print(lua_tonumber(L, i));
            else if (lua_isboolean(L, i)) Serial.print(lua_toboolean(L, i) ? "true" : "false");
            else if (lua_isnil(L, i)) Serial.print("nil");
            else Serial.print(luaL_typename(L, i));

            if (i < nresults) Serial.print("\t");
        }
        if (nresults > 0) Serial.println();

        lua_settop(L, 0);  // clear all return values
        return true;
    }




} // namespace nekos
