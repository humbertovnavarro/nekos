#pragma once
#include <Arduino.h>
#include "FreeRTOS.h"
#include "semphr.h"
#define REGKEY_CWD "CWD"
namespace nekos {

class Registry {
    public:
        static void set(const char* key, const char* value);
        static const char* get(const char* key, const char* defaultValue = "");
        static bool remove(const char* key);
        static bool has(const char* key);
        static void clear();

    private:
        Registry() = default;
        ~Registry() = default;
        static void initMutex();
        static constexpr size_t MAX_ENTRIES = 512;
        static constexpr size_t MAX_KEY_LEN = 16;
        static constexpr size_t MAX_VALUE_LEN = 64;
        static char keys[MAX_ENTRIES][MAX_KEY_LEN];
        static char values[MAX_ENTRIES][MAX_VALUE_LEN];
        static size_t count;
        static SemaphoreHandle_t mutex;
    };
}
