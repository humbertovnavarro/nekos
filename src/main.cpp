#include "Arduino.h"
#include "FFat.h"
#include "drivers/input.hpp"
#include "drivers/neopixel.hpp"
#include "drivers/u8g2.hpp"
#include "drivers/usbmsc.hpp"
#include "sys/LuaCompiler.hpp"
#include "sys/LuaProcessScheduler.hpp"

bool usbMSCInitialized = false;
static String inputBuffer;
static String currentDir = "/";

String resolvePath(const String& path) {
    if (path.startsWith("/")) return path;
    if (currentDir.endsWith("/")) return currentDir + path;
    return currentDir + "/" + path;
}

void printPrompt() {
    Serial.printf("root:%s$ ", currentDir.c_str());
}

void cmd_top(const String&) {
    Serial.println(" PID     STATE       FILE");
    Serial.println("--------------------------------");
    for (int i = 0; i < MAX_LUA_PROCESSES; i++) {
        auto& proc = LuaProcessScheduler::luaProcesses[i];
        TaskHandle_t taskHandle = proc.taskHandle;
        lua_State* luaState = proc.L;
        if (!taskHandle || !luaState || !proc.luaCFilePath)
            continue;
        eTaskState tstate = eTaskGetState(taskHandle);
        const char* stateStr = "Unknown";
        switch (tstate) {
            case eRunning:   stateStr = "Running"; break;
            case eReady:     stateStr = "Ready"; break;
            case eBlocked:   stateStr = "Blocked"; break;
            case eSuspended: stateStr = "Suspended"; break;
            case eDeleted:   stateStr = "Deleted"; break;
            default: break;
        }
        Serial.printf(" %-6u  %-10s  %s\n",
            proc.pid,
            stateStr,
            proc.luaCFilePath ? proc.luaCFilePath : "(null)");
    }
    Serial.println("--------------------------------");
}

void cmd_mount() {
    if(!usbMSCInitialized) {
        setupUSBMSC();
        usbMSCInitialized = true;
    }
    FFat.end();
    usbMsc.mediaPresent(true);
}

void cmd_unmount() {
    if(!usbMSCInitialized) return;
    usbMsc.mediaPresent(false);
    FFat.begin();
}

void cmd_pkill(const String& arg) {
    if (arg.isEmpty()) {
        Serial.println("Usage: pkill <pid>");
        return;
    }
    uint32_t pid = arg.toInt();
    if (pid == 0 || pid > MAX_LUA_PROCESSES) {
        Serial.println("pkill: invalid pid");
        return;
    }
    lua_close(LuaProcessScheduler::luaProcesses[pid].L);
}

void cmd_luac(const String& arg) {
    if (arg.isEmpty()) {
        Serial.println("Usage: luac <file.lua> [output.luac]");
        return;
    }
    int space = arg.indexOf(' ');
    String srcPath = space == -1 ? arg : arg.substring(0, space);
    String dstPath = space == -1 ? ""  : arg.substring(space + 1);

    srcPath = resolvePath(srcPath);
    if (dstPath.isEmpty()) {
        // default output: same path but with .luac
        if (srcPath.endsWith(".lua"))
            dstPath = srcPath.substring(0, srcPath.length() - 4) + ".luac";
        else
            dstPath = srcPath + ".luac";
    } else {
        dstPath = resolvePath(dstPath);
    }

    Serial.printf("[LuaC] Compiling '%s' -> '%s'\n", srcPath.c_str(), dstPath.c_str());

    if (!FFat.exists(srcPath)) {
        Serial.printf("luac: source file not found: %s\n", srcPath.c_str());
        return;
    }
    bool success = compileLuaFileIfNeeded(srcPath.c_str());
    if (success)
        Serial.printf("[LuaC] Success! Wrote %s\n", dstPath.c_str());
    else
        Serial.printf("[LuaC] Failed to compile %s\n", srcPath.c_str());
}

void cmd_ls(const String& arg) {
    String path = arg.isEmpty() ? currentDir : resolvePath(arg);
    File dir = FFat.open(path);
    if (!dir || !dir.isDirectory()) {
        Serial.printf("ls: cannot access '%s': No such directory\n", path.c_str());
        return;
    }
    File f = dir.openNextFile();
    while (f) {
        Serial.printf("%-20s %8u\n", f.name(), f.size());
        f = dir.openNextFile();
    }
}

