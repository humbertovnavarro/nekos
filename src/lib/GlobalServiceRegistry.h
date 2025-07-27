#pragma once

#include <vector>
#include "TaskService.h"

// Simple global registry for TaskService instances
class GlobalServiceRegistry {
public:
    // Add a service to the registry
    static void addService(TaskService* svc);

    // Get all registered services
    static const std::vector<TaskService*>& getAllServices();

private:
    // Access the singleton vector storing services
    static std::vector<TaskService*>& getServices();
};

#define REGISTER_TASK_SERVICE(SERVICE_NAME, STACK_SIZE, PRIORITY, CORE_ID) \
    void SERVICE_NAME##_setup(void* service);                                         \
    void SERVICE_NAME##_loop(void* service);                                          \
    TaskService SERVICE_NAME##_service(#SERVICE_NAME, STACK_SIZE, PRIORITY, CORE_ID,  \
                                      SERVICE_NAME##_setup, SERVICE_NAME##_loop);     \
    __attribute__((constructor)) static void register_##SERVICE_NAME() {              \
        GlobalServiceRegistry::addService(&SERVICE_NAME##_service);                   \
    }
