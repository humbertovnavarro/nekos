#include "NekosCommandRegistry.h"
#include "NekosConsole.h"
#include "NekosArgParse.h"
#include "NekosCommand.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
namespace nekos
{
    struct CommandEntry {
        const char* name;
        Command* cmd;
    };

    // Adjust size as needed
    static CommandEntry commandList[NEKOS_COMMAND_REGISTRY_TABLE_SIZE];
    static size_t commandCount = 0;

    Command* CommandRegistry::registerCommand(
        const char* name,
        CommandCallback cb
    )
    {
        if (commandCount >= NEKOS_COMMAND_REGISTRY_TABLE_SIZE) {
            Console::logf("😿 Command registry full!");
            return nullptr;
        }

        Command* c = new Command(name, cb);
        commandList[commandCount++] = { name, c };
        c->inQueue    = xQueueCreate(NEKOS_STDIO_BUFFER_COUNT, sizeof(char[NEKOS_STDIO_NUM_CHARS]));
        c->outQueue   = xQueueCreate(NEKOS_STDIO_BUFFER_COUNT, sizeof(char[NEKOS_STDIO_NUM_CHARS]));
        Console::logf("😸 registered command [%s]", name);
        return c;
    }

    bool CommandRegistry::executeCommand(const char *name, const char *args)
    {
        Command* cmd = nullptr;

        for (size_t i = 0; i < commandCount; i++) {
            if (strcmp(commandList[i].name, name) == 0) {
                cmd = commandList[i].cmd;
                break;
            }
        }

        if (!cmd) {
            Console::logf("😿 Unknown command: %s\n", name);
            return false;
        }
        if (!cmd->cb) {
            Console::logf("🙀 Command '%s' has no callback!\n", name);
            return false;
        }
        if (args && !cmd->args.parse(args)) {
            Console::log(cmd->args.usage(name).c_str());
            return false;
        }

        cmd->cb(cmd, args);
        return true;
    }
}
