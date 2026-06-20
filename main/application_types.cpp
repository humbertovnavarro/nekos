#include "application_types.hpp"

bool create_task(TaskLaunchOptions* options) {
    BaseType_t result = xTaskCreate(
        options->task_fn,
        options->name,
        options->task_stack_depth,
        options,
        options->task_priority,
        &options->task
    );
    return result == pdPASS;
}