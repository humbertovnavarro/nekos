#include "NekosCoreUtils.h"
#include "NekosConsole.h"
#include "NekosFS.h"
#include "Arduino.h"
namespace nekos {
    void registerCoreUtils() {

        Console::registerCommand("help", [](const char* args){
            Console::log("Available commands:");
            int count = Console::getCommandCount();
            for (int i = 0; i < count; i++) {
                const char* cmdName = Console::getCommandName(i);
                if (cmdName) {
                    Console::logf("  %s", cmdName);
                }
            }
        });
        
        Console::registerCommand("heap", [](const char* args){
            size_t freeHeap = xPortGetFreeHeapSize();
            size_t minHeap = xPortGetMinimumEverFreeHeapSize();
            Console::logf("Free Heap: %u bytes", (unsigned)freeHeap);
            Console::logf("Min Ever Free Heap: %u bytes", (unsigned)minHeap);
        });

        Console::registerCommand("uptime", [](const char* args){
            uint64_t ms = esp_timer_get_time() / 1000;
            uint64_t seconds = ms / 1000;
            uint64_t minutes = seconds / 60;
            uint64_t hours = minutes / 60;
            Console::logf("Uptime: %llu hours, %llu minutes, %llu seconds",
                        hours, minutes % 60, seconds % 60);
        });

        Console::registerCommand("ls", [](const char* args) {
            const char* cwd = Console::getEnv("CWD");
            char path[256];
            if (!args || strlen(args) == 0) {
                if (cwd && cwd[0] != '\0') {
                    strncpy(path, cwd, sizeof(path));
                    path[sizeof(path)-1] = '\0';
                } else {
                    strncpy(path, "/", sizeof(path));
                }
            } else {
                nekos::fs::joinPath(cwd, args, path, sizeof(path));
            }
            nekos::fs::listDir(path);
        });

        Console::registerCommand("cat", [](const char* args) {
            if (!args || strlen(args) == 0) {
                Console::log("Usage: cat <file>");
                return;
            }
            const char* cwd = Console::getEnv("CWD");
            char path[256];
            nekos::fs::joinPath(cwd, args, path, sizeof(path));

            String content = nekos::fs::readFile(path);
            if (content.length() == 0) {
                Console::logf("[cat] Cannot read file: %s", path);
            } else {
                Serial.println(content);
            }
        });

        Console::registerCommand("rm", [](const char* args) {
            if (!args || strlen(args) == 0) {
                Console::log("Usage: rm <file>");
                return;
            }
            const char* cwd = Console::getEnv("CWD");
            char path[256];
            nekos::fs::joinPath(cwd, args, path, sizeof(path));

            if (nekos::fs::deleteFile(path)) {
                Console::logf("[rm] Deleted: %s", path);
            } else {
                Console::logf("[rm] Failed to delete: %s", path);
            }
        });

        Console::registerCommand("touch", [](const char* args) {
            if (!args || strlen(args) == 0) {
                Console::log("Usage: touch <file>");
                return;
            }
            const char* cwd = Console::getEnv("CWD");
            char path[256];
            nekos::fs::joinPath(cwd, args, path, sizeof(path));

            if (nekos::fs::touchFile(path)) {
                Console::logf("[touch] File created/updated: %s", path);
            } else {
                Console::logf("[touch] Failed: %s", path);
            }
        });

        Console::registerCommand("echo", [](const char* args) {
            if (!args || strlen(args) == 0) {
                Console::log("Usage: echo <text> > <file>");
                return;
            }

            const char* redirect = strstr(args, ">");
            if (!redirect) {
                Console::log("Usage: echo <text> > <file>");
                return;
            }

            String text = String(args, redirect - args);
            text.trim();
            String file = String(redirect + 1);
            file.trim();

            if (file.length() == 0) {
                Console::log("No output file specified.");
                return;
            }

            const char* cwd = Console::getEnv("CWD");
            char path[256];
            nekos::fs::joinPath(cwd, file.c_str(), path, sizeof(path));

            if (fs::writeFile(path, text + "\n")) {
                Console::logf("[echo] Wrote to %s", path);
            } else {
                Console::logf("[echo] Failed to write to %s", path);
            }
        });

        Console::registerCommand("mkdir", [](const char* args) {
            if (!args || strlen(args) == 0) {
                Console::log("Usage: mkdir <directory>");
                return;
            }
            const char* cwd = Console::getEnv("CWD");
            char path[256];
            
            nekos::fs::joinPath(cwd, args, path, sizeof(path));

            if (nekos::fs::makeDir(path)) {
                Console::logf("[mkdir] Created path: %s", path);
            } else {
                Console::logf("[mkdir] Failed to create path: %s", path);
            }
        });

        Console::registerCommand("cd", [](const char* args) {
            if (!args || strlen(args) == 0) {
                Console::log("Usage: cd <directory>");
                return;
            }
            const char* cwd = Console::getEnv("CWD");
            char newPath[256];
            nekos::fs::joinPath(cwd, args, newPath, sizeof(newPath));
            // Check if newPath is a directory (you may need to implement fs::isDir)
            if (nekos::fs::isDir(newPath)) {  // assuming listDir returns true if path exists and is directory
                if (Console::setEnv("CWD", newPath)) {
                    Console::logf("Changed directory to: %s", newPath);
                } else {
                    Console::log("Failed to update CWD environment variable");
                }
            } else {
                Console::logf("Directory not found: %s", newPath);
            }
        });
    }
}
