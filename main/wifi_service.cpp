#include "wifi_service.hpp"

#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "nvs_flash.h"

#include <cstring>

namespace wifi {

WifiMessageBus message_bus = WifiMessageBus();

namespace {

bool g_started = false;

void wifi_event_handler(void*, esp_event_base_t event_base, int32_t event_id, void* /*event_data*/) {
    if (event_base != WIFI_EVENT) {
        return;
    }
    message_bus.post(static_cast<wifi_event_t>(event_id));
}

esp_err_t ensure_mode(wifi_mode_t required) {
    wifi_mode_t current = WIFI_MODE_NULL;
    esp_wifi_get_mode(&current);

    wifi_mode_t desired = required;
    if (required == WIFI_MODE_STA && (current == WIFI_MODE_AP || current == WIFI_MODE_APSTA)) {
        desired = WIFI_MODE_APSTA;
    } else if (required == WIFI_MODE_AP && (current == WIFI_MODE_STA || current == WIFI_MODE_APSTA)) {
        desired = WIFI_MODE_APSTA;
    }

    if (desired == current) {
        return ESP_OK;
    }
    return esp_wifi_set_mode(desired);
}

esp_err_t ensure_started() {
    if (g_started) {
        return ESP_OK;
    }
    esp_err_t err = esp_wifi_start();
    if (err == ESP_OK) {
        g_started = true;
    }
    return err;
}

}  // namespace

void init() {
    esp_netif_create_default_wifi_sta();
    esp_netif_create_default_wifi_ap();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, nullptr, nullptr));
}

esp_err_t connect_sta(const char* ssid, const char* password) {
    if (ssid == nullptr) {
        return ESP_ERR_INVALID_ARG;
    }

    wifi_config_t wifi_config = {};
    size_t ssid_len = strlen(ssid);
    size_t password_len = password ? strlen(password) : 0;
    if (ssid_len == 0 || ssid_len >= sizeof(wifi_config.sta.ssid)) {
        return ESP_ERR_INVALID_ARG;
    }
    if (password_len > 0 && password_len < 8) {
        return ESP_ERR_INVALID_ARG;  // WPA2-PSK needs 8-63 chars
    }

    strlcpy(reinterpret_cast<char*>(wifi_config.sta.ssid), ssid, sizeof(wifi_config.sta.ssid));
    if (password_len > 0) {
        strlcpy(reinterpret_cast<char*>(wifi_config.sta.password), password, sizeof(wifi_config.sta.password));
        wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    } else {
        wifi_config.sta.threshold.authmode = WIFI_AUTH_OPEN;
    }

    esp_err_t err = ensure_mode(WIFI_MODE_STA);
    if (err != ESP_OK) return err;

    err = esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    if (err != ESP_OK) return err;

    err = ensure_started();
    if (err != ESP_OK) return err;

    return esp_wifi_connect();
}

esp_err_t scan(bool show_hidden) {
    esp_err_t err = ensure_mode(WIFI_MODE_STA);
    if (err != ESP_OK) return err;

    err = ensure_started();
    if (err != ESP_OK) return err;

    wifi_scan_config_t scan_config = {};
    scan_config.show_hidden = show_hidden;
    scan_config.scan_type = WIFI_SCAN_TYPE_ACTIVE;

    return esp_wifi_scan_start(&scan_config, false);
}

esp_err_t get_scan_results(etl::vector<wifi_ap_record_t, kMaxScanResults>& out_results) {
    out_results.clear();

    uint16_t ap_count = 0;
    esp_err_t err = esp_wifi_scan_get_ap_num(&ap_count);
    if (err != ESP_OK) return err;

    uint16_t to_fetch = ap_count < kMaxScanResults ? ap_count : kMaxScanResults;
    if (to_fetch == 0) {
        return ESP_OK;
    }

    wifi_ap_record_t records[kMaxScanResults];
    err = esp_wifi_scan_get_ap_records(&to_fetch, records);
    if (err != ESP_OK) return err;

    for (uint16_t i = 0; i < to_fetch; i++) {
        out_results.push_back(records[i]);
    }
    return ESP_OK;
}

esp_err_t set_channel(uint8_t primary_channel, wifi_second_chan_t second_channel) {
    return esp_wifi_set_channel(primary_channel, second_channel);
}

esp_err_t host_ap(const char* ssid, const char* password, uint8_t channel, uint8_t max_connections) {
    if (ssid == nullptr) {
        return ESP_ERR_INVALID_ARG;
    }
    wifi_config_t wifi_config = {};
    size_t ssid_len = strlen(ssid);
    size_t password_len = password ? strlen(password) : 0;
    if (ssid_len == 0 || ssid_len > sizeof(wifi_config.ap.ssid)) {
        return ESP_ERR_INVALID_ARG;
    }
    if (password_len > 0 && password_len < 8) {
        return ESP_ERR_INVALID_ARG;  // WPA2-PSK needs 8-63 chars
    }
    memcpy(wifi_config.ap.ssid, ssid, ssid_len);
    wifi_config.ap.ssid_len = ssid_len;
    wifi_config.ap.channel = channel;
    wifi_config.ap.max_connection = max_connections;
    if (password_len > 0) {
        strlcpy(reinterpret_cast<char*>(wifi_config.ap.password), password, sizeof(wifi_config.ap.password));
        wifi_config.ap.authmode = WIFI_AUTH_WPA2_PSK;
    } else {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }
    esp_err_t err = ensure_mode(WIFI_MODE_AP);
    if (err != ESP_OK) return err;
    err = esp_wifi_set_config(WIFI_IF_AP, &wifi_config);
    if (err != ESP_OK) return err;
    return ensure_started();
}

}  // namespace wifi