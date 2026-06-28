// counter.hpp
#pragma once
#include "app.hpp"
#include "misc/lv_types.h"

namespace nekos::app::counter {

struct CounterState {
    lv_obj_t* label = nullptr;
    int   count = 0;
};

extern App<CounterState> app;

} // namespace nekos::app::counter