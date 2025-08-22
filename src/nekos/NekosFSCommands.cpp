#include "NekosConsole.h"
#include "NekosFS.h"
namespace nekos {
    void registerFSCommands() {
        CommandRegistry::registerCommand("rm", [](Command* cmd, const char* args) {
            const char* path = cmd->args.get("path");
            if (fs::deleteFile(path)) {
                Console::logf("[rm] Deleted: %s\n", path);
            } else {
                Console::logf("[rm] Failed to delete: %s\n", path);
            }
        })->args.addArgument("path", true);

        CommandRegistry::registerCommand("touch", [](Command* cmd, const char* args) {
            const char* path = cmd->args.get("path");
            if (fs::touchFile(path)) {
                Console::logf("[touch] File created/updated: %s\n", path);
            } else {
                Console::logf("[touch] Failed: %s\n", path);
            }
        })->args.addArgument("path", true);

        CommandRegistry::registerCommand("echo", [](Command* cmd, const char* args) {
            const char* path = cmd->args.get("path");
            const char* text = cmd->args.get("text");
            if (fs::writeFile(path, text)) {
                Console::logf("[echo] Wrote to %s\n", path);
            } else {
                Console::logf("[echo] Failed to write to %s\n", path);
            }
        })
        ->args.addArgument("path", true)
        ->addArgument("text", true);

        CommandRegistry::registerCommand("mkdir", [](Command* cmd, const char* args) {
            const char* path = cmd->args.get("path");
            if (fs::makeDir(path)) {
                Console::logf("[mkdir] Created path: %s\n", path);
            } else {
                Console::logf("[mkdir] Failed to create path: %s\n", path);
            }
        })->args.addArgument("path", true);
        
        CommandRegistry::registerCommand("cat", [](Command* cmd, const char* args) {
            const char* path = cmd->args.get("path");
            String content = fs::readFile(path);
            if (content.length() == 0) {
                Console::logf("[cat] Cannot read file: %s\n", path);
            } else {
                Serial.println(content);
            }
        })->args.addArgument("path", true);
    }
}