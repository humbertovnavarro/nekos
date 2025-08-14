#include "NekosRegistry.h"
#include <cstring>
#include "NekosMemConfig.h"
namespace nekos {
    char* keys;
    char* values;
    SemaphoreHandle_t Registry::mutex = nullptr;
    void Registry::init() {
        if (!mutex) {
            mutex = xSemaphoreCreateMutex();
            if (!mutex) {
                // handle error
                return;
            }
        }
        keys = (char*) PSMALLOC_OR_MALLOC(sizeof(char) * MAX_REGISTRY_ENTRIES * MAX_REGISTRY_VALUE_LEN);
        values = (char*) PSMALLOC_OR_MALLOC(sizeof(char) * MAX_REGISTRY_ENTRIES * MAX_REGISTRY_VALUE_LEN);
        memset(keys, sizeof(char) * MAX_REGISTRY_ENTRIES * MAX_REGISTRY_VALUE_LEN, 0);
        memset(values, sizeof(char) * MAX_REGISTRY_ENTRIES * MAX_REGISTRY_VALUE_LEN, 0);
    }

    bool Registry::set(const char* key, const char* value) {
        if (!key || !value) return false;

        xSemaphoreTake(mutex, portMAX_DELAY);

        for (size_t i = 0; i < MAX_REGISTRY_ENTRIES; i++) {
            char* k = keys + (i * MAX_REGISTRY_KEY_LEN);

            // Found existing key
            if (strncmp(k, key, MAX_REGISTRY_KEY_LEN) == 0) {
                strncpy(values + (i * MAX_REGISTRY_VALUE_LEN), value, MAX_REGISTRY_VALUE_LEN - 1);
                (values + (i * MAX_REGISTRY_VALUE_LEN))[MAX_REGISTRY_VALUE_LEN - 1] = '\0';
                xSemaphoreGive(mutex);
                return true;
            }

            // Empty slot
            if (k[0] == '\0') {
                strncpy(k, key, MAX_REGISTRY_KEY_LEN - 1);
                k[MAX_REGISTRY_KEY_LEN - 1] = '\0';
                strncpy(values + (i * MAX_REGISTRY_VALUE_LEN), value, MAX_REGISTRY_VALUE_LEN - 1);
                (values + (i * MAX_REGISTRY_VALUE_LEN))[MAX_REGISTRY_VALUE_LEN - 1] = '\0';
                xSemaphoreGive(mutex);
                return true;
            }
        }

        xSemaphoreGive(mutex);
        return false; // No space
    }

    const char* Registry::get(const char* key) {
        if (!key) return nullptr;

        xSemaphoreTake(mutex, portMAX_DELAY);
        for (size_t i = 0; i < MAX_REGISTRY_ENTRIES; i++) {
            char* k = keys + (i * MAX_REGISTRY_KEY_LEN);
            if (strncmp(k, key, MAX_REGISTRY_KEY_LEN) == 0) {
                const char* val = values + (i * MAX_REGISTRY_VALUE_LEN);
                xSemaphoreGive(mutex);
                return val;
            }
        }
        xSemaphoreGive(mutex);
        return nullptr;
    }

    bool Registry::has(const char* key) {
        if (!key) return false;

        xSemaphoreTake(mutex, portMAX_DELAY);
        for (size_t i = 0; i < MAX_REGISTRY_ENTRIES; i++) {
            if (strncmp(keys + (i * MAX_REGISTRY_KEY_LEN), key, MAX_REGISTRY_KEY_LEN) == 0) {
                xSemaphoreGive(mutex);
                return true;
            }
        }
        xSemaphoreGive(mutex);
        return false;
    }

    bool Registry::remove(const char* key) {
        if (!key) return false;

        xSemaphoreTake(mutex, portMAX_DELAY);
        for (size_t i = 0; i < MAX_REGISTRY_ENTRIES; i++) {
            char* k = keys + (i * MAX_REGISTRY_KEY_LEN);
            if (strncmp(k, key, MAX_REGISTRY_KEY_LEN) == 0) {
                k[0] = '\0';
                values[i * MAX_REGISTRY_VALUE_LEN] = '\0';
                xSemaphoreGive(mutex);
                return true;
            }
        }
        xSemaphoreGive(mutex);
        return false;
    }
}
