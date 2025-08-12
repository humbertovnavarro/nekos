#pragma once
#include <Arduino.h>
using CommandCallback = void(*)(const char* args);

#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_RESET   "\x1b[0m"

class Console {
public:
    static constexpr int MAX_COMMANDS = 64;
    static constexpr int SHELL_INPUT_BUFFER_SIZE = 256;
    static constexpr int QUEUE_MSG_SIZE = 256;
    // Initialize serial console at given baud rate
    static void begin(unsigned long baud = 115200);

    // Call repeatedly in your loop()
    static void poll();

    // Register a command by name and callback (callback receives args string)
    static bool registerCommand(const char* name, CommandCallback cb);

    // Helpers for listing commands
    static int getCommandCount();
    static const char* getCommandName(int index);

    // Logging helpers
    static void log(const char *message);
    static void logf(const char *fmt, ...);
    static void setDeviceName(const char* name);
private:
    struct Command {
        char name[32];
        CommandCallback cb;
    };
    static void printPrompt();
    static void _dispatchLine(const char* line);

    static char _lineBuf[SHELL_INPUT_BUFFER_SIZE];
    static size_t _lineIndex;

    static Command _commands[MAX_COMMANDS];
    static int _commandCount;
};
