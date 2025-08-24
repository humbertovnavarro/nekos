extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}
#include "Arduino.h"
#include "NekosLua.h"
#include <Wire.h>
#include "FreeRTOS.h"
#include "task.h"
#include "NekosCommandRegistry.h"

namespace nekos {
    // Lua wrapper to execute a command
    int lua_executeCommand(lua_State* L) {
        // Expect: command name + args
        const char* name = luaL_checkstring(L, 1);
        const char* args = "";
        if (lua_gettop(L) >= 2) {
            args = luaL_checkstring(L, 2);
        }

        if (!CommandRegistry::commandExists(name)) {
            lua_pushboolean(L, false);
            lua_pushfstring(L, "Unknown command: %s", name);
            return 2;
        }

        Command* cmd = CommandRegistry::getCommand(name);

        // Parse arguments with ArgParse
        if (!cmd->args.parse(args)) {
            lua_pushboolean(L, false);
            lua_pushstring(L, cmd->args.usage(name).c_str());
            return 2;
        }

        // Run the command callback
        cmd->cb(cmd, args);

        lua_pushboolean(L, true);
        return 1;
    }

    void registerCommandBindings(lua_State* L) {
        lua_register(L, "run_command", lua_executeCommand);
    }


    // --- Register function helper ---
    static void registerFunction(lua_State *L, const char* name, lua_CFunction func) {
        lua_pushcfunction(L, func);
        lua_setglobal(L, name);
    }

    // FreeRTOS
    static int luaTaskDelay(lua_State* L) {
        int ms = luaL_checkinteger(L, 1);
        vTaskDelay(pdMS_TO_TICKS(ms));
        return 0;
    }

    String luaExec(const char* script) {
        if(!script) {
            return;
        }
        lua_State* L = luaL_newstate();
        luaL_openlibs(L);
        // Override print
        registerCommandBindings(L);
        if (luaL_dostring(L, script) != LUA_OK) {
            Serial.printf("Lua Error: %s\n", lua_tostring(L, -1));
        }
        lua_close(L);
    }
}
