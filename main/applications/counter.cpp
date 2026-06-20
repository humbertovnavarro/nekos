#include "esp32_s3_touch_amoled_2_06.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl.h"
#include "etl/string.h"
#include <cstddef>
#include "counter.hpp"

static struct {
    lv_obj_t *screen;
    lv_obj_t *count_label;
    lv_obj_t *btn_inc;
    lv_obj_t *btn_dec;
    lv_obj_t *btn_reset;
    int32_t   count;
} s_ctx{};

static void update_label()
{
    char buf[16];
    snprintf(buf, sizeof(buf), "%ld", (long)s_ctx.count);
    lv_label_set_text(s_ctx.count_label, buf);
}

static void on_increment(lv_event_t *e)
{
    (void)e;
    s_ctx.count++;
    update_label();
}

static void on_decrement(lv_event_t *e)
{
    (void)e;
    s_ctx.count--;
    update_label();
}

static void on_reset(lv_event_t *e)
{
    (void)e;
    s_ctx.count = 0;
    update_label();
}

static void build_ui()
{
    s_ctx.screen = lv_obj_create(nullptr);
    lv_obj_set_style_bg_color(s_ctx.screen, lv_color_hex(0x1A1A2E), 0);
    lv_scr_load(s_ctx.screen);

    lv_obj_t *title = lv_label_create(s_ctx.screen);
    lv_label_set_text(title, "Counter");
    lv_obj_set_style_text_color(title, lv_color_hex(0xA0A0C0), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);

    s_ctx.count_label = lv_label_create(s_ctx.screen);
    lv_label_set_text(s_ctx.count_label, "0");
    lv_obj_set_style_text_color(s_ctx.count_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(s_ctx.count_label, &lv_font_montserrat_48, 0);
    lv_obj_align(s_ctx.count_label, LV_ALIGN_CENTER, 0, -20);

    lv_obj_t *divider = lv_obj_create(s_ctx.screen);
    lv_obj_set_size(divider, 120, 2);
    lv_obj_set_style_bg_color(divider, lv_color_hex(0x3A3A5C), 0);
    lv_obj_set_style_border_width(divider, 0, 0);
    lv_obj_set_style_radius(divider, 0, 0);
    lv_obj_align(divider, LV_ALIGN_CENTER, 0, 18);

    static lv_style_t btn_style;
    lv_style_init(&btn_style);
    lv_style_set_radius(&btn_style, 12);
    lv_style_set_border_width(&btn_style, 0);
    lv_style_set_text_font(&btn_style, &lv_font_montserrat_14);
    lv_style_set_pad_hor(&btn_style, 24);
    lv_style_set_pad_ver(&btn_style, 12);

    static lv_style_t btn_pressed;
    lv_style_init(&btn_pressed);
    lv_style_set_transform_scale(&btn_pressed, 950); // 0.95×

    s_ctx.btn_dec = lv_btn_create(s_ctx.screen);
    lv_obj_add_style(s_ctx.btn_dec, &btn_style, 0);
    lv_obj_add_style(s_ctx.btn_dec, &btn_pressed, LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(s_ctx.btn_dec, lv_color_hex(0xE74C3C), 0);
    lv_obj_set_size(s_ctx.btn_dec, 72, 56);
    lv_obj_align(s_ctx.btn_dec, LV_ALIGN_BOTTOM_MID, -90, -30);
    lv_obj_add_event_cb(s_ctx.btn_dec, on_decrement, LV_EVENT_CLICKED, nullptr);

    lv_obj_t *lbl_dec = lv_label_create(s_ctx.btn_dec);
    lv_label_set_text(lbl_dec, LV_SYMBOL_MINUS);
    lv_obj_center(lbl_dec);

    s_ctx.btn_reset = lv_btn_create(s_ctx.screen);
    lv_obj_add_style(s_ctx.btn_reset, &btn_style, 0);
    lv_obj_add_style(s_ctx.btn_reset, &btn_pressed, LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(s_ctx.btn_reset, lv_color_hex(0x3A3A5C), 0);
    lv_obj_set_size(s_ctx.btn_reset, 72, 56);
    lv_obj_align(s_ctx.btn_reset, LV_ALIGN_BOTTOM_MID, 0, -30);
    lv_obj_add_event_cb(s_ctx.btn_reset, on_reset, LV_EVENT_CLICKED, nullptr);

    lv_obj_t *lbl_rst = lv_label_create(s_ctx.btn_reset);
    lv_label_set_text(lbl_rst, LV_SYMBOL_REFRESH);
    lv_obj_center(lbl_rst);

    s_ctx.btn_inc = lv_btn_create(s_ctx.screen);
    lv_obj_add_style(s_ctx.btn_inc, &btn_style, 0);
    lv_obj_add_style(s_ctx.btn_inc, &btn_pressed, LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(s_ctx.btn_inc, lv_color_hex(0x27AE60), 0);
    lv_obj_set_size(s_ctx.btn_inc, 72, 56);
    lv_obj_align(s_ctx.btn_inc, LV_ALIGN_BOTTOM_MID, 90, -30);
    lv_obj_add_event_cb(s_ctx.btn_inc, on_increment, LV_EVENT_CLICKED, nullptr);

    lv_obj_t *lbl_inc = lv_label_create(s_ctx.btn_inc);
    lv_label_set_text(lbl_inc, LV_SYMBOL_PLUS);
    lv_obj_center(lbl_inc);
}

static void counter_task(void *arg)
{
    (void)arg;
    if (bsp_display_lock(100)) {
        build_ui();
        bsp_display_unlock();
    }
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    vTaskDelete(nullptr);
}

TaskLaunchOptions counter_task_opts = {
    .name = "counter_task",
    .icon = "⏲️",
    .task = nullptr,
    .task_fn = counter_task,
    .task_stack_depth = 4096,
    .task_priority = 3
};