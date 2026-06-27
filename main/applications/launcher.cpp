#include "launcher.hpp"
#include "app.hpp"
#include "applications/counter.hpp"
#include "core/lv_obj.h"
#include "display/lv_display.h"
#include "esp32_s3_touch_amoled_2_06.h"
#include "etl/vector.h"
#include "freertos/idf_additions.h"
#include "lv_api_map_v8.h"
#include "portmacro.h"
#include <cstddef>
#include <utility>

namespace nekos::app::launcher {

struct HandoffCtx {
    nekos::App<State>* launcher;
    AppBase*        target;
};

static void on_button_press(lv_event_t* e) {
    auto* pressed= static_cast<nekos::AppBase*>(lv_event_get_user_data(e));
    app.references.pending = pressed;
}

static lv_obj_t* draw_button(lv_obj_t* parent, nekos::AppBase* a) {
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

    lv_obj_t* name = lv_label_create(btn);
    lv_label_set_text(name, a->name);
    lv_obj_set_style_text_color(name, lv_palette_lighten(LV_PALETTE_GREY, 1), 0);
    lv_obj_set_style_text_font(name, &lv_font_montserrat_14, 0);
    lv_obj_align(name, LV_ALIGN_CENTER, 0, 40);

    return btn;
}

static void draw(nekos::App<State>* self) {
    State& s = self->references;

    lv_obj_set_style_bg_color(s.root, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(s.root, LV_OPA_COVER, 0);

    lv_obj_t* launcher = lv_obj_create(s.root);
    lv_obj_remove_style_all(launcher);
    lv_obj_set_size(launcher, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_opa(launcher, LV_OPA_TRANSP, 0);

    lv_obj_t* app_list = lv_obj_create(launcher);
    lv_obj_remove_style_all(app_list);
    lv_obj_set_size(app_list, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_opa(app_list, LV_OPA_TRANSP, 0);
    lv_obj_set_flex_flow(app_list, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(app_list, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scroll_dir(app_list, LV_DIR_HOR);
    lv_obj_set_scrollbar_mode(app_list, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scroll_snap_x(app_list, LV_SCROLL_SNAP_CENTER);
    lv_obj_set_style_pad_left(app_list, 32, 0);
    lv_obj_set_style_pad_right(app_list, 32, 0);
    lv_obj_set_style_pad_top(app_list, 24, 0);
    lv_obj_set_style_pad_bottom(app_list, 24, 0);
    lv_obj_set_style_pad_column(app_list, 20, 0);

    for (auto* a : *s.apps) {
        draw_button(app_list, a);
    }
}

static void handoff_task(void* arg) {
    auto* ctx = static_cast<HandoffCtx*>(arg);
    
    ctx->launcher->stop();

    while(ctx->launcher->handle() != nullptr) {
        vTaskDelay(1);
    }

    if(ctx->target->launch()) {
        delete ctx;
        xSemaphoreTake(app.references.semphr, portMAX_DELAY);
        app.references.running = ctx->launcher;
        xSemaphoreGive(app.references.semphr);
    } else {
        delete ctx;
        ctx->launcher->launch();
    }
    vTaskDelete(nullptr);
}

App<State> app = App<State>::create({
    .references = {},
    .name       = "launcher",
    .icon       = "",
    .fn         = [](nekos::App<State>* self) {
        bsp_display_lock(portMAX_DELAY);
        draw(self);
        lv_screen_load(self->references.root);
        bsp_display_unlock();

        while (1) {
            if (self->references.pending != nullptr) {
                AppBase* target         = self->references.pending;
                self->references.pending   = nullptr;
                auto* ctx = new HandoffCtx{ self, target };
                xTaskCreate(handoff_task, "handoff", 2048, ctx, configMAX_PRIORITIES - 1, nullptr);
                return;
            }
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    },
    .allocater  = [](nekos::App<State>* self) {
        if(self->references.semphr == nullptr) {
            self->references.semphr = xSemaphoreCreateMutex();
        } else {
            xSemaphoreTake(self->references.semphr, portMAX_DELAY);
        }

        self->references.root    = lv_obj_create(NULL);
        self->references.apps    = new etl::vector<AppBase*, 32>();
        self->references.pending = nullptr;
        self->references.apps->push_back((App<void*>*)&nekos::app::counter::app);
        xSemaphoreGive(self->references.semphr);
    },
    .deleter    = [](nekos::App<State>* self) {
        xSemaphoreTake(self->references.semphr, portMAX_DELAY);
        bsp_display_lock(portMAX_DELAY);
        lv_obj_del(self->references.root);
        bsp_display_unlock();
        delete self->references.apps;
        self->references.apps = nullptr;
        xSemaphoreGive(self->references.semphr);
    },
});


} // namespace nekos::app::launcher