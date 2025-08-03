#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <functional>
#include <string>

using TaskFn = std::function<void()>;
using StarterFn = std::function<void()>;

// Task wrapper function
static void task_entry(void* params) {
    auto* fn_pair = static_cast<std::pair<TaskFn, TaskFn>*>(params);
    fn_pair->first(); // call setup()
    while (true) {
        fn_pair->second(); // call loop()
    }
}

// Returns a function that starts a FreeRTOS task with the given setup and loop functions
StarterFn makeTask(const std::string& name, int priority, int stack_size, TaskFn setup, TaskFn loop) {
    return [=]() {
        auto* fn_pair = new std::pair<TaskFn, TaskFn>(setup, loop); // will live forever
        xTaskCreate(
            task_entry,
            name.c_str(),
            stack_size,
            fn_pair,
            priority,
            nullptr
        );
    };
}
