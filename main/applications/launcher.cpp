#include "launcher.hpp"
#include "app.hpp"
#include "core/lv_obj.h"
#include "core/lv_obj_event.h"
#include "display/lv_display.h"
#include "esp32_s3_touch_amoled_2_06.h"
#include "etl/vector.h"
#include "font/lv_symbol_def.h"
#include "freertos/idf_additions.h"
#include "portmacro.h"
#include "esp_log.h"
#include <cstddef>
#include <cstdint>

namespace nekos::app::launcher {

// ── UI ────────────────────────────────────────────────────────────────────────

static void on_button_press(lv_event_t* e) {
    auto* pressed = static_cast<nekos::IApp*>(lv_event_get_user_data(e));
    if (pressed == nullptr) return;

    app.state.lock();
    if (app.state.pending == nullptr) {
        app.state.pending = pressed;
    }
    app.state.unlock();
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

static void ui_build(nekos::App<AppLauncherState>* self) {
    bsp_display_lock(portMAX_DELAY);
    self->state.root     = lv_obj_create(NULL);
    self->state.launcher = lv_obj_create(self->state.root);
    self->state.app_list = lv_obj_create(self->state.launcher);

    lv_obj_set_style_bg_color(self->state.root, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(self->state.root, LV_OPA_COVER, 0);
    lv_obj_remove_style_all(self->state.launcher);
    lv_obj_set_size(self->state.launcher, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_opa(self->state.launcher, LV_OPA_TRANSP, 0);

    draw_app_list(self);
    lv_screen_load(self->state.root);
    bsp_display_unlock();
}

static void ui_destroy(nekos::App<AppLauncherState>* self) {
    bsp_display_lock(portMAX_DELAY);
    if (self->state.root != nullptr) {
        lv_obj_del(self->state.root);
        self->state.root     = nullptr;
        self->state.launcher = nullptr;
        self->state.app_list = nullptr;
    }
    bsp_display_unlock();
}

// ── App definition ────────────────────────────────────────────────────────────

App<AppLauncherState> app = App<AppLauncherState>::create({
    .name = "launcher",
    .icon = LV_SYMBOL_EYE_OPEN,
    .fn   = [](nekos::App<AppLauncherState>* self) {
        self->state.apps    = new etl::vector<IApp*, 255>();
        self->state.pending = nullptr;
        self->state.running = nullptr;

        ui_build(self);

        while (self->poll()) {

            if(self->signalled(AppSignal::StateDirty)) {
                bsp_display_lock(portMAX_DELAY);
                draw_app_list(self);
                bsp_display_unlock();
            }

            self->state.lock();
            IApp* pending       = self->state.pending;
            self->state.pending = nullptr;
            self->state.unlock();
            if (pending == nullptr) continue;

            ui_destroy(self);

            self->state.running = pending;
            pending->launch();

            while (self->state.running->running()) {
                vTaskDelay(pdMS_TO_TICKS(100));
            }

            self->state.running = nullptr;
            ui_build(self);
        }

        // ── Teardown (Stop received) ───────────────────────────────────────────
        self->state.lock();
        IApp* running = self->state.running;
        self->state.unlock();

        if (running != nullptr && running->running()) {
            running->stop();
        }

        ui_destroy(self);

        delete self->state.apps;
        self->state.apps = nullptr;
    },
});

bool register_app(IApp* a, bool is_isr) {
    if (app.state.apps == nullptr) {
        ESP_LOGE("launcher", "register_app called before launcher is running");
        return false;
    }
    app.state.lock(is_isr);
    app.state.apps->push_back(a);
    app.state.unlock(is_isr);
    app.dirty();
    return true;
}

} // namespace nekos::app::launcher