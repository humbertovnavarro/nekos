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
#include "NekosMeta.h"

namespace nekos {

    void luaRegisterBindings(lua_State* L) {
        luaRegisterSerialBindings(L);
        luaRegisterTimeBindings(L);
        luaRegisterNekosMetaBindings(L);
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
