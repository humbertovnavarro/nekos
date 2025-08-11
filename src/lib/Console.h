#pragma once
#include <Arduino.h>  // Arduino API
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include <cstdint>
#include <cstddef>

#define MAX_LOG_QUEUES     16
#define QUEUE_MSG_SIZE     64
#define QUEUE_LENGTH       16
#define SHELL_INPUT_BUFFER_SIZE 64
#define SHELL_MAX_COMMANDS 64

using CommandCallback = std::function<void(const char* args)>;

class Console {
public:
    static void log(const char *message);
    static void logf(const char *fmt, ...);
    static void logfLevel(const char *level, const char *fmt, ...);
    static void init(bool enableShell = false);
    static bool registerCommand(const char* name, CommandCallback cb);
    static int getCommandCount();
    static const char* getCommandName(int index);
private:
    static constexpr int MAX_COMMANDS = 64;
    static QueueHandle_t getShellCommandQueue();
    struct Command {
        char name[16];
        CommandCallback callback;
        Command* next;
    };
    static Command* commandListHead;
    static int commandCount;
    static Command commands[MAX_COMMANDS];
    static void shellCommandProcessorTask(void* param);
    static QueueHandle_t outputQueues[MAX_LOG_QUEUES];
    static SemaphoreHandle_t mutex;
    static QueueHandle_t ttyQueue;
    static QueueHandle_t shellCommandQueue;
    static void taskOutputTTY(void *param);
    static void taskShellInput(void *param);
    static QueueHandle_t registerConsoleMessageQueueHandle();
};
