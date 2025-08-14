#include "NekosConsole.h"
namespace nekos {
    void registerSysCommands() {
        Console::commands.registerCommand("heap", [](Command* cmd) {
            size_t freeHeap = xPortGetFreeHeapSize();
            size_t minHeap = xPortGetMinimumEverFreeHeapSize();
            Console::logf("Free Heap: %u bytes\n", (unsigned)freeHeap);
            Console::logf("Min Ever Free Heap: %u bytes\n", (unsigned)minHeap);
        });
        Console::commands.registerCommand("help", [](Command* cmd) {
                for (auto& [cmdName, cmdPtr] : Console::commands.commandMap) {
                    String usageStr = cmdPtr->args.usage(cmdName.c_str());
                    Console::logf("%s\n", usageStr.c_str());
                }
        });
    }

}