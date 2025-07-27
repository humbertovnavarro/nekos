#include "freertos/projdefs.h"
#include "esp_log.h"
#include "TaskService.h"

extern "C" void TaskHandler(void* pvparams) {
    TaskService* service = static_cast<TaskService*>(pvparams);
    if (service && service->setup && service->loop) {
        service->setup(service);
        for (;;) {
            service->loop(service);
        }
    }
}

TaskService::TaskService(const char* service_id,
                         uint32_t stack_depth,
                         UBaseType_t priority,
                         BaseType_t core_id,
                         TaskFunction_t setup,
                         TaskFunction_t loop)
    : service_id(service_id),
      stack_depth(stack_depth),
      priority(priority),
      core_id(core_id),
      setup(setup),
      loop(loop),
      service_handle(nullptr),
      status(pdFAIL)
{
    status = xTaskCreatePinnedToCore(
        TaskHandler,
        service_id,
        stack_depth,
        this,
        priority,
        &service_handle,
        core_id
    );

    if (status != pdPASS) {
        ESP_LOGE("TaskService", "Failed to create service task: %s", service_id);
    }
}

TaskService::~TaskService() {
    terminate();
}

void TaskService::suspend() {
    if (service_handle) {
        vTaskSuspend(service_handle);
    }
}

void TaskService::resume() {
    if (service_handle) {
        vTaskResume(service_handle);
    }
}

void TaskService::terminate() {
    if (service_handle) {
        vTaskDelete(service_handle);
        service_handle = nullptr;
    }
}

BaseType_t TaskService::getStatus() const {
    return status;
}

TaskHandle_t TaskService::getHandle() const {
    return service_handle;
}

UBaseType_t TaskService::getStackHighWaterMark() const {
    if (service_handle) {
        return uxTaskGetStackHighWaterMark(service_handle);
    }
    return 0;
}

void TaskService::setPriority(UBaseType_t new_priority) {
    if (service_handle) {
        vTaskPrioritySet(service_handle, new_priority);
        priority = new_priority;
    }
}

UBaseType_t TaskService::getPriority() const {
    return priority;
}

bool TaskService::isSuspended() const {
    if (!service_handle) return false;
    eTaskState state = eTaskGetState(service_handle);
    return state == eSuspended;
}

bool TaskService::isRunning() const {
    if (!service_handle) return false;
    eTaskState state = eTaskGetState(service_handle);
    return state == eRunning;
}

void GlobalServiceRegistry::addService(TaskService* svc) {
    if (service_count < MAX_SERVICES) {
        services[service_count++] = svc;
    } else {
        ESP_LOGE("ServiceRegistry", "Max service limit reached (%zu)", MAX_SERVICES);
    }
}

TaskService* const* GlobalServiceRegistry::getAllServices() {
    return services;
}

size_t GlobalServiceRegistry::getServiceCount() {
    return service_count;
}

TaskService* GlobalServiceRegistry::services[GlobalServiceRegistry::MAX_SERVICES] = {};
size_t GlobalServiceRegistry::service_count = 0;