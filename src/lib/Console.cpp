#include "Console.h"
#include <cstdarg>
#include <cstdio>
#include <cstring>

QueueHandle_t Console::outputQueues[MAX_LOG_QUEUES] = { nullptr };
SemaphoreHandle_t Console::mutex = nullptr;
QueueHandle_t Console::ttyQueue = nullptr;
QueueHandle_t Console::shellCommandQueue = nullptr;
Console::Command Console::commands[MAX_COMMANDS] = {};
Console::Command* Console::commandListHead = nullptr;
int Console::commandCount = 0;

int Console::getCommandCount() {
    return commandCount;
}

const char* Console::getCommandName(int index) {
    if (index < 0 || index >= commandCount) return nullptr;
    Command* current = commandListHead;
    int i = 0;
    while (current && i < index) {
        current = current->next;
        i++;
    }
    return current ? current->name : nullptr;
}


bool Console::registerCommand(const char* name, CommandCallback cb) {
    // Allocate new command node
    Command* cmd = (Command*)malloc(sizeof(Command));
    if (!cmd) return false; // malloc failed

    // Copy name safely
    strncpy(cmd->name, name, sizeof(cmd->name) - 1);
    cmd->name[sizeof(cmd->name) - 1] = '\0';

    cmd->callback = cb;
    cmd->next = nullptr;

    // Append to list
    if (commandListHead == nullptr) {
        commandListHead = cmd;
    } else {
        Command* tail = commandListHead;
        while (tail->next) tail = tail->next;
        tail->next = cmd;
    }

    commandCount++;
    return true;
}

void Console::shellCommandProcessorTask(void* param) {
    char cmdBuffer[SHELL_INPUT_BUFFER_SIZE];
    QueueHandle_t cmdQueue = Console::shellCommandQueue;
    while (true) {
        if (xQueueReceive(cmdQueue, cmdBuffer, portMAX_DELAY) == pdTRUE) {
            // Parse command and args
            char* args = nullptr;
            char* cmdName = strtok_r(cmdBuffer, " ", &args);

            if (!cmdName) {
                log("Empty command received");
                continue;
            }

            bool handled = false;
            Command* current = commandListHead;
            while (current) {
                if (strcmp(cmdName, current->name) == 0) {
                    if (current->callback) {
                        current->callback(args);
                        handled = true;
                        break;
                    }
                }
                current = current->next;
            }

            if (!handled) {
                logf("Unknown command: %s", cmdName);
            }
        }
    }
}

void Console::init(bool enableShell) {
    Serial.begin(115200);
    mutex = xSemaphoreCreateMutex();
    while (!Serial) {
        delay(10); // Wait for serial ready on some boards
    }

    ttyQueue = xQueueCreate(QUEUE_LENGTH, QUEUE_MSG_SIZE);
    xTaskCreate(taskOutputTTY, "ConsoleTTY", 2048, NULL, 2, NULL);

    if (enableShell) {
        shellCommandQueue = xQueueCreate(QUEUE_LENGTH, SHELL_INPUT_BUFFER_SIZE);
        xTaskCreate(taskShellInput, "ConsoleShellIn", 4096, NULL, 3, NULL);
        xTaskCreate(shellCommandProcessorTask, "ShellCmdProc", 4096, NULL, 1, NULL);
    }
}

QueueHandle_t Console::registerConsoleMessageQueueHandle() {
    xSemaphoreTake(mutex, portMAX_DELAY);
    for (int i = 0; i < MAX_LOG_QUEUES; i++) {
        if (outputQueues[i] == nullptr) {
            outputQueues[i] = xQueueCreate(QUEUE_LENGTH, QUEUE_MSG_SIZE);
            xSemaphoreGive(mutex);
            return outputQueues[i];
        }
    }
    xSemaphoreGive(mutex);
    return nullptr;
}

void Console::log(const char *message) {
    if (!message) return;
    logf("%s", message);
}

void Console::logf(const char *fmt, ...) {
    if (!fmt) return;
    char buffer[QUEUE_MSG_SIZE];

    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    xSemaphoreTake(mutex, portMAX_DELAY);
    for (int i = 0; i < MAX_LOG_QUEUES; i++) {
        if (outputQueues[i]) {
            xQueueSendToBack(outputQueues[i], buffer, 0);
        }
    }
    if (ttyQueue) {
        xQueueSendToBack(ttyQueue, buffer, 0);
    }
    xSemaphoreGive(mutex);
}

void Console::logfLevel(const char *level, const char *fmt, ...) {
    if (!fmt) return;
    char buffer[QUEUE_MSG_SIZE];

    if (level) {
        int written = snprintf(buffer, sizeof(buffer), "[%s] ", level);
        if (written < 0 || written >= (int)sizeof(buffer)) {
            buffer[sizeof(buffer) - 1] = '\0';
            logf("%s", buffer);
            return;
        }

        va_list args;
        va_start(args, fmt);
        vsnprintf(buffer + written, sizeof(buffer) - written, fmt, args);
        va_end(args);
    } else {
        va_list args;
        va_start(args, fmt);
        vsnprintf(buffer, sizeof(buffer), fmt, args);
        va_end(args);
    }

    logf("%s", buffer);
}

void Console::taskOutputTTY(void *param) {
    char buffer[QUEUE_MSG_SIZE];
    while (true) {
        if (xQueueReceive(ttyQueue, buffer, portMAX_DELAY) == pdTRUE) {
            Serial.print(buffer);
            Serial.print("\r\n");
        }
    }
}

void Console::taskShellInput(void *param) {
    static char lineBuffer[SHELL_INPUT_BUFFER_SIZE];
    size_t index = 0;

    while (true) {
        if (Serial.available() > 0) {
            char byte = (char)Serial.read();

            if (byte == '\r' || byte == '\n') {
                if (index > 0) {
                    lineBuffer[index] = '\0';
                    if (shellCommandQueue) {
                        xQueueSendToBack(shellCommandQueue, lineBuffer, 0);
                    }
                    index = 0;
                    Console::logf("> %s", lineBuffer); // Echo command
                }
                Serial.print("\r\n");
            } else if (byte == '\b' || byte == 0x7F) {
                if (index > 0) {
                    index--;
                    Serial.print("\b \b");
                }
            } else if (index < (SHELL_INPUT_BUFFER_SIZE - 1)) {
                lineBuffer[index++] = byte;
                Serial.write(byte);
            }
        } else {
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
    }
}
