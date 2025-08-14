#include "config/device.h"
#include "NekosCommandRegistry.h"
#include "NekosConsole.h"
#include "NekosArgParse.h"
#include "NekosEventBus.h"
namespace nekos
{
    std::map<std::string, std::unique_ptr<Command>> CommandRegistry::commandMap;
    Command* CommandRegistry::registerCommand(
        const char *name,
        std::function<void(Command *cmd)> cb
    )
    {
        Command *c = new Command(name, cb);
        commandMap[name] = std::unique_ptr<Command>(c);
        c->topicQueue = xQueueCreate(NEKOS_STDIO_BUFFER_COUNT, sizeof(Topic));
        c->inQueue = xQueueCreate(NEKOS_STDIO_BUFFER_COUNT, sizeof(char[NEKOS_STDIO_NUM_CHARS]));
        c->outQueue = xQueueCreate(NEKOS_STDIO_BUFFER_COUNT, sizeof(char[NEKOS_STDIO_NUM_CHARS]));
        Console::logf("😸 registered command [%s]", name);
        return commandMap[name].get();
    }

    bool CommandRegistry::executeCommand(const char *name, const char *args)
    {
        auto it = commandMap.find(name);
        if (it == commandMap.end())
        {
            Console::logf("😿 Unknown command: %s\n", name);
            return false;
        }
        Command *cmd = it->second.get();
        if (!cmd->cb)
        {
            Console::logf("🙀 Command '%s' has no callback!\n", name);
            return false;
        }
        if (args && !cmd->args.parse(args))
        {
            Console::log(cmd->args.usage(name).c_str());
            return false;
        }
        cmd->cb(cmd);
        return true;
    }
}
