#include "Arduino.h"
#include "LuaProcessScheduler.hpp"
#include "neopixel.hpp"
#include "display.hpp"
#include "input.hpp"
#include "USB.h"
#include "USBMSC.h"
#include "FFat.h"

constexpr uint32_t SECTOR_SIZE = 512;
static const esp_partition_t* ffatPartition = nullptr;
static uint32_t blockCount = 0;

bool mscStartStop(uint8_t, bool start, bool load_eject) {
    if (load_eject) {
        if (start) {
            Serial.println("💽 Host requested disk load");
            FFat.end();  // ensure not mounted locally
        } else {
            Serial.println("💽 Host requested disk eject");
            // after host ejects, remount so ESP can use it
            FFat.begin(true);
        }
    }
    return true;
}

int32_t mscRead(uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize) {
    uint32_t addr = (lba * SECTOR_SIZE) + offset;

    esp_err_t err = esp_partition_read(ffatPartition, addr, buffer, bufsize);
    if (err != ESP_OK) {
        ESP_LOGE("MSC", "Read failed: 0x%x @ %lu", err, addr);
        return -1;
    }

    return bufsize;
}

int32_t mscWrite(uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize) {
    uint32_t addr = (lba * SECTOR_SIZE) + offset;
    esp_err_t err = esp_partition_erase_range(ffatPartition, addr & ~(0x1000 - 1), 0x1000);
    if (err != ESP_OK && err != ESP_ERR_INVALID_SIZE) {
        ESP_LOGE("MSC", "Erase failed: 0x%x", err);
        return -1;
    }
    err = esp_partition_write(ffatPartition, addr, buffer, bufsize);
    if (err != ESP_OK) {
        ESP_LOGE("MSC", "Write failed: 0x%x", err);
        return -1;
    }
    return bufsize;
}

USBMSC usbMsc;
void setup() {
    ffatPartition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_FAT, "ffat");
    if (!FFat.begin(true)) {
        Serial.println("⚠️ FFat mount failed!");
        return;
    }
    Serial.begin(115200);
    neopixel.begin();
    u8g2.begin();
    inputBegin();
    LuaProcessScheduler::begin();
    USB.begin();
    uint32_t blockCount = FFat.totalBytes() / SECTOR_SIZE;
    Serial.printf("FFat mounted. Total bytes: %zu  (%.2f MB)\n",
                  FFat.totalBytes(), FFat.totalBytes() / 1024.0 / 1024.0);
    Serial.printf("USB MSC block count: %u\n", blockCount);
    USB.begin();
    usbMsc.vendorID("LuaVM");
    usbMsc.productID("ScriptDrive");
    usbMsc.productRevision("1.0");
    usbMsc.mediaPresent(true);
    Serial.println("✅ USB MSC ready — you should see 'ScriptDrive' on your computer!");
    usbMsc.onRead(mscRead);
    usbMsc.onStartStop(mscStartStop);
    usbMsc.onWrite(mscWrite);
    usbMsc.begin(blockCount, SECTOR_SIZE);
    vTaskDelay(100);
    LuaProcessScheduler::runFile("/boot.lua");
}

void loop() {
    delay(1000);
    Serial.println("Heartbeat");
}
