#include "NekoShell.h"
#include "NekosNet.h"
#include "NekosAppRegistry.h"
namespace nekos {
    void registerNetApps() {
        AppRegistry::registerApp("wifi_connect", [](App* app) {
            const char* ssid = app->args.get("ssid");
            const char* password = app->args.get("password");
            if (net::wifiConnect(ssid, password)) {
                Console::logf("WiFi connected: %s\n", ssid);
                Console::logf("IP: %s\n", net::wifiGetLocalIP().c_str());
            } else {
                Console::log("Failed to connect to WiFi.\n");
            }
        })->args.addArgument("ssid", true)
          ->addArgument("password", true);

        // Disconnect WiFi
        AppRegistry::registerApp("wifi_disconnect", [](App* app) {
            net::wifiDisconnect();
            Console::log("WiFi disconnected.\n");
        });

        // WiFi status
        AppRegistry::registerApp("wifi_status", [](App* app) {
            if (net::wifiIsConnected()) {
                Console::logf("Connected. IP: %s\n", net::wifiGetLocalIP().c_str());
            } else {
                Console::log("Not connected to WiFi.\n");
            }
        });

        // Scan available networks
        AppRegistry::registerApp("wifi_scan", [](App* app) {
            net::wifiScanNetworks();
        });

        // HTTP GET: http_get <url>
        AppRegistry::registerApp("http_get", [](App* app) {
            const char* url = app->args.get("url");
            if (!url || strlen(url) == 0) {
                Console::log("Usage: http_get <URL>\n");
                return;
            }
            String resp = net::httpGet(url);
            Console::log("HTTP GET Response:\n");
            Console::log(resp.c_str());
        })->args.addArgument("url", true);

        // HTTP POST JSON: http_post <url> <json>
        AppRegistry::registerApp("http_post", [](App* app) {
            const char* url = app->args.get("url");
            const char* json = app->args.get("json");
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