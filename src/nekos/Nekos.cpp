extern "C" {
    #include "lua.h"
    #include "lualib.h"
    #include "lauxlib.h"
}
#include "NekosSerial.h"
#include "Arduino.h"
#include "LuaScripts.h"
#include "NekosFat.h"
#include "Nekos.h"

namespace nekos {
    // -------------------------------
    // Register Lua bindings
    // -------------------------------
    void registerLuaBindings(lua_State* L) {

        // Serial bindings
        registerSerialBindings(L);
        
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
