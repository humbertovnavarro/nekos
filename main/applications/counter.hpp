// counter.hpp
#pragma once
#include "app.hpp"
#include "misc/lv_types.h"

namespace nekos::app::counter {

struct State {
    lv_obj_t* root  = nullptr;
    lv_obj_t* label = nullptr;
    int   count = 0;
};

extern App<State> app;

} // namespace nekos::app::counter