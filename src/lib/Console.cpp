#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include <string.h>
#include <stdint.h>
#include "Console.h"
#define MAX_LOG_QUEUES 16

QueueHandle_t Console::registerConsoleMessageQueueHandle() {
    for(int i = 0; i < MAX_LOG_QUEUES; i++) {
        if(Console::outputQueues[i] == nullptr) {
            Console::outputQueues[i] == xQueueCreate(QUEUE_LENGTH, sizeof(char) * 32);
            return Console::outputQueues[i];
        }
    }
    return nullptr;
}

void Console::log(const char* message) {
    char buffer[MAX_MESSAGE_LENGTH];
    strncpy(buffer, message, MAX_MESSAGE_LENGTH);
    buffer[MAX_MESSAGE_LENGTH - 1] = '\0';
    for(int i = 0; i < MAX_LOG_QUEUES; i++) {
        if(Console::outputQueues[i] == nullptr) {
            break;
        }
        xQueueSendToBack(Console::outputQueues[i], message, pdMS_TO_TICKS(100));
    }
}

QueueHandle_t Console::outputQueues[MAX_LOG_QUEUES];