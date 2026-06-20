#include "styles.hpp"
#include "font/lv_font.h"
#include "misc/lv_style.h"
#include "misc/lv_style_gen.h"
namespace style {
lv_style_t button_pressed;
lv_style_t button;

inline void button_style() {
    lv_style_init(&button);
    lv_style_set_radius(&button, 12);
    lv_style_set_border_width(&button, 1);
    lv_style_set_text_font(&button,&lv_font_montserrat_14);
    lv_style_set_pad_hor(&button, 24);
    lv_style_set_pad_ver(&button, 12);
}

inline void button_pressed_style() {
    lv_style_init(&button_pressed);
    lv_style_set_transform_scale(&button_pressed, 300);
}

void init() {
    button_style();
    button_pressed_style();
}

} // namespace style