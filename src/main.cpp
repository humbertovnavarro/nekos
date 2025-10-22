#include "Arduino.h"
#include "USB.h"
#include "USBMSC.h"
#include "FFat.h"
#include "sys/LuaCompiler.hpp"
#include "sys/LuaProcessScheduler.hpp"

constexpr uint32_t SECTOR_SIZE = 512;
static const esp_partition_t* ffatPartition = nullptr;

USBMSC usbMsc;

bool mscStartStop(uint8_t, bool start, bool load_eject) {
    if (load_eject) {
        if (start) {
            FFat.end();
        } else {
            FFat.begin(true);
        }
    }
    return true;
}

int32_t mscRead(uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize) {
    uint32_t addr = lba * SECTOR_SIZE + offset;
    if (esp_partition_read(ffatPartition, addr, buffer, bufsize) != ESP_OK) return -1;
    return bufsize;
}

int32_t mscWrite(uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize) {
    uint32_t addr = lba * SECTOR_SIZE + offset;
    if (esp_partition_erase_range(ffatPartition, addr & ~(0x1000-1), 0x1000) != ESP_OK)
        return -1;
    if (esp_partition_write(ffatPartition, addr, buffer, bufsize) != ESP_OK)
        return -1;
    return bufsize;
}

void setupUSBMSC() {
    USB.begin();
    ffatPartition = esp_partition_find_first(
        ESP_PARTITION_TYPE_DATA,
        ESP_PARTITION_SUBTYPE_DATA_FAT,
        "ffat"
    );
    if (!FFat.begin(true)) {
        return;
    }
    usbMsc.vendorID("LuaVM");
    usbMsc.productID("ScriptDrive");
    usbMsc.productRevision("1.0");
    usbMsc.mediaPresent(true);
    usbMsc.onRead(mscRead);
    usbMsc.onWrite(mscWrite);
    usbMsc.onStartStop(mscStartStop);
    uint32_t blocks = FFat.totalBytes() / SECTOR_SIZE;
    usbMsc.begin(blocks, SECTOR_SIZE);
}

void setup() {
    setupUSBMSC();
    Serial.begin();
    enumerateAndCompileLuaFiles("/");
    LuaProcessScheduler::begin();
}

void loop() {
    unsigned long startTime = micros();
    LuaProcessScheduler::run("/boot.luac");
    unsigned long endTime = micros();
    unsigned long duration = endTime - startTime;
    Serial.print("Lua process time: ");
    Serial.print(duration);
    Serial.println(" µs");
    vTaskDelay(10 / portTICK_PERIOD_MS);
}
