#pragma once
#include "NekosArgParse.h"
namespace nekos {
    struct Command {
        std::string name;
        ArgParse args;
        std::function<void(Command*)> cb;
        Command(const char* n, std::function<void(Command*)> f) : name(n), cb(f) {}
        Command() = delete;
    };
}
