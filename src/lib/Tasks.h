#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stddef.h>  // for size_t

// Maximum number of tasks that can be registered
#define MAX_REGISTERED_TASKS 32

// Struct representing a registered task
struct RegisteredTask {
    const char* name;
    TaskHandle_t handle;
};

// Registers a task with a given name and handle
void register_task(const char* name, TaskHandle_t handle);

// Returns the number of currently registered tasks
size_t get_registered_task_count();

// Retrieves a pointer to the RegisteredTask at the given index
const RegisteredTask* get_registered_task(size_t index);

#define CREATE_TASK(TASK_NAME, SETUP_FN, LOOP_FN, STACK_SIZE, PRIORITY, CORE_ID)        \
    extern "C" void TASK_NAME##_handler(void* pvParams) {                                \
        (void)pvParams;                                                                  \
        SETUP_FN(pvParams);                                                              \
        for (;;) {                                                                       \
            LOOP_FN(pvParams);                                                           \
        }                                                                                \
    }                                                                                    \
    static TaskHandle_t TASK_NAME##_handle = nullptr;                                    \
    static esp_err_t TASK_NAME##_status = xTaskCreatePinnedToCore(                       \
        TASK_NAME##_handler, #TASK_NAME, STACK_SIZE, nullptr, PRIORITY,                  \
        &TASK_NAME##_handle, CORE_ID);                                                   \
    static int TASK_NAME##_reg = (                                                       \
        register_task(#TASK_NAME, TASK_NAME##_handle), 0)
