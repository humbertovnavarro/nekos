// launcher.hpp
#pragma once
#include "app.hpp"
#include "etl/vector.h"
#include "core/lv_obj.h"
#include "freertos/idf_additions.h"

namespace nekos::app::launcher {

struct State {
    SemaphoreHandle_t semphr = nullptr;
    lv_obj_t*                         root    = nullptr;
    lv_obj_t*                         app_list = nullptr;
    lv_obj_t*                         launcher = nullptr;
    etl::vector<nekos::AppBase*, 32>* apps    = nullptr;
    nekos::AppBase*                   pending = nullptr;
    nekos::AppBase*                   running = nullptr;
};

bool register_app(nekos::AppBase* app);

extern nekos::App<State> app;



} // namespace nekos::app::launcher