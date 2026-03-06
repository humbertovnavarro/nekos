#pragma once
#include "input.hpp"
#include "task.h"

extern TaskHandle_t inputTaskHandle;

namespace input {
inline static void task_fn(void *) {
    while (1) { Input::pollInputs(); vTaskDelay(1); }
}

} // namespace input