#include "launcher.hpp"
#include "app.hpp"
#include "applications/counter.hpp"
#include "core/lv_obj.h"
#include "core/lv_obj_event.h"
#include "display/lv_display.h"
#include "esp32_s3_touch_amoled_2_06.h"
#include "etl/vector.h"
#include "font/lv_symbol_def.h"
#include "freertos/idf_additions.h"
#include "lv_api_map_v8.h"
#include "portmacro.h"
#include <cstddef>
#include <cstdint>
#include "esp_log.h"

namespace nekos::app::launcher {

static void on_button_press(lv_event_t* e) {
    auto* pressed = static_cast<nekos::IApp*>(lv_event_get_user_data(e));
    
    app.state.lock();

    if(pressed == nullptr) {
        app.state.unlock();
        return;
    } else if (app.state.pending != nullptr) {
        app.state.unlock();
        return;
    } else {
        app.state.pending = pressed;
        app.state.unlock();
    }

}

static lv_obj_t* draw_button(lv_obj_t* parent, nekos::IApp* a) {
    lv_obj_t* btn = lv_button_create(parent);
    lv_obj_set_size(btn, 120, 120);
    lv_obj_set_style_bg_color(btn, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(btn, 1, 0);
    lv_obj_set_style_border_color(btn, lv_palette_darken(LV_PALETTE_GREY, 3), 0);
    lv_obj_set_style_radius(btn, 20, 0);
    lv_obj_set_style_shadow_width(btn, 0, 0);
    lv_obj_set_style_outline_width(btn, 0, 0);
    lv_obj_add_event_cb(btn, on_button_press, LV_EVENT_CLICKED, a);

    lv_obj_t* label = lv_label_create(btn);
    lv_label_set_text(label, a->name);
    lv_obj_set_style_text_color(label, lv_palette_lighten(LV_PALETTE_GREY, 1), 0);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 40);

    return btn;
}

static void draw_app_list(nekos::App<AppLauncherState>* self) {
    AppLauncherState& s = self->state;
    
    lv_obj_clean(s.app_list);
    lv_obj_remove_style_all(s.app_list);
    lv_obj_set_size(s.app_list, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_opa(s.app_list, LV_OPA_TRANSP, 0);
    lv_obj_set_flex_flow(s.app_list, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(s.app_list, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scroll_dir(s.app_list, LV_DIR_HOR);
    lv_obj_set_scrollbar_mode(s.app_list, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scroll_snap_x(s.app_list, LV_SCROLL_SNAP_CENTER);
    lv_obj_set_style_pad_left(s.app_list, 32, 0);
    lv_obj_set_style_pad_right(s.app_list, 32, 0);
    lv_obj_set_style_pad_top(s.app_list, 24, 0);
    lv_obj_set_style_pad_bottom(s.app_list, 24, 0);
    lv_obj_set_style_pad_column(s.app_list, 20, 0);

    for (auto* a : *s.apps) {
        draw_button(s.app_list, a);
    }
}

static void draw(nekos::App<AppLauncherState>* self) {
    lv_obj_set_style_bg_color(self->state.root, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(self->state.root, LV_OPA_COVER, 0);
    lv_obj_remove_style_all(self->state.launcher);
    lv_obj_set_size(self->state.launcher, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_opa(self->state.launcher, LV_OPA_TRANSP, 0);
    draw_app_list(self);
}

App<AppLauncherState> app = App<AppLauncherState>::create({
    .name      = "launcher",
    .icon      = LV_SYMBOL_EYE_OPEN,
    .fn        = [](nekos::App<AppLauncherState>* self) {
        bsp_display_lock(portMAX_DELAY);
        self->state.root = lv_obj_create(NULL);
        self->state.launcher = lv_obj_create(self->state.root);
        self->state.app_list = lv_obj_create(self->state.launcher);
        draw(self);
        lv_screen_load(self->state.root);
        bsp_display_unlock();

        while (true) {
            auto* pending = self->state.pending;
            if (pending != nullptr) {
                lv_obj_del(self->state.root);
                self->state.running = self->state.pending;
                self->state.pending->launch();
                vTaskDelete(nullptr);
            }
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    },
    .allocater = [](nekos::App<AppLauncherState>* self) {
        self->state.apps = new etl::vector<IApp*, 255>();
    },
    .deleter   = [](nekos::App<AppLauncherState>* self) {
    },
});

bool register_app(IApp* a, bool is_isr) {
    if (app.state.apps == nullptr || !app.state.allocated) {
        ESP_LOGE("launcher", "tried to register an app before allocating launcher. call nekos::app::launcher::malloc");
        return false;
    }
    app.state.lock(is_isr);
    app.state.apps->push_back(a);
    app.state.unlock(is_isr);
    return true;
}

} // namespace nekos::app::launcher