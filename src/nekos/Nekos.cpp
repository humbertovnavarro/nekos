extern "C" {
    #include "lua.h"
    #include "lualib.h"
    #include "lauxlib.h"
}

#include "Arduino.h"
#include "LuaScripts.h"
#include "NekosFat.h"
#include "Nekos.h"

namespace nekos {

    // -------------------------------
    // Lua: print → Serial
    // -------------------------------
    static int luaSerialPrint(lua_State* L) {
        int nargs = lua_gettop(L);
        for (int i = 1; i <= nargs; ++i) {
            size_t len;
            const char* str = luaL_tolstring(L, i, &len);
            if (str) {
                Serial.write(str, len);
                lua_pop(L, 1); // pop luaL_tolstring result
            }
            if (i < nargs) Serial.write('\t');
        }
        Serial.write('\n');
        return 0;
    }

    // -------------------------------
    // Register Lua bindings
    // -------------------------------
    void registerLuaBindings(lua_State* L) {
        
        // Print -> serial print
        lua_pushcfunction(L, luaSerialPrint);
        lua_setglobal(L, "print");

        // FFat bindings
        registerFFatBindings(L);

        // Expose scripts
        lua_newtable(L);
        int index = 1;
        for (auto &kv : luaScriptMap) {
            lua_pushstring(L, kv.second);   // value = script
            lua_setfield(L, -2, kv.first.c_str()); // key = script name
            index++;
        }

        lua_setglobal(L, "bakedScripts"); // global.bakedScripts = table
    }

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


}
