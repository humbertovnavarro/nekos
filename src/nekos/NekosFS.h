#include "Arduino.h"
#include "USBMSC.h"
#include "semphr.h"
#include "esp_partition.h"

namespace nekos {
namespace fs {
// Constants for disk geometry
extern const uint32_t DISK_SECTOR_COUNT;
extern const uint16_t DISK_SECTOR_SIZE;
extern SemaphoreHandle_t ioMutex;

// Functions
void begin();
}
}
