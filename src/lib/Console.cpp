#include "Console.h"

MessageBufferHandle_t Console::msgBuffer = nullptr;
TaskHandle_t Console::receiverTaskHandle = nullptr;
size_t Console::bufferSize = 0;
bool Console::isRunning = false;

Console::ConsoleCallback Console::callbacks[8] = { nullptr };
int Console::callbackCount = 0;

void Console::init(size_t bufSize) {
    if (isRunning) return;

    bufferSize = bufSize;
    msgBuffer = xMessageBufferCreate(bufferSize);

    if (!msgBuffer) {
        Serial.println("Failed to create Console message buffer!");
        return;
    }

    isRunning = true;
    xTaskCreatePinnedToCore(
        Console::receiverTask,
        "ConsoleReceiver",
        4096,
        nullptr,
        1,
        &receiverTaskHandle,
        1
    );
}

void Console::shutdown() {
    if (!isRunning) return;
    isRunning = false;

    if (receiverTaskHandle) {
        vTaskDelete(receiverTaskHandle);
        receiverTaskHandle = nullptr;
    }

    if (msgBuffer) {
        vMessageBufferDelete(msgBuffer);
        msgBuffer = nullptr;
    }

    callbackCount = 0;
}

bool Console::log(const char* message) {
    if (!isRunning || !msgBuffer) return false;

    size_t len = strlen(message) + 1;
    size_t sent = xMessageBufferSend(msgBuffer, message, len, 0);
    return sent == len;
}

void Console::addCallback(ConsoleCallback cb) {
    if (callbackCount < 8 && cb != nullptr) {
        callbacks[callbackCount++] = cb;
    }
}

void Console::receiverTask(void* /*unused*/) {
    char buffer[256];
    while (isRunning) {
        size_t received = xMessageBufferReceive(msgBuffer, buffer, sizeof(buffer), pdMS_TO_TICKS(100));
        if (received > 0) {
            Serial.println(buffer);  // Always print to Serial
            // Call registered callbacks
            for (int i = 0; i < callbackCount; ++i) {
                if (callbacks[i]) {
                    callbacks[i](buffer);
                }
            }
        }
    }
    vTaskDelete(nullptr);
}
