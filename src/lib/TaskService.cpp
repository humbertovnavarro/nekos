#include "TaskService.h"
#include "Console.h"
extern "C" void TaskHandler(void* pvparams)
{
    TaskService* service = static_cast<TaskService*>(pvparams);
    if (service && service->setup && service->loop) {
        service->setup(service);
        for (;;) {
            service->loop(service);
        }
    } else {
        Console::log("TaskHandler received invalid service pointer!\n");
        vTaskDelete(nullptr);  // Terminate task
    }
}

TaskService::TaskService(const char* service_id,
                         uint32_t stack_depth,
                         UBaseType_t priority,
                         BaseType_t core_id,
                         TaskFunction_t setup,
                         TaskFunction_t loop)
    : setup(setup),
      loop(loop),
      service_id(service_id),
      stack_depth(stack_depth),
      priority(priority),
      core_id(core_id),
      status(pdFAIL),
      service_handle(nullptr)
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
        Console::log("Failed to start: ");
        Console::log(service_id);
        Console::log("\n");
    }
}

TaskService::~TaskService()
{
    terminate();
}

void TaskService::suspend()
{
    if (service_handle) {
        vTaskSuspend(service_handle);
    }
}

void TaskService::resume()
{
    if (service_handle) {
        vTaskResume(service_handle);
    }
}

void TaskService::terminate()
{
    if (service_handle) {
        vTaskDelete(service_handle);
        service_handle = nullptr;
    }
}

BaseType_t TaskService::getStatus() const
{
    return status;
}

TaskHandle_t TaskService::getHandle() const
{
    return service_handle;
}

UBaseType_t TaskService::getStackHighWaterMark() const
{
    return service_handle ? uxTaskGetStackHighWaterMark(service_handle) : 0;
}

void TaskService::setPriority(UBaseType_t new_priority)
{
    if (service_handle) {
        vTaskPrioritySet(service_handle, new_priority);
    }
}

UBaseType_t TaskService::getPriority() const
{
    return service_handle ? uxTaskPriorityGet(service_handle) : 0;
}

bool TaskService::isSuspended() const
{
    return service_handle && eTaskGetState(service_handle) == eSuspended;
}

bool TaskService::isRunning() const
{
    if (!service_handle) return false;
    eTaskState state = eTaskGetState(service_handle);
    return state == eRunning || state == eReady || state == eBlocked;
}
