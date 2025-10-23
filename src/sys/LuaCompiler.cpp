#include "LuaCompiler.hpp"
#include "FFat.h"
#include "lua.hpp"

#define LUA_COMPILE_BUFFER_SIZE 512

bool compileLuaFileIfNeeded(const char* basePath) {
    File luaFile = FFat.open(String(basePath) + ".lua", "r");

    if (!luaFile) {
        Serial.printf("[LuaPreCompiler] ⚠️ Source Lua file not found: %s.lua\n", basePath);
        return false;
    }

    Serial.printf("[LuaPreCompiler] Opened source file: %s.lua (%u bytes available)\n", basePath, luaFile.size());

    File outFile = FFat.open(String(basePath) + ".luac", "w");

    if (!outFile) {
        Serial.printf("[LuaPreCompiler] ⚠️ Failed to open output file: %s.luac\n", basePath);
        luaFile.close();
        return false;
    }

    Serial.printf("[LuaPreCompiler] Output file opened: %s.luac\n", basePath);

    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    Serial.printf("[LuaPreCompiler] Lua state created\n");

    char buffer[LUA_COMPILE_BUFFER_SIZE];
    size_t chunkCount = 0;

    while (luaFile.available()) {
        size_t n = luaFile.readBytes(buffer, LUA_COMPILE_BUFFER_SIZE);
        if (n == 0) break;
        int res = luaL_loadbuffer(L, buffer, n, luaFile.name());
        if (res != LUA_OK) {
            Serial.printf("[LuaPreCompiler] Lua compile error: %s\n", lua_tostring(L, -1));
            lua_pop(L, 1);
            luaFile.close();
            outFile.close();
            lua_close(L);
            return false;
        }
        auto writerLambda = [](lua_State*, const void* p, size_t size, void* ud) -> int {
            File* f = reinterpret_cast<File*>(ud);
            if (f) {
                size_t written = f->write(reinterpret_cast<const uint8_t*>(p), size);
            }
            return 0;
        };

        if (lua_dump(L, writerLambda, &outFile, 0) != LUA_OK) {
            Serial.println("[LuaPreCompiler] Lua dump failed");
            luaFile.close();
            outFile.close();
            lua_close(L);
            return false;
        }
        chunkCount++;
    }
    luaFile.close();
    outFile.close();
    lua_close(L);
    Serial.printf("[LuaPreCompiler] ✅ Compiled %s.lua → %s.luac (%u chunks)\n", basePath, basePath, chunkCount);
    return true;
}

bool enumerateAndCompileLuaFiles(const char* basePath) {
    File dir = FFat.open(basePath);
    if (!dir || !dir.isDirectory()) {
        Serial.printf("[LuaPreCompiler] ⚠️ Not a directory: %s\n", basePath);
        return false;
    }

    Serial.printf("[LuaPreCompiler] Enumerating directory: %s\n", basePath);
    File entry;
    dir.rewindDirectory();
    while ((entry = dir.openNextFile())) {
        if (entry.isDirectory()) {
            char subDirPath[256];
            snprintf(subDirPath, sizeof(subDirPath), "%s/%s", basePath, entry.name());
            enumerateAndCompileLuaFiles(subDirPath);
        } else {
            String name = entry.name();
            if (name.endsWith(".lua")) {
                String base = String(basePath) + name.substring(0, name.length() - 4);
                Serial.printf("[LuaPreCompiler] Found Lua file: %s.lua\n", base.c_str());
                compileLuaFileIfNeeded(base.c_str());
            }
        }
        entry.close();
    }
    dir.close();
    return true;
}
