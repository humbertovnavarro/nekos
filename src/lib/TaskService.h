#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stddef.h>  // for size_t
#include "esp_err.h"
class TaskService;
typedef void (*TaskServiceHandler)(TaskService* service);
extern "C" void TaskHandler(void* pvparams);
class TaskService {
public:
    TaskServiceHandler setup;
    TaskServiceHandler loop;
    TaskService(const char* service_id, uint32_t stack_depth, UBaseType_t priority, TaskServiceHandler setup, TaskServiceHandler loop);
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

class GlobalServiceRegistry {
public:
    static constexpr size_t MAX_SERVICES = 32;

    static void addService(TaskService* svc);
    static TaskService* const* getAllServices();
    static size_t getServiceCount();

private:
    static TaskService* services[MAX_SERVICES];
    static size_t service_count;
};