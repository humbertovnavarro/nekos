#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"

#define CREATE_TASK(TASK_NAME, SETUP_FN, LOOP_FN, STACK_SIZE, PRIORITY, CORE_ID)        \
    extern "C" void TASK_NAME##_handler(void* pvParams) {                                \
        (void)pvParams;                                                                    \
        SETUP_FN(pvParams);                                                                        \
        for (;;) {                                                                         \
            LOOP_FN(pvParams);                                                                     \
        }                                                                                  \
    }                                                                                      \
    static TaskHandle_t TASK_NAME##_handle = nullptr;                                      \
    static esp_err_t TASK_NAME##_status = xTaskCreatePinnedToCore(                         \
        TASK_NAME##_handler, #TASK_NAME, STACK_SIZE, nullptr, PRIORITY,                    \
        &TASK_NAME##_handle, CORE_ID)
