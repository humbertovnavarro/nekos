#pragma once
#include "NekosCommand.h"
#include "defaults.h"

namespace nekos
{
    using CommandCallback = void (*)(Command*, const char*);
    class CommandRegistry
    {
    public:
        static Command* registerCommand(const char* name, CommandCallback cb);
        static bool executeCommand(const char* name, const char* args);
    private:
    };
}
