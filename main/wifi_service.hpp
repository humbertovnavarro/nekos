#pragma once

#include "esp_err.h"
#include "esp_wifi_types_generic.h"
#include "etl/vector.h"
#include "message_bus.hpp"

using WifiMessageBus = MessageBus<wifi_event_t, 16, 3>;

namespace wifi {
extern WifiMessageBus message_bus;

// Cap on how many scan results get_scan_results() will copy out.
constexpr uint16_t kMaxScanResults = 20;

// Brings up NVS, the default netif/event loop, and the esp_wifi driver
// itself. Call once before anything else in this file. From here on,
// every WIFI_EVENT_* (connect, disconnect, scan done, AP client join,
// etc.) is forwarded onto `message_bus` as it happens -- subscribe to
// react to them. IP_EVENT (e.g. "got IP") is a different event type than
// wifi_event_t and isn't carried by this bus; hook esp_event yourself for
// that if you need it.
void init();

// Configures the station interface and starts connecting. `ssid` must be
// non-null, 1-31 characters. `password` may be null/empty for an open
// network; otherwise WPA2-PSK is assumed and must be 8-63 characters.
// Can be called alongside host_ap() -- the driver is switched to APSTA
// mode automatically rather than dropping the other role.
// Connect success/failure arrives asynchronously on message_bus as
// WIFI_EVENT_STA_CONNECTED / WIFI_EVENT_STA_DISCONNECTED.
esp_err_t connect_sta(const char* ssid, const char* password);

// Starts an active scan across all channels. Non-blocking: completion is
// announced on message_bus as WIFI_EVENT_SCAN_DONE, after which you'd call
// get_scan_results().
esp_err_t scan(bool show_hidden = false);

// Fetches results from the most recently completed scan (up to
// kMaxScanResults of them) into out_results, replacing its contents.
esp_err_t get_scan_results(etl::vector<wifi_ap_record_t, kMaxScanResults>& out_results);

// Sets the radio's channel directly. Only meaningful in AP-only mode (or
// for sniffing) -- a station already associated to an AP has its channel
// dictated by that AP, and this call will fail in that case.
esp_err_t set_channel(uint8_t primary_channel, wifi_second_chan_t second_channel = WIFI_SECOND_CHAN_NONE);

// Configures and starts a SoftAP. `ssid` must be non-null, 1-32 characters
// (SoftAP SSIDs don't need to be null-terminated, unlike STA's). `password`
// may be null/empty for an open network; otherwise WPA2-PSK is assumed and
// must be 8-63 characters. Can be called alongside connect_sta() (APSTA).
esp_err_t host_ap(const char* ssid, const char* password, uint8_t channel = 1, uint8_t max_connections = 4);

}  // namespace wifi