#include "Arduino.h"
namespace nekos {
    namespace net {
        // WiFi management
        void wifiDisconnect();
        bool wifiIsConnected();
        void wifiScanNetworks();
        String wifiGetLocalIP();
        bool wifiConnect(const char* ssid, const char* password, unsigned long timeoutMs = 10000);
        void wifiDisconnect();
        bool wifiIsConnected();
        void wifiScanNetworks();
        // HTTP Helpers
        String httpGet(const char* url);
        bool httpPostJSON(const char* url, const String& payload);
    }
}