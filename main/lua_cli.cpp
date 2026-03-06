#include <Arduino.h>
#include <cstring>
#include "freertos/task.h"
#include "lua_runner.hpp"
#include "stdin.hpp"
#include "lua_cli.hpp"

namespace cli {

void put(const char *s) { Serial.print(s); }

bool builtin(const Arguments &blk)
{
    if (blk.argc() == 0) return true;

    if (strcmp(blk.arg(0), "help") == 0) {
        put("Commands: any .lua script in " LUA_BIN_PATH " (omit .lua),\r\n"
            "          or an absolute path.\r\n"
            "Built-ins: help, clear\r\n");
        return true;
    }
    if (strcmp(blk.arg(0), "clear") == 0) {
        put("\x1b[2J\x1b[H");
        return true;
    }
    return false;
}

void execute(Stdin::Line &line)
{
    Arguments *blk = StdinArgPool::alloc();
    if (Stdin::tokenise(line, *blk) == 0) { StdinArgPool::free(blk); return; }
    if (builtin(*blk))                    { StdinArgPool::free(blk); return; }
    char path[256];
    const char *script = blk->arg(0);
    if (script[0] == '/')
        snprintf(path, sizeof(path), "%s", script);
    else
        snprintf(path, sizeof(path), "%s/%s.lua", LUA_BIN_PATH, script);
    blk->argv.erase(blk->argv.begin());
    if (!lua_submit(path, blk)) {
        StdinArgPool::free(blk);
        put("error: failed to submit job\r\n");
    }
}

void shell_loop()
{
    Stdin::Line line;
    for (;;) {
        put("\x1b[32m$ \x1b[0m");
        if (Stdin::readline(line) > 0)
            execute(line);
        vTaskDelay(1);
    }
}

void task_fn(void *)
{
    vTaskDelay(pdMS_TO_TICKS(200));
    if (!StdinArgPool::init()) {
        put("FATAL: StdinArgPool init failed\r\n");
        vTaskDelete(nullptr);
    }
    put("\r\nLua CLI ready. Type 'help'.\r\n");
    shell_loop();
}

} // namespace cli