#include "defaults.h"
#include "NekosConsole.h"
#include "NekosLua.h"

#ifdef NEKOS_HAS_WIFI
#include "WiFi.h"
#endif

namespace nekos {
    void printVendorInfo() {
        #ifdef ESP32
        Console::logf("Chip: %s rev %d\n", ESP.getChipModel(), ESP.getChipRevision());
        Console::logf("Cores: %d\n", ESP.getChipCores());
        Console::logf("CPU freq: %d MHz\n", ESP.getCpuFreqMHz());
        Console::logf("Flash: %u MB\n", ESP.getFlashChipSize() / (1024 * 1024));
        Console::logf("Sketch: %u KB, Free: %u KB\n", ESP.getSketchSize() / 1024, ESP.getFreeSketchSpace() / 1024);
        Console::logf("SDK: %s\n", ESP.getSdkVersion());
        #endif
        #ifdef ESP8266
            Console::logf("Chip ID: %lu\n", ESP.getChipId());
            Console::logf("Boot mode: %d\n", ESP.getBootMode());
        #endif
    }

    void printNetworkInfo() {
        #ifdef NEKOS_HAS_WIFI
            Console::logf("WiFi MAC: %s\n", WiFi.macAddress().c_str());
        #endif
    }

    void printMemoryStats() {
    #ifdef ESP32
        size_t freeHeapKB = ESP.getFreeHeap() / 1024;
        size_t minHeapKB = ESP.getMinFreeHeap() / 1024;
        size_t totalHeapKB = ESP.getHeapSize() / 1024;
        size_t usedHeapKB = totalHeapKB - freeHeapKB;
        float usedHeapPercent = (usedHeapKB * 100.0) / totalHeapKB;

        size_t sketchSizeKB = ESP.getSketchSize() / 1024;
        size_t freeSketchKB = ESP.getFreeSketchSpace() / 1024;
        float usedSketchPercent = (sketchSizeKB * 100.0) / (sketchSizeKB + freeSketchKB);

        Console::log("=== ESP32 Memory Stats (KB) ===");
        Console::logf("Heap: used %u KB (%.1f%%), free %u KB, min ever %u KB", 
                    usedHeapKB, usedHeapPercent, freeHeapKB, minHeapKB);
        Console::logf("Sketch: used %u KB (%.1f%%), free %u KB", 
                    sketchSizeKB, usedSketchPercent, freeSketchKB);
        Console::logf("Total heap: %u KB, Total flash for sketch: %u KB", 
                    totalHeapKB, sketchSizeKB + freeSketchKB);
        if (psramFound()) {
            Console::logf("\nPSRAM found! Size: %.2f MB\n", ESP.getPsramSize() / 1024.0 / 1024.0);
        }
    #endif
    }


    void registerSysCommands() {
    #ifdef NEOPIXEL_PIN
        pixel.begin();
        pixel.setBrightness(0);
        Console::commands.registerCommand("neopixel_set", [](Command* cmd, const char* args) {
            int led = atoi(cmd->args.get("led"));
            int r = atoi(cmd->args.get("r"));
            int g = atoi(cmd->args.get("g"));
            int b = atoi(cmd->args.get("b"));
            int i = atoi(cmd->args.get("i"));
            pixel.setPixelColor(led, pixel.Color(r, g, b));
            pixel.setBrightness(i);
            pixel.show();
            Console::logf("Set LED %d to RGB(%d,%d,%d)\n", led, r, g, b);
        })->args.addArgument("led", true)
        ->addArgument("r", true)
        ->addArgument("g", true)
        ->addArgument("b", true)
        ->addArgument("i", false, "20", "intensity (0-255)");
    #endif

    Console::commands.registerCommand("eval", [](Command* cmd, const char* args) {
        luaExec(args);
    });

    Console::commands.registerCommand("heap", [](Command* cmd, const char* args) {
        size_t freeHeap = xPortGetFreeHeapSize();
        size_t minHeap = xPortGetMinimumEverFreeHeapSize();
        Console::logf("Free Heap: %u bytes\n", (unsigned)freeHeap);
        Console::logf("Min Ever Free Heap: %u bytes\n", (unsigned)minHeap);
    });

    // Console::commands.registerCommand("help", [](Command* cmd, const char* args) {
    //         for (auto& [cmdName, cmdPtr] : Console::commands.commandMap) {
    //             String usageStr = cmdPtr->args.usage(cmdName.c_str());
    //             Console::logf("%s\n", usageStr.c_str());
    //         }
    // });

    Console::commands.registerCommand("lshw", [](Command* cmd, const char* args) {
        Console::log("=== Hardware Info ===");
        #ifdef ARDUINO_BOARD
        Console::log("Board: " ARDUINO_BOARD);
        #endif
        #ifdef ARDUINO_VARIANT
        Console::log("Variant: " ARDUINO_VARIANT);
        #endif
        #ifdef F_CPU
        Console::logf("CPU: %lu MHz\n", F_CPU / 1000000);
        #endif
        #ifdef ARDUINO
        Console::logf("Arduino API: %ld\n", ARDUINO);
        #endif
        printMemoryStats();
        printVendorInfo();
        printNetworkInfo();
    #ifdef __AVR__
        extern int __heap_start, *__brkval;
        int freeRam() { int v; return (int)&v - (__brkval==0 ? (int)&__heap_start : (int)__brkval); }
        Console::logf("Free RAM: %d bytes\n", freeRam());
    #endif
    });
    }
}