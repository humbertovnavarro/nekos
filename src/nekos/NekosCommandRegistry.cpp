#include "config/device.h"
#include "NekosCommandRegistry.h"
#include "NekosConsole.h"
#include "NekosArgParse.h"
#include "Arduino.h"
namespace nekos
{
    std::map<String, std::unique_ptr<Command>> CommandRegistry::commandMap;
    Command* CommandRegistry::registerCommand(
        const char* name,
        std::function<void(Command *cmd, const char* args)> cb
    )
    {
        Command *c = new Command(name, cb);
        commandMap[name] = std::unique_ptr<Command>(c);
        Console::logf("😸 registered command [%s]", name);
        return commandMap[name].get();
    }
}
