#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include <string.h>
#include <stdint.h>

class Console {
public:
    static constexpr size_t MAX_MESSAGE_LENGTH = 64;
    static constexpr size_t QUEUE_LENGTH = 8;
    static constexpr size_t MAX_LOG_QUEUES = 16;

    // Initialize if needed (not currently doing anything)
    static void init();

    // Log a message to all registered queues
    static void log(const char* message);

    // Register a new message queue and get a handle to it
    static QueueHandle_t registerConsoleMessageQueueHandle();

private:
    static QueueHandle_t outputQueues[MAX_LOG_QUEUES];
};
