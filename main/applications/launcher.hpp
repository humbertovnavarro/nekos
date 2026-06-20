#include "esp32_s3_touch_amoled_2_06.h"
#include "application_types.hpp"
#include "etl/vector.h"

namespace launcher {
    lv_obj_t* draw_component(lv_obj_t* root, etl::vector<TaskLaunchOptions*, 32>* apps);
    extern TaskLaunchOptions launcher_task_opts;
    extern TaskLaunchOptions* active;
}