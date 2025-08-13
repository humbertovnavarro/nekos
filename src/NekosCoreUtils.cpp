#include "NekosCoreUtils.h"
#include "NekosConsole.h"
#include "NekosFS.h"
#include "NekosNet.h"
#include "Arduino.h"
#include "NekosArgParse.h"

namespace nekos {
    void registerCoreUtils() {

        Console::registerCommand("help", [](const char* args) {
            Console::log("Available commands:");
            int count = Console::getCommandCount();
            for (int i = 0; i < count; i++) {
                const char* cmdName = Console::getCommandName(i);
                if (cmdName) {
                    Console::logf("  %s", cmdName);
                }
            }
        });

        Console::registerCommand("heap", [](const char*) {
            size_t freeHeap = xPortGetFreeHeapSize();
            size_t minHeap = xPortGetMinimumEverFreeHeapSize();
            Console::logf("Free Heap: %u bytes", (unsigned)freeHeap);
            Console::logf("Min Ever Free Heap: %u bytes", (unsigned)minHeap);
        });

        Console::registerCommand("uptime", [](const char*) {
            uint64_t ms = esp_timer_get_time() / 1000;
            uint64_t seconds = ms / 1000;
            uint64_t minutes = seconds / 60;
            uint64_t hours = minutes / 60;
            Console::logf("Uptime: %llu hours, %llu minutes, %llu seconds",
                          hours, minutes % 60, seconds % 60);
        });

        Console::registerCommand("cat", [](const char* args) {
            NekosArgParse parser;
            parser.addArgument("file", true, "", "File to read");
            if (!parser.parse(args)) {
                Console::log(parser.usage("cat").c_str());
                return;
            }
            const char* cwd = Console::getEnv("CWD");
            char path[256];
            nekos::fs::joinPath(cwd, parser.get("file"), path, sizeof(path));

            String content = nekos::fs::readFile(path);
            if (content.length() == 0) {
                Console::logf("[cat] Cannot read file: %s", path);
            } else {
                Serial.println(content);
            }
        });

        Console::registerCommand("rm", [](const char* args) {
            NekosArgParse parser;
            parser.addArgument("file", true, "", "File to delete");
            if (!parser.parse(args)) {
                Console::log(parser.usage("rm").c_str());
                return;
            }
            const char* cwd = Console::getEnv("CWD");
            char path[256];
            nekos::fs::joinPath(cwd, parser.get("file"), path, sizeof(path));

            if (nekos::fs::deleteFile(path)) {
                Console::logf("[rm] Deleted: %s", path);
            } else {
                Console::logf("[rm] Failed to delete: %s", path);
            }
        });

        Console::registerCommand("touch", [](const char* args) {
            NekosArgParse parser;
            parser.addArgument("file", true, "", "File to create/update");
            if (!parser.parse(args)) {
                Console::log(parser.usage("touch").c_str());
                return;
            }
            const char* cwd = Console::getEnv("CWD");
            char path[256];
            nekos::fs::joinPath(cwd, parser.get("file"), path, sizeof(path));

            if (nekos::fs::touchFile(path)) {
                Console::logf("[touch] File created/updated: %s", path);
            } else {
                Console::logf("[touch] Failed: %s", path);
            }
        });

        Console::registerCommand("echo", [](const char* args) {
            NekosArgParse parser;
            parser.addArgument("text", true, "", "Text to write");
            parser.addArgument("file", true, "", "Output file");
            if (!parser.parse(args)) {
                Console::log("Usage: echo <text> > <file>");
                return;
            }

            const char* cwd = Console::getEnv("CWD");
            char path[256];
            nekos::fs::joinPath(cwd, parser.get("file"), path, sizeof(path));

            if (fs::writeFile(path, String(parser.get("text")) + "\n")) {
                Console::logf("[echo] Wrote to %s", path);
            } else {
                Console::logf("[echo] Failed to write to %s", path);
            }
        });

        Console::registerCommand("mkdir", [](const char* args) {
            NekosArgParse parser;
            parser.addArgument("dir", true, "", "Directory to create");
            if (!parser.parse(args)) {
                Console::log(parser.usage("mkdir").c_str());
                return;
            }
            const char* cwd = Console::getEnv("CWD");
            char path[256];
            nekos::fs::joinPath(cwd, parser.get("dir"), path, sizeof(path));

            if (nekos::fs::makeDir(path)) {
                Console::logf("[mkdir] Created path: %s", path);
            } else {
                Console::logf("[mkdir] Failed to create path: %s", path);
            }
        });

        Console::registerCommand("cd", [](const char* args) {
            NekosArgParse parser;
            parser.addArgument("dir", true, "", "Directory to change into");
            if (!parser.parse(args)) {
                Console::log(parser.usage("cd").c_str());
                return;
            }
            const char* cwd = Console::getEnv("CWD");
            char newPath[256];
            nekos::fs::joinPath(cwd, parser.get("dir"), newPath, sizeof(newPath));

            if (nekos::fs::isDir(newPath)) {
                if (Console::setEnv("CWD", newPath)) {
                    Console::logf("Changed directory to: %s", newPath);
                } else {
                    Console::log("Failed to update CWD environment variable");
                }
            } else {
                Console::logf("Directory not found: %s", newPath);
            }
        });

        Console::registerCommand("wifi_connect", [](const char* args) {
            NekosArgParse parser;
            parser.addArgument("ssid", true, "", "WiFi SSID");
            parser.addArgument("password", true, "", "WiFi password");
            parser.addArgument("hidden", false, "", "Set if network is hidden", true);
            if (!parser.parse(args)) {
                Console::log(parser.usage("wifi_connect").c_str());
                return;
            }
            Console::logf("SSID: '%s'", parser.get("ssid"));
            Console::logf("Password: '%s'", parser.get("password"));
            Console::logf("Hidden: %d", parser.getFlag("hidden"));
            nekos::net::wifiConnect(parser.get("ssid"), parser.get("password"));
        });

        Console::registerCommand("wifi_disconnect", [](const char*) {
            nekos::net::wifiDisconnect();
            Console::log("WiFi disconnected.");
        });

        Console::registerCommand("wifi_status", [](const char*) {
            if (nekos::net::wifiIsConnected()) {
                Console::logf("Connected. IP: %s", nekos::net::wifiGetLocalIP().c_str());
            } else {
                Console::log("Not connected to WiFi.");
            }
        });

        Console::registerCommand("wifi_scan", [](const char*) {
            nekos::net::wifiScanNetworks();
        });

        Console::registerCommand("http_get", [](const char* args) {
            NekosArgParse parser;
            parser.addArgument("url", true, "", "Target URL");
            if (!parser.parse(args)) {
                Console::log(parser.usage("http_get").c_str());
                return;
            }
            String resp = nekos::net::httpGet(parser.get("url"));
            Console::log("HTTP GET Response:");
            Console::log(resp.c_str());
        });

        Console::registerCommand("http_post", [](const char* args) {
            NekosArgParse parser;
            parser.addArgument("url", true, "", "Target URL");
            parser.addArgument("json", true, "", "JSON body");
            if (!parser.parse(args)) {
                Console::log(parser.usage("http_post").c_str());
                return;
            }
            if (nekos::net::httpPostJSON(parser.get("url"), parser.get("json"))) {
                Console::log("POST successful.");
            } else {
                Console::log("POST failed.");
            }
        });
    }
}
