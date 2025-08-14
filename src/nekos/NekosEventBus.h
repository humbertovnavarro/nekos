#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <map>
#include <vector>
#include <memory>
#include <functional>
#include <string>
#include "NekosConsole.h"
#include "NekosCommandRegistry.h"

namespace nekos {
    enum class Topic {
        GLOBAL,
        COMMAND_EXECUTE,
        REBOOT,
        CUSTOM
    };
    class EventBus {
    public:
        static void subscribe(Command* cmd, Topic topic);
        static void publish(Topic topic);
        static void processQueue(Command* cmd);
    private:
        static inline std::map<Topic, std::vector<Command*>> subscribers;
        static inline std::vector<Command*> allSubscribers;
    };
}
