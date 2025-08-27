#include "NekosConsole.h"
#include "NekosFS.h"
#include "FFat.h"
namespace nekos {
    void registerFSCommands() {
        CommandRegistry::registerCommand("rm", [](Command* cmd, const char* args) {
            const char* path = cmd->args.get("path");
            if (FFat.remove(path)) {
                Console::logf("[rm] Deleted: %s\n", path);
            } else {
                Console::logf("[rm] Failed to delete: %s\n", path);
            }
        })->args.addArgument("path", true);
        CommandRegistry::registerCommand("mkdir", [](Command* cmd, const char* args) {
            const char* path = cmd->args.get("path");
            if (FFat.mkdir(args)) {
                Console::logf("[mkdir] Created path: %s\n", path);
            } else {
                Console::logf("[mkdir] Failed to create path: %s\n", path);
            }
        })->args.addArgument("path", true);
    }
}