#pragma once
#include "FreeRTOS.h"
#include "freertos/task.h"
#include "stdin.hpp"

#ifndef LUA_BIN_PATH
  #define LUA_BIN_PATH "/lua"
#endif

namespace cli {
    void put(const char *s);
    bool builtin(const Arguments &blk);
    void execute(Stdin::Line &line);
    void shell_loop();
    void task_fn(void *);
} // namespace cli