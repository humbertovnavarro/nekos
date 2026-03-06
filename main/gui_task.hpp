#pragma once
#include "input.hpp"
#include "menu.h"
#include "task.h"

extern TaskHandle_t guiTaskHandle;

namespace gui {
inline oled::MenuItem root_menu_items[] = {
    { "Wifi",      nullptr, nullptr },
    { "Bluetooth", nullptr,                                nullptr },
};

inline Input* pager_button;

inline oled::Menu root_menu(root_menu_items, 2);

inline static void task_fn(void *) {
    while (1) {
        bool dirty = false;
        if (Input::get_pressed(0)) { root_menu.down();    dirty = true; }
        if (Input::get_held(0))  { root_menu.confirm(); dirty = true; }
        if (dirty) { oled::clear(); root_menu.draw(); oled::flush(); }
        vTaskDelay(1);
    }
}

} // namespace gui