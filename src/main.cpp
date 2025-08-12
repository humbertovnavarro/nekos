#include "Arduino.h"
#include "lib/Console.h"
#include "WiFi.h"

void setup() {
    Console::registerCommand("help", [](const char* args){
        Console::log("Available commands:");
        int count = Console::getCommandCount();
        for (int i = 0; i < count; i++) {
            const char* cmdName = Console::getCommandName(i);
            if (cmdName) {
                Console::logf("  %s", cmdName);
            }
        }
    });
    
    Console::registerCommand("heap", [](const char* args){
        size_t freeHeap = xPortGetFreeHeapSize();
        size_t minHeap = xPortGetMinimumEverFreeHeapSize();
        Console::logf("Free Heap: %u bytes", (unsigned)freeHeap);
        Console::logf("Min Ever Free Heap: %u bytes", (unsigned)minHeap);
    });

    Console::registerCommand("uptime", [](const char* args){
        uint64_t ms = esp_timer_get_time() / 1000;
        uint64_t seconds = ms / 1000;
        uint64_t minutes = seconds / 60;
        uint64_t hours = minutes / 60;
        Console::logf("Uptime: %llu hours, %llu minutes, %llu seconds",
                    hours, minutes % 60, seconds % 60);
    });
    
    Console::begin();
}

void loop() {
    Console::poll();
}