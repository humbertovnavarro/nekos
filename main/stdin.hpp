#pragma once
#include <Arduino.h>
#include "etl/pool.h"
#include "etl/vector.h"
#include "etl/string.h"

#ifndef LUA_MAX_ARGS
  #define LUA_MAX_ARGS    8
#endif
#ifndef LUA_MAX_ARG_LEN
  #define LUA_MAX_ARG_LEN 64
#endif
#ifndef LUA_QUEUE_DEPTH
  #define LUA_QUEUE_DEPTH 16
#endif
#ifndef STDIN_LINE_MAX
  #define STDIN_LINE_MAX  256
#endif
#ifndef STDIN_ARG_SLOTS
  #define STDIN_ARG_SLOTS (LUA_QUEUE_DEPTH + 4)
#endif

struct Arguments {
    using Arg  = etl::string<LUA_MAX_ARG_LEN>;
    using Argv = etl::vector<Arg, LUA_MAX_ARGS>;
    Argv argv;
    void        assign(int argc, const char *const *args);
    int         argc()      const { return static_cast<int>(argv.size()); }
    const char *arg(int i)  const { return argv[i].c_str(); }
};

class StdinArgPool {
public:
    static bool     init();
    static Arguments *alloc();
    static void     free(Arguments *blk);
    static size_t   available();
private:
    static etl::pool<Arguments, STDIN_ARG_SLOTS> _pool;
    static SemaphoreHandle_t                    _mutex;
};

class Stdin {
public:
    using Line = etl::string<STDIN_LINE_MAX>;
    static int readline(Line &out);
    static int tokenise(Line &line, Arguments &blk);
};