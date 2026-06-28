#include "counter.hpp"
#include "app.hpp"
#include "core/lv_obj.h"
#include "core/lv_obj_tree.h"
#include "esp32_s3_touch_amoled_2_06.h"
#include "freertos/idf_additions.h"
#include "portmacro.h"

namespace nekos::app::counter {

static void on_increment(lv_event_t* e) {
    auto* self = static_cast<nekos::App<State>*>(lv_event_get_user_data(e));
    self->references.count++;
    lv_label_set_text_fmt(self->references.label, "%d", self->references.count);
}

static void on_decrement(lv_event_t* e) {
    auto* self = static_cast<nekos::App<State>*>(lv_event_get_user_data(e));
    self->references.count--;
    lv_label_set_text_fmt(self->references.label, "%d", self->references.count);
}

static void on_reset(lv_event_t* e) {
    auto* self = static_cast<nekos::App<State>*>(lv_event_get_user_data(e));
    self->references.count = 0;
    lv_label_set_text_fmt(self->references.label, "%d", self->references.count);
}

static lv_obj_t* draw_button(lv_obj_t* parent, const char* text, lv_event_cb_t cb, nekos::App<State>* self) {
    lv_obj_t* btn = lv_button_create(parent);
    lv_obj_set_size(btn, 80, 80);
    lv_obj_set_style_bg_color(btn, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(btn, 1, 0);
    lv_obj_set_style_border_color(btn, lv_palette_darken(LV_PALETTE_GREY, 3), 0);
    lv_obj_set_style_radius(btn, 20, 0);
    lv_obj_set_style_shadow_width(btn, 0, 0);
    lv_obj_set_style_outline_width(btn, 0, 0);
    lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, self);

    lv_obj_t* lbl = lv_label_create(btn);
    lv_label_set_text(lbl, text);
    lv_obj_set_style_text_color(lbl, lv_palette_lighten(LV_PALETTE_GREY, 1), 0);
    lv_obj_set_style_text_font(lbl, &lv_font_montserrat_14, 0);
    lv_obj_align(lbl, LV_ALIGN_CENTER, 0, 0);

    return btn;
}

static void draw(nekos::App<State>* self) {
    State& s = self->references;

    lv_obj_set_style_bg_color(s.root, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(s.root, LV_OPA_COVER, 0);

    lv_obj_t* container = lv_obj_create(s.root);
    lv_obj_remove_style_all(container);
    lv_obj_set_size(container, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_opa(container, LV_OPA_TRANSP, 0);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(container, 24, 0);

    s.label = lv_label_create(container);
    lv_label_set_text_fmt(s.label, "%d", (int)s.count);
    lv_obj_set_style_text_color(s.label, lv_color_white(), 0);
    lv_obj_set_style_text_font(s.label, &lv_font_montserrat_48, 0);

    lv_obj_t* btn_row = lv_obj_create(container);
    lv_obj_remove_style_all(btn_row);
    lv_obj_set_size(btn_row, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(btn_row, LV_OPA_TRANSP, 0);
    lv_obj_set_flex_flow(btn_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btn_row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(btn_row, 20, 0);

    draw_button(btn_row, "-",     on_decrement, self);
    draw_button(btn_row, "RST",   on_reset,     self);
    draw_button(btn_row, "+",     on_increment, self);
}

App<State> app = App<State>::create({
    .name       = "counter",
    .icon       = "",
    .fn         = [](auto self) {
        bsp_display_lock(portMAX_DELAY);
        draw(self);
        lv_screen_load(self->references.root);
        bsp_display_unlock();
        while (1) {
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    },
    .allocater  = [](auto self) {
        self->references.root  = lv_obj_create(NULL);
        self->references.label = nullptr;
        self->references.count = 0;
    },
    .deleter    = [](auto self) {
        bsp_display_lock(portMAX_DELAY);
        lv_obj_del(self->references.root);
        bsp_display_unlock();
        self->references.root  = nullptr;
        self->references.label = nullptr;
    },
    .on_background = NEKOS_DISPLAY_BACKGROUND,
    .on_foreground = NEKOS_DISPLAY_FOREGROUND(draw)
});

} // namespace nekos::app::counter