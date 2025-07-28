#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "Tasks.h"
#define MAX_REGISTERED_TASKS 32

static RegisteredTask g_registered_tasks[MAX_REGISTERED_TASKS];
static size_t g_registered_task_count = 0;

void register_task(const char* name, TaskHandle_t handle) {
    if (g_registered_task_count < MAX_REGISTERED_TASKS) {
        g_registered_tasks[g_registered_task_count++] = { name, handle };
    }
}

size_t get_registered_task_count() {
    return g_registered_task_count;
}

const RegisteredTask* get_registered_task(size_t index) {
    if (index >= g_registered_task_count) return nullptr;
    return &g_registered_tasks[index];
}
