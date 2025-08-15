#pragma once
#include "Arduino.h"
#include "NekosArgParse.h"

namespace nekos {
    class Command {
        public:
            String name;
            ArgParse args;
            TaskHandle_t taskHandle = nullptr;
            QueueHandle_t inQueue;
            QueueHandle_t outQueue;
            std::function<void(Command*, const char*)> cb;
            Command(const char* n, std::function<void(Command*, const char*)> f) : name(n), cb(f) {}
            Command() = delete;
            boolean isBackgroundTask() { return taskHandle != nullptr; };
    };
}
