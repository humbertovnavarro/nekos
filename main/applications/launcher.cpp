#include "launcher.hpp"
#include "application_types.hpp"
#include "core/lv_obj.h"
#include "esp32_s3_touch_amoled_2_06.h"
#include "etl/vector.h"

#include "applications/counter.hpp"
#include "freertos/idf_additions.h"
#include <cstddef>

namespace launcher {

TaskLaunchOptions* active = nullptr;

void launcher_button_component_press(lv_event_t* e) {
    auto* app_ctx = static_cast<TaskLaunchOptions*>(lv_event_get_user_data(e));
    vTaskDelete(launcher_task_opts.task);
    active = app_ctx;
    create_task(app_ctx);
}

lv_obj_t *draw_launcher_button_component(lv_obj_t *parent, TaskLaunchOptions* app) {
  lv_obj_t *btn = lv_button_create(parent);
  lv_obj_set_size(btn, 120, 120);
  lv_obj_set_style_bg_color(btn, lv_color_black(), 0);
  lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(btn, 1, 0);
  lv_obj_set_style_border_color(btn, lv_palette_darken(LV_PALETTE_GREY, 3), 0);
  lv_obj_set_style_radius(btn, 20, 0);
  lv_obj_set_style_shadow_width(btn, 0, 0);
  lv_obj_set_style_outline_width(btn, 0, 0);
  lv_obj_add_event_cb(btn, launcher_button_component_press, LV_EVENT_CLICKED, app);
  lv_obj_t *name = lv_label_create(btn);
  lv_label_set_text(name, app->name);
  lv_obj_set_style_text_color(name, lv_palette_lighten(LV_PALETTE_GREY, 1), 0);
  lv_obj_set_style_text_font(name, &lv_font_montserrat_14, 0);
  lv_obj_align(name, LV_ALIGN_CENTER, 0, 40);
  return btn;
}

lv_obj_t* draw_component(lv_obj_t* root, etl::vector<TaskLaunchOptions*, 32>* apps) {
  lv_obj_set_style_bg_color(root, lv_color_black(), 0);
  lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);
  lv_obj_t* launcher = lv_obj_create(root);
  lv_obj_remove_style_all(launcher);
  lv_obj_set_size(launcher, lv_pct(100), lv_pct(100));
  lv_obj_set_style_bg_opa(launcher, LV_OPA_TRANSP, 0);
  lv_obj_t *app_list = lv_obj_create(launcher);
  lv_obj_remove_style_all(app_list);
  lv_obj_set_size(app_list, lv_pct(100), lv_pct(100));
  lv_obj_set_style_bg_opa(app_list, LV_OPA_TRANSP, 0);
  lv_obj_set_flex_flow(app_list, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(app_list, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
  LV_FLEX_ALIGN_CENTER);
  lv_obj_set_scroll_dir(app_list, LV_DIR_HOR);
  lv_obj_set_scrollbar_mode(app_list, LV_SCROLLBAR_MODE_OFF);
  lv_obj_set_scroll_snap_x(app_list, LV_SCROLL_SNAP_CENTER);
  lv_obj_set_style_pad_left(app_list, 32, 0);
  lv_obj_set_style_pad_right(app_list, 32, 0);
  lv_obj_set_style_pad_top(app_list, 24, 0);
  lv_obj_set_style_pad_bottom(app_list, 24, 0);
  lv_obj_set_style_pad_column(app_list, 20, 0);
  
  for(auto app : *apps) {
    draw_launcher_button_component(app_list, app);
  }
  return root;
}

void launcher_task_fn(void* params) {
  etl::vector<TaskLaunchOptions*, 32> applications = etl::vector<TaskLaunchOptions*, 32>();
  applications.push_back(&counter_task_opts);
  bsp_display_lock(100);
  lv_obj_t* launcher_root = lv_obj_create(NULL);
  launcher::draw_component(launcher_root, &applications);
  lv_screen_load(launcher_root);
  bsp_display_unlock();

  while(1) {
    vTaskDelay(100);
  }
}

TaskLaunchOptions launcher_task_opts = {
  .name = "launcher",
  .icon = "🏠",
  .task = nullptr,
  .task_fn = launcher_task_fn,
  .task_stack_depth = 4096,
  .task_priority = 3
};

} // namespace launcher