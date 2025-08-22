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
            static bool commandExists(const char* name) {
                return commandMap.find(name) != commandMap.end();
            }
            static Command* getCommand(const char* name) {
                auto it = commandMap.find(name);
                if (it != commandMap.end()) {
                    return it->second.get();
                }
                return nullptr;
            }

        private:
    };
}
