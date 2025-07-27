#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/message_buffer.h"
#include "freertos/task.h"
#include "Arduino.h"

class Console {
public:
    using ConsoleCallback = void (*)(const char* msg);

    static void init(size_t buffer_size = 1024);
    static void shutdown();
    static bool log(const char* message);
    static void addCallback(ConsoleCallback cb);

private:
    static MessageBufferHandle_t msgBuffer;
    static TaskHandle_t receiverTaskHandle;
    static size_t bufferSize;
    static bool isRunning;

    static void receiverTask(void* pvParameters);

    static ConsoleCallback callbacks[8];  // Max 8 callbacks
    static int callbackCount;

    Console() = delete;  // Disallow instantiation
};
