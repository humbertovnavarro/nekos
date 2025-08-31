#pragma once

#include <Arduino.h>
#include <vector>
#include <map>
#include "LuaScripts.h"
#include "Nekos.h"

namespace nekos {

constexpr size_t SHELL_INPUT_BUFFER_SIZE = 128;
constexpr size_t HISTORY_SIZE = 16;

class Console {
public:
    static void begin(unsigned long baud);
    static void poll();

private:
    static char _lineBuf[SHELL_INPUT_BUFFER_SIZE];
    static size_t _lineLen;
    static size_t _cursorPos;

    static std::vector<String> _history;
    static int _histIndex;

    static void printPrompt();
    static void printRepeat(char c, size_t count);
    static void dispatchCommand(const char* line);
    static String completeCommand(const String &prefix);
};

} // namespace nekos
