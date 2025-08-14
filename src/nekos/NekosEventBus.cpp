#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <map>
#include <memory>
#include <functional>
#include <string>
#include <vector>
#include "NekosConsole.h"
#include "NekosCommandRegistry.h"
#include "NekosEventBus.h"
namespace nekos {
    void EventBus::subscribe(Command* cmd, Topic topic) {
        auto& vec = subscribers[topic];
        if (std::find(vec.begin(), vec.end(), cmd) == vec.end()) {
            vec.push_back(cmd);
        }
    }

    void EventBus::publish(Topic topic) {
        auto it = subscribers.find(topic);
        if (it != subscribers.end()) {
            for (Command* cmd : it->second) {
                xQueueSend(cmd->topicQueue, (void*)topic, 100);
            }
        }
    }
}
