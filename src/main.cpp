#include "Arduino.h"
#include "nekos/Nekos.h"
#include "nekos/NekosLuaScheduler.h"
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

struct ShellState {
    char line[128];
    size_t len;
    lua_State* L;
} shell;

// ---------- Command Execution ----------
void executeCommand(const char* line) {
    Serial.print("\r\n");  // start output on new line

    String str(line);
    int space = str.indexOf(' ');
    String cmd = (space >= 0) ? str.substring(0, space) : str;
    String args = (space >= 0) ? str.substring(space + 1) : "";

    if (nekos::LuaScheduler::exists(cmd)) {
        nekos::LuaScheduler::exec(cmd, args.c_str());
    } else {
        nekos::luaRunReplLine(shell.L, line);
    }

    Serial.print("\r\n");  // ensure next command starts on new line
    shell.len = 0;
}

// ---------- Character Handling ----------
void handleChar(char c) {
    if (c == '\r' || c == '\n') {
        shell.line[shell.len] = 0;
        if (shell.len > 0) {
            executeCommand(shell.line);
        } else {
            shell.len = 0;
        }
    } else {
        if (shell.len < sizeof(shell.line) - 1) {
            shell.line[shell.len++] = c;
            Serial.write(c);
        }
    }
}

// ---------- Poll Input ----------
void pollShell() {
    while (Serial.available()) {
        char c = Serial.read();
        handleChar(c);
    }
}

// ---------- Setup ----------
void setup() {
    Serial.begin(115200);
    while (!Serial);
    nekos::LuaScheduler::begin(16);
    nekos::LuaScheduler::exec("banner");
    shell.L = luaL_newstate();
    nekos::luaRegisterBindings(shell.L);
    shell.len = 0;
}

// ---------- Main Loop ----------
void loop() {
    pollShell();
    vTaskDelay(1);
}
