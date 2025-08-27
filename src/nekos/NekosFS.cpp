#include <Arduino.h>
#include "USB.h"
#include "USBMSC.h"
#include "semphr.h"
#include "esp_partition.h"
#include "wear_levelling.h"

namespace nekos {
namespace fs {

USBMSC MSC;
SemaphoreHandle_t ioMutex = nullptr;
wl_handle_t wlHandle = WL_INVALID_HANDLE;

constexpr uint16_t DISK_SECTOR_SIZE  = 512;
constexpr uint32_t DISK_SECTOR_COUNT = 2000;  // ~1MB
constexpr uint32_t DISK_SIZE_BYTES   = DISK_SECTOR_SIZE * DISK_SECTOR_COUNT;

uint32_t lastMSCActivity = 0;
constexpr uint32_t MSC_IDLE_TIMEOUT_MS = 2000; // 2 seconds

// ------------------- MSC callbacks -------------------
static int32_t onWrite(uint32_t lba, uint32_t lba_off, uint8_t *buf, uint32_t sz)
{
    if (wlHandle == WL_INVALID_HANDLE) return 0;
    if (xSemaphoreTake(ioMutex, pdMS_TO_TICKS(1000)) != pdTRUE) return 0;

    uint32_t offset = lba * DISK_SECTOR_SIZE + lba_off;
    digitalWrite(LED_BUILTIN, HIGH);
    esp_err_t res = wl_write(wlHandle, offset, buf, sz);
    digitalWrite(LED_BUILTIN, LOW);

    lastMSCActivity = millis();
    xSemaphoreGive(ioMutex);

    return (res == ESP_OK) ? static_cast<int32_t>(sz) : 0;
}

static int32_t onRead(uint32_t lba, uint32_t lba_off, void *buf, uint32_t sz)
{
    if (wlHandle == WL_INVALID_HANDLE) return 0;
    if (xSemaphoreTake(ioMutex, pdMS_TO_TICKS(1000)) != pdTRUE) return 0;

    uint32_t offset = lba * DISK_SECTOR_SIZE + lba_off;
    esp_err_t res = wl_read(wlHandle, offset, buf, sz);

    xSemaphoreGive(ioMutex);
    return (res == ESP_OK) ? static_cast<int32_t>(sz) : 0;
}

// ------------------- Partition helpers -------------------
void clearPartition()
{
    if (wlHandle == WL_INVALID_HANDLE) return;
    if (xSemaphoreTake(ioMutex, pdMS_TO_TICKS(1000)) != pdTRUE) return;

    uint8_t zero[DISK_SECTOR_SIZE] = {0};
    digitalWrite(LED_BUILTIN, HIGH);
    for (uint32_t lba = 0; lba < DISK_SECTOR_COUNT; lba++) {
        wl_write(wlHandle, lba * DISK_SECTOR_SIZE, zero, DISK_SECTOR_SIZE);
    }
    digitalWrite(LED_BUILTIN, LOW);

    xSemaphoreGive(ioMutex);
}

// ------------------- Initialization -------------------
void begin()
{
    ioMutex = xSemaphoreCreateMutex();
    if (!ioMutex) return;

    pinMode(LED_BUILTIN, OUTPUT);

    const esp_partition_t *storagePartition = esp_partition_find_first(
        ESP_PARTITION_TYPE_DATA,
        ESP_PARTITION_SUBTYPE_ANY,
        "storage"
    );
    if (!storagePartition) return;
    wl_mount(storagePartition, &wlHandle);
    if (wlHandle == WL_INVALID_HANDLE) return;

    MSC.vendorID("Nekos");
    MSC.productID("Nekos Scripts");
    MSC.productRevision("1.0");
    MSC.onRead(onRead);
    MSC.onWrite(onWrite);
    MSC.mediaPresent(true);
    MSC.begin(DISK_SECTOR_COUNT, DISK_SECTOR_SIZE);
}

} // namespace fs
} // namespace nekos
