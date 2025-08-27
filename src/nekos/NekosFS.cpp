#include "NekosFS.h"
#include "Arduino.h"
#include "USB.h"
#include "USBMSC.h"
#include "semphr.h"

namespace nekos {
namespace fs {
TaskHandle_t handleMSCIdleTaskHandle;
USBMSC MSC;
SemaphoreHandle_t ioMutex;
const esp_partition_t *storagePartition = nullptr;
esp_err_t res;

const uint16_t DISK_SECTOR_SIZE  = 512;
const uint32_t DISK_SECTOR_COUNT = 2000;                 // ~1MB
const size_t   DISK_SIZE_BYTES   = (size_t)DISK_SECTOR_COUNT * DISK_SECTOR_SIZE;

uint8_t pageBuffer[SPI_FLASH_SEC_SIZE];                  // 4KB flash RMW

// ------------------- Timing -------------------
uint32_t lastMSCActivity = 0;
const uint32_t MSC_IDLE_TIMEOUT_MS = 2000; // 2 seconds

// ------------------- Flash write -------------------
static void writeFlash(uint32_t offset, const uint8_t *buffer, uint32_t bufsize)
{
  while (bufsize) {
    uint32_t pageOffset = offset % SPI_FLASH_SEC_SIZE;
    uint32_t pageStart  = offset - pageOffset;
    uint32_t chunk      = std::min<uint32_t>(bufsize, SPI_FLASH_SEC_SIZE - pageOffset);
    res = esp_partition_read(storagePartition, pageStart, pageBuffer, SPI_FLASH_SEC_SIZE);
    memcpy(pageBuffer + pageOffset, buffer, chunk);
    res = esp_partition_erase_range(storagePartition, pageStart, SPI_FLASH_SEC_SIZE);
    res = esp_partition_write(storagePartition, pageStart, pageBuffer, SPI_FLASH_SEC_SIZE);
    offset += chunk;
    buffer += chunk;
    bufsize -= chunk;
  }
}

// ------------------- MSC callbacks -------------------
static int32_t onWrite(uint32_t lba, uint32_t lba_off, uint8_t *buf, uint32_t sz)
{
  if (!storagePartition) return 0;
  while(xSemaphoreTake(ioMutex, 1000)) {}
  uint32_t off = lba * DISK_SECTOR_SIZE + lba_off;
  digitalWrite(LED_BUILTIN, HIGH);
  writeFlash(off, buf, sz);
  digitalWrite(LED_BUILTIN, LOW);
  lastMSCActivity = millis();
  return (int32_t)sz;
}

static int32_t onRead(uint32_t lba, uint32_t lba_off, void *buf, uint32_t sz)
{
  if (!storagePartition) return 0;
  uint32_t off = lba * DISK_SECTOR_SIZE + lba_off;
  res = esp_partition_read(storagePartition, off, buf, sz);
  return (int32_t)sz;
}

// ------------------- Partition helpers -------------------
void clearPartition()
{
  if (!storagePartition) return;
  digitalWrite(LED_BUILTIN, HIGH);
  uint8_t zero[DISK_SECTOR_SIZE] = {0};
  for (uint32_t lba = 0; lba < DISK_SECTOR_COUNT; lba++) {
    onWrite(lba, 0, zero, DISK_SECTOR_SIZE);
  }
  digitalWrite(LED_BUILTIN, LOW);
}

// ------------------- Idle handler -------------------
void begin()
{
  ioMutex = xSemaphoreCreateMutex();
  storagePartition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA,
                                              ESP_PARTITION_SUBTYPE_ANY,
                                              "storage");
  if (!storagePartition) return;
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
