#include "esp_heap_caps.h"
#include "lua.hpp"

// Custom allocator for Lua that tries to use PSRAM when available
inline static void* lua_psram_allocator(void* ud, void* ptr, size_t osize, size_t nsize) {
    (void)ud;  // unused
    (void)osize;

    // Free operation
    if (nsize == 0) {
        if (ptr) {
            heap_caps_free(ptr);
        }
        return nullptr;
    }

    // Realloc operation
    if (ptr) {
        void* newPtr = heap_caps_realloc(ptr, nsize, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
        if (newPtr) return newPtr;

        // Fallback to normal heap
        return heap_caps_realloc(ptr, nsize, MALLOC_CAP_DEFAULT);
    }

    // New allocation
    void* newMem = heap_caps_malloc(nsize, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (newMem) return newMem;

    // Fallback if PSRAM is full or unavailable
    return heap_caps_malloc(nsize, MALLOC_CAP_DEFAULT);
}
