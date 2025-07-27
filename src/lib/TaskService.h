#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stddef.h>  // for size_t
#include "esp_err.h"
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

#define REGISTER_TASK_SERVICE(SERVICE_NAME, STACK_SIZE, PRIORITY, CORE_ID)                  \
    void SERVICE_NAME##_setup(TaskService* service);                                        \
    void SERVICE_NAME##_loop(TaskService* service);                                         \
                                                                                           \
    static void SERVICE_NAME##_setup_wrapper(void* service) {                               \
        SERVICE_NAME##_setup(static_cast<TaskService*>(service));                           \
    }                                                                                       \
    static void SERVICE_NAME##_loop_wrapper(void* service) {                                \
        SERVICE_NAME##_loop(static_cast<TaskService*>(service));                            \
    }                                                                                       \
                                                                                           \
    TaskService SERVICE_NAME##_service(                                                     \
        #SERVICE_NAME, STACK_SIZE, PRIORITY, CORE_ID,                                       \
        (TaskFunction_t)(SERVICE_NAME##_setup_wrapper),                                     \
        (TaskFunction_t)(SERVICE_NAME##_loop_wrapper));                                     \
                                                                                           \
    __attribute__((constructor)) static void register_##SERVICE_NAME() {                    \
        GlobalServiceRegistry::addService(&SERVICE_NAME##_service);                         \
    }