#pragma once

#include "esp32-hal.h"
#include <cerrno>
#include <sys/stat.h>
#include <sys/unistd.h>
extern "C" {
    #include "lua.h"
    #include "lauxlib.h"
}
#include "sys/dirent.h"
#include <cstring>
#include <cstdio>
#include <pgmspace.h>

#ifndef LUA_BIN_PATH
#  define LUA_BIN_PATH "/lua"
#endif

namespace lua_utils {

// ─── C bindings ───────────────────────────────────────────────────────────────

static int lua_readdir_iter(lua_State *L) {
    DIR **dp = (DIR **)lua_touserdata(L, lua_upvalueindex(1));
    if (!*dp) { lua_pushnil(L); return 1; }
    struct dirent *ent = readdir(*dp);
    if (!ent) { closedir(*dp); *dp = nullptr; lua_pushnil(L); return 1; }
    lua_pushstring(L, ent->d_name);
    return 1;
}
static int lua_readdir(lua_State *L) {
    const char *path = luaL_checkstring(L, 1);
    DIR **dp = (DIR **)lua_newuserdata(L, sizeof(DIR *));
    *dp = opendir(path);
    if (!*dp) { lua_pushnil(L); lua_pushstring(L, strerror(errno)); return 2; }
    lua_pushcclosure(L, lua_readdir_iter, 1);
    return 1;
}
static int lua_mkdir_bind(lua_State *L) {
    const char *path = luaL_checkstring(L, 1);
    if (::mkdir(path, 0777) == 0) { lua_pushboolean(L, 1); return 1; }
    lua_pushnil(L); lua_pushstring(L, strerror(errno)); return 2;
}
static int lua_getcwd(lua_State *L) {
    char buf[256];
    if (getcwd(buf, sizeof(buf))) lua_pushstring(L, buf);
    else lua_pushstring(L, "/");
    return 1;
}
static int lua_millis(lua_State *L) {
    lua_pushinteger(L, (lua_Integer)millis()); return 1;
}
static int lua_delay_ms(lua_State *L) {
    vTaskDelay(pdMS_TO_TICKS((int)luaL_checkinteger(L, 1))); return 0;
}
static int lua_esp_free_heap(lua_State *L) {
    lua_pushinteger(L, (lua_Integer)esp_get_free_heap_size()); return 1;
}
static int lua_esp_min_heap(lua_State *L) {
    lua_pushinteger(L, (lua_Integer)esp_get_minimum_free_heap_size()); return 1;
}
static int lua_esp_free_psram(lua_State *L) {
    lua_pushinteger(L, (lua_Integer)esp_get_free_internal_heap_size()); return 1;
}
static int lua_esp_restart(lua_State *L) {
    esp_restart(); return 0;
}

inline void open(lua_State *L) {
    lua_register(L, "readdir",  lua_readdir);
    lua_register(L, "mkdir",    lua_mkdir_bind);
    lua_register(L, "getcwd",   lua_getcwd);
    lua_register(L, "millis",   lua_millis);
    lua_register(L, "delay_ms", lua_delay_ms);

    lua_newtable(L);
    lua_pushcfunction(L, lua_esp_free_heap);  lua_setfield(L, -2, "free_heap");
    lua_pushcfunction(L, lua_esp_min_heap);   lua_setfield(L, -2, "min_heap");
    lua_pushcfunction(L, lua_esp_free_psram); lua_setfield(L, -2, "free_psram");
    lua_pushcfunction(L, lua_esp_restart);    lua_setfield(L, -2, "restart");
    lua_setglobal(L, "esp");
}

struct _Builtin { const char *name; const char *src; };

inline const _Builtin* _builtins() {
static const _Builtin table[] = {
        { "ls", 
R"lua(
local path = arg[1] or "/"
local t = {}
for name in readdir(path) do
    if name ~= "." and name ~= ".." then t[#t+1] = name end
end
table.sort(t)
for _, n in ipairs(t) do print(n) end
)lua"},
        { "mkdir",
R"lua(
local path = arg[1]
if not path then print("usage: mkdir <path>");  end
local ok, err = mkdir(path)
if not ok then print("mkdir: " .. tostring(err)) end
)lua"},
        { "rm",
R"lua(
local path = arg[1]
if not path then print("usage: rm <path>");  end
local ok, err = os.remove(path)
if not ok then print("rm: " .. tostring(err)) end
)lua"},
        { "mv",
R"lua(
local src, dst = arg[1], arg[2]
if not src or not dst then print("usage: mv <src> <dst>");  end
local ok, err = os.rename(src, dst)
if not ok then print("mv: " .. tostring(err)) end
)lua"},
        { "cat",
R"lua(
local path = arg[1]
if not path then print("usage: cat <file>");  end
local f, err = io.open(path, "r")
if not f then print("cat: " .. tostring(err));  end
io.write(f:read("*a"))
f:close()
)lua"},
        { "cp",
R"lua(
local src, dst = arg[1], arg[2]
if not src or not dst then print("usage: cp <src> <dst>");  end
local fi, err = io.open(src, "rb")
if not fi then print("cp: " .. tostring(err));  end
local fo, err2 = io.open(dst, "wb")
if not fo then fi:close(); print("cp: "..tostring(err2));  end
while true do
    local data = fi:read(512)
    if not data or #data == 0 then break end
    fo:write(data)
end
fi:close(); fo:close()
)lua"},
        { "top",
R"lua(
local iters    = tonumber(arg[1]) or 5
local interval = tonumber(arg[2]) or 1000
for i = 1, iters do
    local free_heap  = esp.free_heap  and esp.free_heap()  or collectgarbage("count")*1024
    local min_heap   = esp.min_heap   and esp.min_heap()   or 0
    local free_psram = esp.free_psram and esp.free_psram() or 0
    local uptime_s   = math.floor((millis and millis() or 0) / 1000)
    print(string.format("[%3ds]  heap: %6d B free  (min %6d)  psram: %6d B free",
        uptime_s, free_heap, min_heap, free_psram))
    if i < iters then delay_ms(interval) end
end
)lua"},
        { "uptime",
R"lua(
local ms = millis()
local s = math.floor(ms/1000)
local m = math.floor(s/60)
local h = math.floor(m/60)
print(string.format("up %dh %02dm %02ds  (%d ms)", h, m%60, s%60, ms))
)lua"},
        { "free",
R"lua(
local heap  = esp.free_heap  and esp.free_heap()  or 0
local psram = esp.free_psram and esp.free_psram() or 0
print(string.format("heap:  %d bytes free", heap))
print(string.format("psram: %d bytes free", psram))
print(string.format("lua:   %.1f KB used", collectgarbage("count")))
)lua"},
        { "reboot",
R"lua(
print("Rebooting...")
delay_ms(200)
esp.restart()
)lua"},
        { "echo",
R"lua(
local parts = {}
for i = 1, #arg do parts[i] = tostring(arg[i]) end
print(table.concat(parts, " "))
)lua"},
        { "pwd",
R"lua(
print(getcwd and getcwd() or "/")
)lua"},
        { "touch",
R"lua(
local path = arg[1]
if not path then print("usage: touch <file>");  end
local f, err = io.open(path, "ab")
if not f then print("touch: " .. tostring(err));  end
f:close()
)lua"},
        { "wc",
R"lua(
local path = arg[1]
if not path then print("usage: wc <file>");  end
local f, err = io.open(path, "r")
if not f then print("wc: " .. tostring(err));  end
local lines, words, bytes = 0, 0, 0
for line in f:lines() do
    lines = lines + 1
    bytes = bytes + #line + 1
    for _ in line:gmatch("%S+") do words = words + 1 end
end
f:close()
print(string.format("%6d %6d %6d  %s", lines, words, bytes, path))
)lua"},
        { nullptr, nullptr },
    };
    return table;
}

inline void install(bool force = false) {
    struct stat st{};
    if (stat(LUA_BIN_PATH, &st) != 0)
        ::mkdir(LUA_BIN_PATH, 0777);

    for (const _Builtin *b = _builtins(); b->name; b++) {
        char path[128];
        snprintf(path, sizeof(path), "%s/%s.lua", LUA_BIN_PATH, b->name);
        if (!force) {
            FILE *probe = fopen(path, "r");
            if (probe) { fclose(probe); continue; }
        }
        FILE *f = fopen(path, "w");
        if (!f) continue;
        fputs(b->src, f);
        fclose(f);
    }
}

inline bool find(const char *name, char *out, int out_sz) {
    snprintf(out, out_sz, "%s/%s.lua", LUA_BIN_PATH, name);
    struct stat st{};
    return stat(out, &st) == 0;
}

} // namespace lua_utils