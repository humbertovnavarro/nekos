#pragma once
#include "config/device.h"
#include "NekosCommand.h"
#include <memory>
#include "Arduino.h"
namespace nekos {
    class CommandRegistry {
        public:
            static std::map<String, std::unique_ptr<Command>> commandMap;
            static Command* registerCommand(
                const char* name,
                std::function<void(Command* cmd, const char*)> cb
            );
            static bool executeCommand(const char* name, const char* args);
        private:
    };
}
