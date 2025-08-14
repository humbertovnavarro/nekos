#pragma once
#include "NekosArgParse.h"

namespace nekos {
    class Command {
        public:
            std::string name;
            ArgParse args;
            TaskHandle_t taskHandle = nullptr;
            QueueHandle_t topicQueue;
            QueueHandle_t inQueue;
            QueueHandle_t outQueue;
            std::function<void(Command*)> cb;
            Command(const char* n, std::function<void(Command*)> f) : name(n), cb(f) {}
            Command() = delete;
            boolean isBackgroundTask() { return taskHandle != nullptr; };
    };
}
