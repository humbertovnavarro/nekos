#include "FreeRTOS.h"
#include "freertos/task.h"
#include "FFat.h"
#include "sys/LuaModule.hpp"

LuaModule luaRequireModule("require", [](lua_State* L) {
    lua_pushcfunction(L, [](lua_State* L) -> int {
        const char* basePath = luaL_checkstring(L, 1);
        if (!basePath) {
            luaL_error(L, "require: expected a file path");
            return 0;
        }
        String luaPath = String(basePath) + ".lua";
        String luacPath = String(basePath) + ".luac";
        // Try loading precompiled .luac file first
        if (FFat.exists(luacPath)) {
            File f = FFat.open(luacPath, "r");
            if (!f) {
                luaL_error(L, "require: failed to open %s", luacPath.c_str());
                return 0;
            }
            size_t size = f.size();
            std::unique_ptr<char[]> buf(new char[size]);
            f.readBytes(buf.get(), size);
            f.close();

            if (luaL_loadbuffer(L, buf.get(), size, luacPath.c_str()) || lua_pcall(L, 0, 1, 0)) {
                luaL_error(L, "require: error executing %s: %s", luacPath.c_str(), lua_tostring(L, -1));
                return 0;
            }

            return 1;
        }
        // Try loading .lua source file
        if (FFat.exists(luaPath)) {
            File f = FFat.open(luaPath, "r");
            if (!f) {
                luaL_error(L, "require: failed to open %s", luaPath.c_str());
                return 0;
            }
            size_t size = f.size();
            std::unique_ptr<char[]> buf(new char[size]);
            f.readBytes(buf.get(), size);
            f.close();
            if (luaL_loadbuffer(L, buf.get(), size, luaPath.c_str()) != LUA_OK) {
                luaL_error(L, "require: compile error in %s: %s", luaPath.c_str(), lua_tostring(L, -1));
                return 0;
            }
            lua_pcall(L, 0, 1, 0);
            return 1;
        }
        luaL_error(L, "require: file not found: %s(.lua/.luac)", basePath);
        return 0;
    });
    return 1;
});
