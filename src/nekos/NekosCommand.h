#pragma once
#include "Arduino.h"
#include "NekosArgParse.h"

namespace nekos {
    class Command;
    using  CommandCallback = std::function<void(Command*, const char*)>;
    class Command {
        public:
            const char* name;
            String output;
            ArgParse args;
            CommandCallback cb;
            Command(const char* n, CommandCallback f) : name(n), cb(f) {}
            Command() = delete;
    };
}