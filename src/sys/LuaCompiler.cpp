#include "LuaCompiler.hpp"
#include "FFat.h"
#include "lua.hpp"

#define LUA_COMPILE_BUFFER_SIZE 512

bool compileLuaFileIfNeeded(const char* basePath) {
    File luaFile = FFat.open(String(basePath) + ".lua", "r");
    if (!luaFile) {
        Serial.printf("[LuaCompiler] ⚠️ Source Lua file not found: %s.lua\n", basePath);
        return false;
    }

    Serial.printf("[LuaCompiler] Opened source file: %s.lua (%u bytes available)\n", basePath, luaFile.size());

    File outFile = FFat.open(String(basePath) + ".luac", "w");
    if (!outFile) {
        Serial.printf("[LuaCompiler] ⚠️ Failed to open output file: %s.luac\n", basePath);
        luaFile.close();
        return false;
    }

    Serial.printf("[LuaCompiler] Output file opened: %s.luac\n", basePath);

    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    Serial.printf("[LuaCompiler] Lua state created\n");

    char buffer[LUA_COMPILE_BUFFER_SIZE];
    size_t chunkCount = 0;
    while (luaFile.available()) {
        size_t n = luaFile.readBytes(buffer, LUA_COMPILE_BUFFER_SIZE);
        if (n == 0) break;

        Serial.printf("[LuaCompiler] Read %u bytes from %s.lua (chunk %u)\n", n, basePath, chunkCount);

        int res = luaL_loadbuffer(L, buffer, n, luaFile.name());
        if (res != LUA_OK) {
            Serial.printf("[LuaCompiler] Lua compile error: %s\n", lua_tostring(L, -1));
            lua_pop(L, 1);
            luaFile.close();
            outFile.close();
            lua_close(L);
            return false;
        }

        // Write compiled bytecode directly to file
        auto writerLambda = [](lua_State*, const void* p, size_t size, void* ud) -> int {
            File* f = reinterpret_cast<File*>(ud);
            if (f) {
                size_t written = f->write(reinterpret_cast<const uint8_t*>(p), size);
            }
            return 0;
        };

        if (lua_dump(L, writerLambda, &outFile, 0) != LUA_OK) {
            Serial.println("[LuaCompiler] Lua dump failed");
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

    Serial.printf("[LuaCompiler] ✅ Compiled %s.lua → %s.luac (%u chunks)\n", basePath, basePath, chunkCount);
    return true;
}

bool enumerateAndCompileLuaFiles(const char* basePath) {
    File dir = FFat.open(basePath);
    if (!dir || !dir.isDirectory()) {
        Serial.printf("[LuaCompiler] ⚠️ Not a directory: %s\n", basePath);
        return false;
    }

    Serial.printf("[LuaCompiler] Enumerating directory: %s\n", basePath);
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
                String base = String(basePath) + "/" + name.substring(0, name.length() - 4);
                Serial.printf("[LuaCompiler] Found Lua file: %s.lua\n", base.c_str());
                compileLuaFileIfNeeded(base.c_str());
            }
        }
        entry.close();
    }
    dir.close();
    return true;
}
