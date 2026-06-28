#include "launcher.hpp"
#include "app.hpp"
#include "applications/counter.hpp"
#include "core/lv_obj.h"
#include "display/lv_display.h"
#include "esp32_s3_touch_amoled_2_06.h"
#include "etl/vector.h"
#include "font/lv_symbol_def.h"
#include "freertos/idf_additions.h"
#include "portmacro.h"

namespace nekos::app::launcher {

struct HandoffCtx {
    nekos::App<State>* launcher;
    AppBase*           target;
};

static void on_button_press(lv_event_t* e) {
    auto* pressed = static_cast<nekos::AppBase*>(lv_event_get_user_data(e));
    if(app.references.pending != nullptr) {
        return;
    }
    xSemaphoreTake(app.references.semphr, portMAX_DELAY);
    app.references.pending = pressed;
    xSemaphoreGive(app.references.semphr);
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

    lv_obj_t* label = lv_label_create(btn);
    lv_label_set_text(label, a->name);
    lv_obj_set_style_text_color(label, lv_palette_lighten(LV_PALETTE_GREY, 1), 0);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 40);

    return btn;
}

static void draw_app_list(nekos::App<State>* self) {
    State& s = self->references;

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

static void draw(nekos::App<State>* self) {
    State& s = self->references;

    lv_obj_set_style_bg_color(s.root, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(s.root, LV_OPA_COVER, 0);

    lv_obj_remove_style_all(s.launcher);
    lv_obj_set_size(s.launcher, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_opa(s.launcher, LV_OPA_TRANSP, 0);

    draw_app_list(self);
}

static void handoff_task(void* arg) {
    auto* ctx = static_cast<HandoffCtx*>(arg);

    ctx->launcher->stop();

    if (!ctx->target->launch()) {
        ctx->launcher->launch();
    }

    delete ctx;
    vTaskDelete(nullptr);
}

App<State> app = App<State>::create({
    .name      = "launcher",
    .icon      = LV_SYMBOL_EYE_OPEN,
    .fn        = [](nekos::App<State>* self) {
        bsp_display_lock(portMAX_DELAY);
        draw(self);
        lv_screen_load(self->references.root);
        bsp_display_unlock();

        while (true) {
            xSemaphoreTake(self->references.semphr, portMAX_DELAY);
            AppBase* pending = self->references.pending;
            self->references.pending = nullptr;
            xSemaphoreGive(self->references.semphr);

            if (pending != nullptr) {
                auto* ctx = new HandoffCtx{ self, pending };
                xTaskCreate(handoff_task, "handoff", 2048, ctx, configMAX_PRIORITIES - 1, nullptr);
                return;
            }

            vTaskDelay(pdMS_TO_TICKS(100));
        }
    },
    .allocater = [](nekos::App<State>* self) {
        State& s = self->references;

        if (s.semphr == nullptr) {
            s.semphr = xSemaphoreCreateMutex();
        }

        xSemaphoreTake(s.semphr, portMAX_DELAY);

        if (s.apps == nullptr) {
            s.apps = new etl::vector<AppBase*, 32>();
            s.apps->push_back(static_cast<AppBase*>(&nekos::app::counter::app));
        }

        s.root     = lv_obj_create(nullptr);
        s.launcher = lv_obj_create(s.root);
        s.app_list = lv_obj_create(s.launcher);
        s.pending  = nullptr;

        xSemaphoreGive(s.semphr);
    },
    .deleter   = [](nekos::App<State>* self) {
        State& s = self->references;

        xSemaphoreTake(s.semphr, portMAX_DELAY);

        bsp_display_lock(portMAX_DELAY);
        lv_obj_delete(s.root);
        s.root     = nullptr;
        s.launcher = nullptr;
        s.app_list = nullptr;
        bsp_display_unlock();

        delete s.apps;
        s.apps = nullptr;

        xSemaphoreGive(s.semphr);
    },
});

bool register_app(AppBase* a) {
    if (app.references.semphr == nullptr) {
        app.references.semphr = xSemaphoreCreateMutex();
    }

    if (app.references.apps == nullptr) {
        app.references.apps = new etl::vector<AppBase*, 32>();
    }

    xSemaphoreTake(app.references.semphr, portMAX_DELAY);
    app.references.apps->push_back(a);
    xSemaphoreGive(app.references.semphr);

    if (app.running()) {
        bsp_display_lock(portMAX_DELAY);
        draw_app_list(&app);
        bsp_display_unlock();
    }

    return true;
}

} // namespace nekos::app::launcher