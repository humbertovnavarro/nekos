#pragma once
#include "NekosMemConfig.h"
namespace nekos {
    class Console {
    public:
        static void begin(unsigned long baud = 115200);
        static void poll();
        static void log(const char *message);
        static void logf(const char *fmt, ...);
    private:
        static void printPrompt();
        static void _dispatchLine(const char* line);
        static char _lineBuf[SHELL_INPUT_BUFFER_SIZE];
        static size_t _lineIndex;
        static int _commandCount;
    };
}