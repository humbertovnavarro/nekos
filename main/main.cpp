#include <Arduino.h>
#include "freertos/idf_additions.h"
#include "gui_task.hpp"
#include "lua_cli.hpp"
#include "lua_runner.hpp"
#include "lua_state_pool.cpp"
#include "lua_utils.h"
#include "soc/gpio_num.h"
#include "input.hpp"
#include "ssd1306.h"
#include "input_task.hpp"
#include "stdin.hpp"

Input button(GPIO_NUM_0);
TaskHandle_t cliTaskHandle   = nullptr;
TaskHandle_t guiTaskHandle   = nullptr;
TaskHandle_t inputTaskHandle = nullptr;

void setup()
{
    Serial.begin(962100);
    initArduino();
    button.begin();
    oled::init();
    oled::flush();
    lua_fs_mount(true);
    lua_utils::install(true);
    lua_pool_init();
    lua_init();
    
    StdinArgPool::init();

    xTaskCreate(
        cli::task_fn,
        "cli",
        4096,
        nullptr,
        2,
        &cliTaskHandle
    );

    xTaskCreate(
        gui::task_fn,
        "gui",
        4096,
        nullptr,
        1,
        &guiTaskHandle
    );

    xTaskCreate(
        input::task_fn,
        "input",
        2048,
        nullptr,
        3,
        &inputTaskHandle
    );
}

extern "C" void app_main(void)
{
    setup();
}