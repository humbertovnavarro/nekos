// launcher.hpp
#pragma once
#include "app.hpp"
#include "etl/vector.h"
#include "core/lv_obj.h"
#include "freertos/idf_additions.h"

namespace nekos::app::launcher {

struct AppLauncherState {
    lv_obj_t*                         app_list = nullptr;
    lv_obj_t*                         launcher = nullptr;
    etl::vector<nekos::IApp*, 255>*   apps    = nullptr;
    nekos::IApp*                      pending = nullptr;
    nekos::IApp*                      running = nullptr;
    bool launching = false;
};

bool register_app(IApp* a, bool is_isr = false);

extern nekos::App<AppLauncherState> app;

} // namespace nekos::app::launcher