void cmd_cat(const String& arg) {
    String path = resolvePath(arg);
    File f = FFat.open(path);
    if (!f) {
        Serial.printf("cat: %s: No such file\n", path.c_str());
        return;
    }
    while (f.available()) Serial.write(f.read());
    Serial.println();
    f.close();
}

void cmd_rm(const String& arg) {
    String path = resolvePath(arg);
    if (FFat.remove(path))
        Serial.printf("removed '%s'\n", path.c_str());
    else
        Serial.printf("rm: cannot remove '%s'\n", path.c_str());
}

void cmd_cd(const String& arg) {
    String path = resolvePath(arg);
    File dir = FFat.open(path);
    if (dir && dir.isDirectory()) {
        currentDir = path;
    } else {
        Serial.printf("cd: %s: No such directory\n", path.c_str());
    }
}

void cmd_echo(const String& arg) {
    // echo 'text' > file.lua
    int redir = arg.indexOf('>');
    if (redir == -1) {
        Serial.println(arg);
        return;
    }
    String content = arg.substring(0, redir);
    content.trim();
    String dest = arg.substring(redir + 1);
    dest.trim();
    if (dest.isEmpty()) {
        Serial.println("echo: missing file name");
        return;
    }
    String path = resolvePath(dest);
    File f = FFat.open(path, "w");
    if (!f) {
        Serial.printf("echo: cannot write to '%s'\n", path.c_str());
        return;
    }
    f.println(content);
    f.close();
    Serial.printf("Wrote to %s\n", path.c_str());
}



void initializeDrivers() {
    Serial.begin();
    while (!Serial) delay(10);
    FFat.begin(true);
    u8g2.begin();
    neopixel.begin();
    inputBegin();
    // setupUSBMSC();
    Serial.println("\n[System] Drivers initialized.");
}

void prepareLuaEnvironment() {
    Serial.println("[System] Compiling lua source.");
    enumerateAndCompileLuaFiles("/");
    LuaProcessScheduler::begin();
    Serial.println("[System] Lua environment ready.");
}

void handleCommand(String cmd) {
    cmd.trim();
    if (cmd.isEmpty()) return;
    int space = cmd.indexOf(' ');
    String name = space == -1 ? cmd : cmd.substring(0, space);
    String arg  = space == -1 ? ""  : cmd.substring(space + 1);
    if (name == "ls") cmd_ls(arg);
    else if (name == "cat") cmd_cat(arg);
    else if (name == "rm") cmd_rm(arg);
    else if (name == "cd") cmd_cd(arg);
    else if (name == "echo") cmd_echo(arg);
    else if (name == "pwd") Serial.println(currentDir);
    else if (name == "reboot") ESP.restart();
    else if (name == "top") cmd_top(arg);
    else if (name == "pkill") cmd_pkill(arg);
    else if (name == "mount") cmd_mount();
    else if (name == "unmount") cmd_unmount();
    else if (name == "help") {
        Serial.println("Commands: ls, cd, cat, rm, echo, pwd, reboot, top, pkill, mount, unmount");
    }
    else if (name.startsWith("/")) LuaProcessScheduler::run(name.c_str());
    else Serial.printf("%s: command not found\n", name.c_str());
}

void setup() {
    initializeDrivers();
    prepareLuaEnvironment();
    LuaProcessScheduler::run("/boot.luac", {
        .stackSize = 16384,
        .priority = 1,
        .affinity = 0
    });
    Serial.println("[LuaExec] Boot script launched.");
}

void loop() {
    static bool needPrompt = true;
    static char c;

    // Print prompt if needed
    if (needPrompt) {
        Serial.printf("root:%s$ ", currentDir.c_str());
        needPrompt = false;
    }

    // Read serial input
    while (Serial.available()) {
        c = Serial.read();

        // Normalize line endings: ignore '\r', handle '\n'
        if (c == '\r') continue;

        if (c == '\n') {
            Serial.println();  // move to next line
            handleCommand(inputBuffer);
            inputBuffer = "";
            needPrompt = true;
        }
        // Handle backspace / delete
        else if (c == 8 || c == 127) {
            if (!inputBuffer.isEmpty()) {
                inputBuffer.remove(inputBuffer.length() - 1);
                Serial.print("\b \b");  // erase last char
            }
        }
        // Printable characters
        else if (isPrintable(c)) {
            inputBuffer += c;
            Serial.write(c);
        }
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
}
