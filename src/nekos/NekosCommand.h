#pragma once
#include "NekosArgParse.h"
#include "Arduino.h"
#include "FreeRTOS.h"
#include "task.h"
namespace nekos {
    class Command;
    typedef void (*CommandCallback)(Command*, const char*);

    class Command {
        public:
            const char* name;
            ArgParse args;
            TaskHandle_t taskHandle = nullptr;
            QueueHandle_t inQueue;
            QueueHandle_t outQueue;
            CommandCallback cb;
            Command(const char* n, CommandCallback f) : name(n), cb(f) {}
            Command() = delete;
            boolean isBackgroundTask() { return taskHandle != nullptr; };
    };
}
