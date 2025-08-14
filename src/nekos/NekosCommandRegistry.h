#pragma once
#include "config/device.h"
#include "NekosCommand.h"
#include <memory>
namespace nekos {
    class CommandRegistry {
        public:
            static std::map<std::string, std::unique_ptr<Command>> commandMap;
            static Command* registerCommand(const char* name,
                std::function<void(Command* cmd)> cb
            );
            static bool executeCommand(const char* name, const char* args);
        private:
    };
}
