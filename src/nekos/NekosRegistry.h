#pragma once
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
namespace nekos {
    class Registry {
    public:
        Registry() = default;
        // Initialize registry memory and mutex
        void init();
        // Set or overwrite a key-value pair
        bool set(const char* key, const char* value);
        // Retrieve the value for a given key (nullptr if not found)
        const char* get(const char* key);
        // Check if a key exists
        bool has(const char* key);
        // Remove a key-value pair
        bool remove(const char* key);

    private:
        static SemaphoreHandle_t mutex;
        static char* keys;
        static char* values;
    };
}
