#include "NekosRegistry.h"
#include "FreeRTOS.h"
namespace nekos {

char Registry::keys[Registry::MAX_ENTRIES][Registry::MAX_KEY_LEN] = {0};
char Registry::values[Registry::MAX_ENTRIES][Registry::MAX_VALUE_LEN] = {0};
size_t Registry::count = 0;
SemaphoreHandle_t Registry::mutex = nullptr;

void Registry::initMutex() {
    if (!mutex) {
        mutex = xSemaphoreCreateMutex();
    }
}

void Registry::set(const char* key, const char* value) {
    initMutex();
    if (xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE) {
        // Check if key exists
        for (size_t i = 0; i < count; i++) {
            if (strcmp(keys[i], key) == 0) {
                strncpy(values[i], value, MAX_VALUE_LEN - 1);
                values[i][MAX_VALUE_LEN - 1] = '\0';
                xSemaphoreGive(mutex);
                return;
            }
        }
        // Add new key if space
        if (count < MAX_ENTRIES) {
            strncpy(keys[count], key, MAX_KEY_LEN - 1);
            keys[count][MAX_KEY_LEN - 1] = '\0';
            strncpy(values[count], value, MAX_VALUE_LEN - 1);
            values[count][MAX_VALUE_LEN - 1] = '\0';
            count++;
        }
        xSemaphoreGive(mutex);
    }
}

const char* Registry::get(const char* key, const char* defaultValue) {
    initMutex();
    const char* result = defaultValue;
    if (xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE) {
        for (size_t i = 0; i < count; i++) {
            if (strcmp(keys[i], key) == 0) {
                result = values[i];
                break;
            }
        }
        xSemaphoreGive(mutex);
    }
    return result;
}

bool Registry::remove(const char* key) {
    initMutex();
    bool removed = false;
    if (xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE) {
        for (size_t i = 0; i < count; i++) {
            if (strcmp(keys[i], key) == 0) {
                // Shift all later entries down
                for (size_t j = i; j < count - 1; j++) {
                    strncpy(keys[j], keys[j + 1], MAX_KEY_LEN);
                    strncpy(values[j], values[j + 1], MAX_VALUE_LEN);
                }
                count--;
                removed = true;
                break;
            }
        }
        xSemaphoreGive(mutex);
    }
    return removed;
}

bool Registry::has(const char* key) {
    initMutex();
    bool exists = false;
    if (xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE) {
        for (size_t i = 0; i < count; i++) {
            if (strcmp(keys[i], key) == 0) {
                exists = true;
                break;
            }
        }
        xSemaphoreGive(mutex);
    }
    return exists;
}

void Registry::clear() {
    initMutex();
    if (xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE) {
        count = 0;
        xSemaphoreGive(mutex);
    }
}

}
