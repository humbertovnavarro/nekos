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

void initializeDrivers() {
    psramInit();
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
    if (name == "mount") cmd_mount();
    else if (name == "unmount") cmd_unmount();
    else if (name == "help") {
        Serial.println("Commands: mount, unmount");
    }
    else if (name.startsWith("/")) LuaProcessScheduler::run(name.c_str());
    else Serial.printf("%s: command not found\n", name.c_str());
}

void setup() {
    initializeDrivers();
    prepareLuaEnvironment();
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
