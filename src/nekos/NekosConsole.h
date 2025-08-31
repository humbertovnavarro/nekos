#pragma once
#include "FreeRTOS.h"
namespace nekos {
    #define SHELL_INPUT_BUFFER_SIZE 256
    #define QUEUE_MSG_SIZE 256
    class Console {
    public:
        // Initialize serial console at given baud rate
        static void begin(unsigned long baud = 115200);
        // Call repeatedly in your loop()
        static void poll();
        // Logging helpers
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