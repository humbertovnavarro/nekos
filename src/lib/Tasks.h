#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include <functional>
#include <string>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stddef.h> 

using TaskFn = std::function<void()>;
using StarterFn = std::function<void()>;
StarterFn makeTask(const std::string& name, int priority, int stack_size, TaskFn setup, TaskFn loop);

#define START_TASK(NAME, PRIORITY, STACK)                                \
    do {                                                                 \
        extern void NAME##_setup();                                      \
        extern void NAME##_loop();                                       \
        auto start_##NAME = makeTask(#NAME, PRIORITY, STACK,            \
                                     NAME##_setup, NAME##_loop);         \
        start_##NAME();                                                 \
    } while (0)

#define STOP_TASK(NAME)                     \
    do {                                    \
        if (NAME##_task_control.handle &&   \
            *NAME##_task_control.handle) {  \
            vTaskDelete(*NAME##_task_control.handle); \
            *NAME##_task_control.handle = nullptr;   \
        }                                   \
    } while (0)