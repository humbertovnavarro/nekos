#pragma once
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "portmacro.h"
#include <cstddef>
#include <cstdint>

typedef struct TaskLaunchOptions {
    const char* name = "";
    const char* icon = "";
    TaskHandle_t task = NULL;
    TaskFunction_t task_fn;
    size_t task_stack_depth = 4096;
    uint8_t task_priority = 3;
} TaskLaunchOptions;

bool create_task(TaskLaunchOptions* options);