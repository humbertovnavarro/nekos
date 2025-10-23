#include "Arduino.h"
#include "FFat.h"
#include "sys/LuaModule.hpp"

inline LuaModule luaFileSystemModule("fs", [](lua_State* L) {
    LuaModule::begin(L);

    // Mount FFat
    LuaModule::addFunction(L, "mount", [](lua_State* L) -> int {
        bool formatIfFail = lua_toboolean(L, 1);
        bool result = FFat.begin(formatIfFail);
        lua_pushboolean(L, result);
        return 1;
    });

    // Unmount FFat
    LuaModule::addFunction(L, "unmount", [](lua_State* L) -> int {
        FFat.end();
        return 0;
    });

    // Check if a file exists
    LuaModule::addFunction(L, "exists", [](lua_State* L) -> int {
        const char* path = luaL_checkstring(L, 1);
        bool exists = FFat.exists(path);
        lua_pushboolean(L, exists);
        return 1;
    });

    // Remove a file
    LuaModule::addFunction(L, "remove", [](lua_State* L) -> int {
        const char* path = luaL_checkstring(L, 1);
        bool result = FFat.remove(path);
        lua_pushboolean(L, result);
        return 1;
    });

    // Make a directory
    LuaModule::addFunction(L, "mkdir", [](lua_State* L) -> int {
        const char* path = luaL_checkstring(L, 1);
        bool result = FFat.mkdir(path);
        lua_pushboolean(L, result);
        return 1;
    });

    // Remove a directory
    LuaModule::addFunction(L, "rmdir", [](lua_State* L) -> int {
        const char* path = luaL_checkstring(L, 1);
        bool result = FFat.rmdir(path);
        lua_pushboolean(L, result);
        return 1;
    });

    // Rename (move) a file or directory
    LuaModule::addFunction(L, "rename", [](lua_State* L) -> int {
        const char* from = luaL_checkstring(L, 1);
        const char* to = luaL_checkstring(L, 2);
        bool result = FFat.rename(from, to);
        lua_pushboolean(L, result);
        return 1;
    });

    // Read an entire file into a string
    LuaModule::addFunction(L, "read", [](lua_State* L) -> int {
        const char* path = luaL_checkstring(L, 1);
        File file = FFat.open(path, "r");
        if (!file) {
            lua_pushnil(L);
            lua_pushstring(L, "failed to open file");
            return 2;
        }

        String content;
        while (file.available()) {
            content += (char)file.read();
        }
        file.close();

        lua_pushlstring(L, content.c_str(), content.length());
        return 1;
    });

    // Write (overwrite) file with string content
    LuaModule::addFunction(L, "write", [](lua_State* L) -> int {
        const char* path = luaL_checkstring(L, 1);
        size_t len;
        const char* data = luaL_checklstring(L, 2, &len);

        File file = FFat.open(path, "w");
        if (!file) {
            lua_pushboolean(L, false);
            return 1;
        }

        size_t written = file.write((const uint8_t*)data, len);
        file.close();

        lua_pushboolean(L, written == len);
        return 1;
    });

    // Append text to a file
    LuaModule::addFunction(L, "append", [](lua_State* L) -> int {
        const char* path = luaL_checkstring(L, 1);
        size_t len;
        const char* data = luaL_checklstring(L, 2, &len);

        File file = FFat.open(path, "a");
        if (!file) {
            lua_pushboolean(L, false);
            return 1;
        }

        size_t written = file.write((const uint8_t*)data, len);
        file.close();

        lua_pushboolean(L, written == len);
        return 1;
    });

    // List directory contents
    LuaModule::addFunction(L, "list", [](lua_State* L) -> int {
        const char* path = luaL_optstring(L, 1, "/");
        File dir = FFat.open(path);
        if (!dir || !dir.isDirectory()) {
            lua_pushnil(L);
            lua_pushstring(L, "not a directory");
            return 2;
        }

        lua_newtable(L);
        int index = 1;

        File entry = dir.openNextFile();
        while (entry) {
            lua_newtable(L);

            lua_pushstring(L, "name");
            lua_pushstring(L, entry.name());
            lua_settable(L, -3);

            lua_pushstring(L, "size");
            lua_pushinteger(L, entry.size());
            lua_settable(L, -3);

            lua_pushstring(L, "isDir");
            lua_pushboolean(L, entry.isDirectory());
            lua_settable(L, -3);

            lua_rawseti(L, -2, index++);
            entry = dir.openNextFile();
        }

        return 1;
    });
});
