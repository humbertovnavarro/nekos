#include "Console.h"
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <WiFi.h> 
#define BANNER

char Console::_lineBuf[Console::SHELL_INPUT_BUFFER_SIZE];
size_t Console::_lineIndex = 0;
Console::Command Console::_commands[Console::MAX_COMMANDS] = {};
int Console::_commandCount = 0;

void Console::begin(unsigned long baud) {
    Serial.begin(115200);
    delay(100);
    const char* banner[] = {
    "      |\\---/|",
    "      | ,_, |",
    "       \\_`_/-..----.",
    "    ___/ `   ' ,\"\"+ \\  ",
    "   (__...'   __\\    |`.___.';",
    "     (_,...'(_,.`__)/'.....+",
    "  Welcome to Nekos Console! 🐱\n'help' to list commands",
    nullptr
    };
    for (int i = 0; banner[i] != nullptr; ++i) {
        Serial.println(banner[i]);
    }
    Serial.println();
    printPrompt();
}

int Console::getCommandCount() {
    return _commandCount;
}

void Console::printPrompt() {
    String ipStr = WiFi.localIP().toString();
    Serial.printf("[%s@%s] > ", "nekos", ipStr.c_str());
}

const char* Console::getCommandName(int index) {
    if (index < 0 || index >= _commandCount) return nullptr;
    return _commands[index].name;
}

bool Console::registerCommand(const char* name, CommandCallback cb) {
    if (!name || !cb) return false;
    if (_commandCount >= MAX_COMMANDS) return false;

    // Check duplicates
    for (int i = 0; i < _commandCount; i++) {
        if (strcmp(_commands[i].name, name) == 0) {
            return false; // duplicate
        }
    }

    strncpy(_commands[_commandCount].name, name, sizeof(_commands[_commandCount].name) - 1);
    _commands[_commandCount].name[sizeof(_commands[_commandCount].name) - 1] = '\0';
    _commands[_commandCount].cb = cb;
    _commandCount++;
    return true;
}

void Console::log(const char* message) {
    if (!message) return;
    Serial.println(message);
}

void Console::logf(const char* fmt, ...) {
    if (!fmt) return;
    char buf[QUEUE_MSG_SIZE];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    Serial.println(buf);
}

void Console::_dispatchLine(const char* line) {
    if (!line || line[0] == '\0') return;

    // Always start command output on a new line
    Serial.println(); 

    char buf[SHELL_INPUT_BUFFER_SIZE];
    strncpy(buf, line, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    char* saveptr = nullptr;
    char* cmd = strtok_r(buf, " ", &saveptr);
    char* args = strtok_r(nullptr, "", &saveptr);

    if (!cmd) return;

    for (int i = 0; i < _commandCount; ++i) {
        if (strcmp(cmd, _commands[i].name) == 0) {
            char args_copy[SHELL_INPUT_BUFFER_SIZE] = {0};
            if (args) {
                strncpy(args_copy, args, sizeof(args_copy) - 1);
            }
            _commands[i].cb(args_copy);
            printPrompt();
            return;
        }
    }

    logf("\n[ERROR] Unknown command: %s\nType 'help' for available commands.\n", cmd);
    printPrompt();
}

void Console::poll() {
    while (Serial.available() > 0) {
        char c = (char)Serial.read();
        if (c == '\r') continue;

        if (c == '\n') {
            if (_lineIndex > 0) {
                _lineBuf[_lineIndex] = '\0';
                _dispatchLine(_lineBuf);
                _lineIndex = 0;
                _lineBuf[0] = '\0';
            }
        } else if ((c == '\b' || c == 0x7F) && _lineIndex > 0) {
            _lineIndex--;
            _lineBuf[_lineIndex] = '\0';
            Serial.print("\b \b");
        } else if (_lineIndex < (SHELL_INPUT_BUFFER_SIZE - 1)) {
            _lineBuf[_lineIndex++] = c;
            Serial.write(c);
        } else {
            // Overflow, ignore char
        }
    }
}
