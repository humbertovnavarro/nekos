#include "NekosCoreUtils.h"
#include "NekosConsole.h"
#include "NekosFS.h"
#include "NekosNet.h"
#include "NekosArgParse.h"
#include "Arduino.h"
#define ARG(cmd, name) ((cmd)->args.get(name))

namespace nekos {

void registerSysUtils() {
    Console::commands.registerCommand("heap", [](Command* cmd) {
        size_t freeHeap = xPortGetFreeHeapSize();
        size_t minHeap = xPortGetMinimumEverFreeHeapSize();
        Console::logf("Free Heap: %u bytes", (unsigned)freeHeap);
        Console::logf("Min Ever Free Heap: %u bytes", (unsigned)minHeap);
    });
}

void registerFileUtils() {
    Console::commands.registerCommand("cat", [](Command*cmd) {
        const char* path = ARG(cmd, "path");
        String content = nekos::fs::readFile(path);
        if (content.length() == 0) {
            Console::logf("[cat] Cannot read file: %s", path);
        } else {
            Serial.println(content);
        }
    })->args.addArgument("path", true);

    Console::commands.registerCommand("rm", [](Command* cmd) {
        const char* path = ARG(cmd, "path");
        if (nekos::fs::deleteFile(path)) {
            Console::logf("[rm] Deleted: %s", path);
        } else {
            Console::logf("[rm] Failed to delete: %s", path);
        }
    })->args.addArgument("path", true);

    Console::commands.registerCommand("touch", [](Command* cmd) {
        const char* path = ARG(cmd, "file");

        if (nekos::fs::touchFile(path)) {
            Console::logf("[touch] File created/updated: %s", path);
        } else {
            Console::logf("[touch] Failed: %s", path);
        }
    })->args.addArgument("path", true);

    Console::commands.registerCommand("echo", [](Command* cmd) {
        const char* path = ARG(cmd, "path");
        const char* text = ARG(cmd, "text");
        if (nekos::fs::writeFile(path,text)) {
            Console::logf("[echo] Wrote to %s", path);
        } else {
            Console::logf("[echo] Failed to write to %s", path);
        }
    })
    ->args.addArgument("path", true)
    ->addArgument("text", true);

    Console::commands.registerCommand("mkdir", [](Command* cmd) {
        const char* path = ARG(cmd, "path");
        if (nekos::fs::makeDir(path)) {
            Console::logf("[mkdir] Created path: %s", path);
        } else {
            Console::logf("[mkdir] Failed to create path: %s", path);
        }
    })->args.addArgument("path");
}

void registerCoreUtils() {
        Console::commands.registerCommand("help", [](Command* cmd) {
            for (auto& [cmdName, cmdPtr] : Console::commands.commandMap) {
                String usageStr = cmdPtr->args.usage(cmdName.c_str());
                Console::logf("%s\n", usageStr.c_str());
            }
        });
        registerSysUtils();
        registerFileUtils();
    }
}

