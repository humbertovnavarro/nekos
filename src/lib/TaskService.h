// TaskService.h

#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern "C" void TaskHandler(void* pvparams);

class TaskService {
public:
    TaskFunction_t setup;
    TaskFunction_t loop;

    TaskService(const char* service_id, uint32_t stack_depth, UBaseType_t priority, BaseType_t core_id, TaskFunction_t setup, TaskFunction_t loop);
    ~TaskService();
    void suspend();
    void resume();
    void terminate();

    BaseType_t getStatus() const;
    TaskHandle_t getHandle() const;
    UBaseType_t getStackHighWaterMark() const;

    void setPriority(UBaseType_t new_priority);
    UBaseType_t getPriority() const;

    bool isSuspended() const;
    bool isRunning() const;

private:
    BaseType_t status;
    TaskHandle_t service_handle;
    const char* service_id;
    uint32_t stack_depth;
    UBaseType_t priority;
    BaseType_t core_id;
};