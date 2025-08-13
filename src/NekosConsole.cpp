#include <cstdarg>
#include <cstdio>
#include <cstring>
#include "NekosConsole.h"
#include "Arduino.h"

namespace nekos {
    // Define static members
    Console::EnvVar Console::_envVars[Console::MAX_ENV_VARS] = {};
    Console::Command Console::_commands[Console::MAX_COMMANDS] = {};
    int Console::_commandCount = 0;
    char Console::_lineBuf[Console::SHELL_INPUT_BUFFER_SIZE] = {};
    size_t Console::_lineIndex = 0;
    int Console::_logCallbackCount = 0;
    TaskHandle_t consoleLoopHandle;

    void consoleLoop(void* pvparams) {
        for(;;) {
            nekos::Console::poll();
        }
    };

    void Console::begin(unsigned long baud) {
        Serial.begin(baud);
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
        setEnv("CWD", "/");
        printPrompt();
        xTaskCreatePinnedToCore(consoleLoop, "consoleLoop", 8192, nullptr, 1, &consoleLoopHandle, 1);
    }

    int Console::getCommandCount() {
        return _commandCount;
    }

    void Console::printPrompt() {
        Serial.printf("[%s]=> ", "🐈");
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
        Console::logf("%s\n", message);
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
            vTaskDelay(1);
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
            }
        }
    }
    
    bool Console::setEnv(const char* name, const char* value) {
        if (!name || !value) return false;

        // Check name length
        if (strlen(name) >= MAX_ENV_NAME_LEN) return false;

        // Try to find existing var
        for (int i = 0; i < MAX_ENV_VARS; i++) {
            if (_envVars[i].inUse && strcmp(_envVars[i].name, name) == 0) {
                strncpy(_envVars[i].value, value, MAX_ENV_VALUE_LEN - 1);
                _envVars[i].value[MAX_ENV_VALUE_LEN - 1] = '\0';
                return true;
            }
        }

        // Find free slot
        for (int i = 0; i < MAX_ENV_VARS; i++) {
            if (!_envVars[i].inUse) {
                strncpy(_envVars[i].name, name, MAX_ENV_NAME_LEN - 1);
                _envVars[i].name[MAX_ENV_NAME_LEN - 1] = '\0';
                strncpy(_envVars[i].value, value, MAX_ENV_VALUE_LEN - 1);
                _envVars[i].value[MAX_ENV_VALUE_LEN - 1] = '\0';
                _envVars[i].inUse = true;
                return true;
            }
        }
        // No free slot
        return false;
    }

    const char* Console::getEnv(const char* name) {
        if (!name) return nullptr;
        for (int i = 0; i < MAX_ENV_VARS; i++) {
            if (_envVars[i].inUse && strcmp(_envVars[i].name, name) == 0) {
                return _envVars[i].value;
            }
        }
        return nullptr;
    }

    bool Console::unsetEnv(const char* name) {
        if (!name) return false;
        for (int i = 0; i < MAX_ENV_VARS; i++) {
            if (_envVars[i].inUse && strcmp(_envVars[i].name, name) == 0) {
                _envVars[i].inUse = false;
                _envVars[i].name[0] = '\0';
                _envVars[i].value[0] = '\0';
                return true;
            }
        }
        return false;
    }

}

