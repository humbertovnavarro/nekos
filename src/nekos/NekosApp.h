#pragma once
#include "NekosMemConfig.h"
#include "NekosArgParse.h"
namespace nekos {
    enum AppSignal {
        SHUTDOWN,
    };

    class App {
        public:
            std::string name;
            ArgParse args;
            TaskHandle_t taskHandle = nullptr;
            QueueHandle_t appSignalQueue;
            QueueHandle_t inQueue;
            QueueHandle_t outQueue;
            char* inBufs[STDIO_BUFFER_COUNT];
            char* outBufs[STDIO_BUFFER_COUNT];
            std::function<void(App*)> cb;
            App(const char* n, std::function<void(App*)> f) : name(n), cb(f) {}
            App() = delete;
            boolean isBackgroundTask() { return taskHandle != nullptr; };
    };
}