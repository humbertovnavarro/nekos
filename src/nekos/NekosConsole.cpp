#include "NekosConsole.h"
#include <cstring>
#include "LuaScripts.h"
#include "NekosLuaLoader.h"
namespace nekos {

// ------------------- Static members -------------------
char Console::_lineBuf[SHELL_INPUT_BUFFER_SIZE] = {};
size_t Console::_lineLen = 0;
size_t Console::_cursorPos = 0;
std::vector<String> Console::_history;
int Console::_histIndex = -1;

// ------------------- Helpers -------------------
void Console::printPrompt() {
    Serial.print("🐈=> ");
}

void Console::printRepeat(char c, size_t count) {
    for (size_t i = 0; i < count; i++) Serial.print(c);
}

void Console::dispatchCommand(const char* line) {
    if (!line || line[0] == '\0') return;

    char cmd[SHELL_INPUT_BUFFER_SIZE] = {};
    char args[SHELL_INPUT_BUFFER_SIZE] = {};

    // Split command from arguments
    sscanf(line, "%s %[^\n]", cmd, args);

    String commandName(cmd);

    // Execute via NekosLuaLoader static methods (handles caching and PSRAM)
    if (nekos::NekosLuaLoader::exists(commandName)) {
        nekos::NekosLuaLoader::exec(commandName, args[0] ? args : nullptr);
    } else {
        Serial.print("Command '");
        Serial.print(cmd);
        Serial.println("' not found");
    }
}

String Console::completeCommand(const String& prefix) {
    std::vector<String> matches;
    for (auto &p : *luaScriptMap) {
        if (p.first.startsWith(prefix)) matches.push_back(p.first);
    }

    if (matches.empty()) return prefix;
    if (matches.size() == 1) return matches[0];

    // Multiple matches: print options
    Serial.println();
    for (auto &m : matches) { Serial.print(m); Serial.print("  "); }
    Serial.println();
    printPrompt();
    Serial.print(prefix);
    return prefix;
}

// ------------------- Public -------------------
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
    for (int i = 0; banner[i]; i++) Serial.println(banner[i]);
    Serial.println();
    printPrompt();
}

void Console::poll() {
    static bool escSeq = false;
    static int escIndex = 0;
    static char escBuf[2];

    while (Serial.available() > 0) {
        char c = Serial.read();

        // --------- Enter key ----------
        if (c == '\r' || c == '\n') {
            Serial.println();
            if (_lineLen > 0) {
                _lineBuf[_lineLen] = '\0';
                dispatchCommand(_lineBuf);

                // Save history
                _history.insert(_history.begin(), String(_lineBuf));
                if (_history.size() > HISTORY_SIZE) _history.pop_back();
                _histIndex = -1;
            }

            _lineLen = _cursorPos = 0;
            _lineBuf[0] = '\0';
            printPrompt();
            continue;
        }

        // --------- Escape sequence (arrow keys) ----------
        if (escSeq) {
            escBuf[escIndex++] = c;
            if (escIndex == 2) {  // expect [ + <letter>
                if (escBuf[0] == '[') {
                    switch (escBuf[1]) {
                        case 'A': // UP
                            if (!_history.empty() && _histIndex + 1 < (int)_history.size()) {
                                _histIndex++;
                                String h = _history[_histIndex];
                                strncpy(_lineBuf, h.c_str(), SHELL_INPUT_BUFFER_SIZE);
                                _lineLen = _cursorPos = h.length();
                                Serial.print("\r\033[K");
                                printPrompt();
                                Serial.write(_lineBuf, _lineLen);
                            }
                            break;
                        case 'B': // DOWN
                            if (_histIndex > 0) _histIndex--;
                            else _histIndex = -1;

                            {
                                String h = (_histIndex >= 0) ? _history[_histIndex] : "";
                                strncpy(_lineBuf, h.c_str(), SHELL_INPUT_BUFFER_SIZE);
                                _lineLen = _cursorPos = h.length();
                                Serial.print("\r\033[K");
                                printPrompt();
                                Serial.write(_lineBuf, _lineLen);
                            }
                            break;
                        case 'C': // RIGHT
                            if (_cursorPos < _lineLen) {
                                Serial.write(_lineBuf[_cursorPos]);
                                _cursorPos++;
                            }
                            break;
                        case 'D': // LEFT
                            if (_cursorPos > 0) {
                                Serial.print('\b');
                                _cursorPos--;
                            }
                            break;
                    }
                }
                escSeq = false;
                escIndex = 0;
            }
            continue;
        }

        if (c == 27) {  // ESC start
            escSeq = true;
            escIndex = 0;
            continue;
        }

        // --------- Backspace ----------
        if ((c == '\b' || c == 127) && _cursorPos > 0) {
            memmove(_lineBuf + _cursorPos - 1, _lineBuf + _cursorPos, _lineLen - _cursorPos);
            _cursorPos--; _lineLen--;
            _lineBuf[_lineLen] = '\0';

            Serial.print("\r\033[K");
            printPrompt();
            Serial.write(_lineBuf, _lineLen);
            if (_cursorPos < _lineLen) printRepeat('\b', _lineLen - _cursorPos);
            continue;
        }

        // --------- Tab completion ----------
        if (c == '\t') {
            String prefix(_lineBuf, _cursorPos);
            String completion = completeCommand(prefix);
            size_t n = completion.length() - _cursorPos;

            memmove(_lineBuf + _cursorPos + n, _lineBuf + _cursorPos, _lineLen - _cursorPos);
            memcpy(_lineBuf + _cursorPos, completion.c_str() + _cursorPos, n);
            _lineLen += n; _cursorPos += n;

            Serial.print("\r\033[K");
            printPrompt();
            Serial.write(_lineBuf, _lineLen);
            printRepeat('\b', _lineLen - _cursorPos);
            continue;
        }

        // --------- Insert character ----------
        if (_lineLen < SHELL_INPUT_BUFFER_SIZE - 1) {
            memmove(_lineBuf + _cursorPos + 1, _lineBuf + _cursorPos, _lineLen - _cursorPos);
            _lineBuf[_cursorPos] = c;
            _lineLen++; _cursorPos++;

            Serial.print("\r\033[K");
            printPrompt();
            Serial.write(_lineBuf, _lineLen);
            if (_cursorPos < _lineLen) printRepeat('\b', _lineLen - _cursorPos);
        }
    }
}



} // namespace nekos
