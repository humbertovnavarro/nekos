#ifdef NEKOS_HAS_WIFI
#include "NekosConsole.h"
#include "NekosNet.h"
namespace nekos {
    void registerNetCommands() {
        Console::commands.registerCommand("wifi_connect", [](Command* cmd, const char* args) {
            const char* ssid = cmd->args.get("ssid");
            const char* password = cmd->args.get("password");
            if (net::wifiConnect(ssid, password)) {
                Console::logf("WiFi connected: %s\n", ssid);
                Console::logf("IP: %s\n", net::wifiGetLocalIP().c_str());
            } else {
                Console::log("Failed to connect to WiFi.\n");
            }
        })->args.addArgument("ssid", true)
          ->addArgument("password", true);

        // Disconnect WiFi
        Console::commands.registerCommand("wifi_disconnect", [](Command* cmd, const char* args) {
            net::wifiDisconnect();
            Console::log("WiFi disconnected.\n");
        });

        // WiFi status
        Console::commands.registerCommand("wifi_status", [](Command* cmd, const char* args) {
            if (net::wifiIsConnected()) {
                Console::logf("Connected. IP: %s\n", net::wifiGetLocalIP().c_str());
            } else {
                Console::log("Not connected to WiFi.\n");
            }
        });

        // Scan available networks
        Console::commands.registerCommand("wifi_scan", [](Command* cmd, const char* args) {
            net::wifiScanNetworks();
        });

        // HTTP GET: http_get <url>
        Console::commands.registerCommand("http_get", [](Command* cmd, const char* args) {
            const char* url = cmd->args.get("url");
            if (!url || strlen(url) == 0) {
                Console::log("Usage: http_get <URL>\n");
                return;
            }
            String resp = net::httpGet(url);
            Console::log("HTTP GET Response:\n");
            Console::log(resp.c_str());
        })->args.addArgument("url", true);

        // HTTP POST JSON: http_post <url> <json>
        Console::commands.registerCommand("http_post", [](Command* cmd, const char* args) {
            const char* url = cmd->args.get("url");
            const char* json = cmd->args.get("json");
            if (!url || !json || strlen(url) == 0 || strlen(json) == 0) {
                Console::log("Usage: http_post <URL> <JSON_BODY>\n");
                return;
            }
            if (net::httpPostJSON(url, String(json))) {
                Console::log("POST successful.\n");
            } else {
                Console::log("POST failed.\n");
            }
        })->args.addArgument("url", true)
          ->addArgument("json", true);
    }
}
#endif