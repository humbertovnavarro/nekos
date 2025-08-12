#include <WiFi.h>
#include <HTTPClient.h>
#include "NekosNet.h"
namespace nekos {
    namespace net {
        // Connect to WiFi, returns true if connected successfully (within timeout)
        bool wifiConnect(const char* ssid, const char* password, unsigned long timeoutMs) {
            if (WiFi.status() == WL_CONNECTED) {
                if (strcmp(WiFi.SSID().c_str(), ssid) == 0) {
                    // Already connected to the requested SSID
                    return true;
                }
                WiFi.disconnect(true);
            }

            Serial.printf("[WiFi] Connecting to SSID: %s\n", ssid);
            WiFi.begin(ssid, password);

            unsigned long start = millis();
            while (WiFi.status() != WL_CONNECTED) {
                if (millis() - start >= timeoutMs) {
                    Serial.println("[WiFi] Connection timeout");
                    return false;
                }
                delay(100);
            }
            Serial.printf("[WiFi] Connected! IP: %s\n", WiFi.localIP().toString().c_str());
            return true;
        }

        // Disconnect WiFi
        void wifiDisconnect() {
            Serial.println("[WiFi] Disconnecting");
            WiFi.disconnect(true);
        }

        // Check if WiFi is connected
        bool wifiIsConnected() {
            return WiFi.status() == WL_CONNECTED;
        }

        // Scan for networks and print SSIDs + RSSI
        void wifiScanNetworks() {
            Serial.println("[WiFi] Scanning for networks...");
            int n = WiFi.scanNetworks();
            if (n == 0) {
                Serial.println("[WiFi] No networks found");
                return;
            }
            for (int i = 0; i < n; ++i) {
                Serial.printf("  %d: %s (RSSI: %d dBm) %s\n", i + 1,
                              WiFi.SSID(i).c_str(),
                              WiFi.RSSI(i),
                              WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "Open" : "Secured");
            }
        }

        // Get local IP as String (empty if not connected)
        String wifiGetLocalIP() {
            if (wifiIsConnected()) {
                return WiFi.localIP().toString();
            }
            return String();
        }
        
        String httpGet(const char* url) {
            if (WiFi.status() != WL_CONNECTED) {
                Serial.println("[HTTP] WiFi not connected");
                return "";
            }

            HTTPClient http;
            http.begin(url);

            int httpCode = http.GET();
            if (httpCode > 0) {
                if (httpCode == HTTP_CODE_OK) {
                    String payload = http.getString();
                    http.end();
                    return payload;
                } else {
                    Serial.printf("[HTTP] GET failed, code: %d\n", httpCode);
                }
            } else {
                Serial.printf("[HTTP] GET failed, error: %s\n", http.errorToString(httpCode).c_str());
            }
            http.end();
            return "";
        }

        bool httpPostJSON(const char* url, const String& payload) {
            if (WiFi.status() != WL_CONNECTED) {
                Serial.println("[HTTP] WiFi not connected");
                return false;
            }

            HTTPClient http;
            http.begin(url);
            http.addHeader("Content-Type", "application/json");  // assuming JSON, adjust as needed

            int httpCode = http.POST(payload);
            if (httpCode > 0) {
                if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_CREATED) {
                    http.end();
                    return true;
                } else {
                    Serial.printf("[HTTP] POST failed, code: %d\n", httpCode);
                }
            } else {
                Serial.printf("[HTTP] POST failed, error: %s\n", http.errorToString(httpCode).c_str());
            }
            http.end();
            return false;
        }
    }
}
