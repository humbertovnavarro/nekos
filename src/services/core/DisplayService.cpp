#include <U8g2lib.h>
#include <MUIU8g2.h>
#include "FreeRTOS.h"
#include "config/device.h"
#include "../../lib/TaskService.h"

#ifdef SSD1306I2C
U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
#else
U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
#endif

muif_t muif_list[] = {
    MUIF_RO("GP",mui_u8g2_goto_data),
    MUIF_BUTTON("GC", mui_u8g2_goto_form_w1_pi),
};

fds_t fds_data[] = {
    MUI_FORM(1)
    MUI_STYLE(0)
    MUI_DATA("GP", 
        MUI_10 "Goto Buttons|"
        MUI_20 "uint8 Number|"
        MUI_30 "uint8 Checkbox|"
        MUI_40 "uint8 Cycle Options|"
        MUI_50 "uint8 ParentChild Select|"
        MUI_60 "uint8 Char/Text Input|"
        MUI_70 "uint16 Cycle Options|"
        MUI_80 "uint16 ParentChild Select")
    MUI_XYA("GC", 5, 25, 0) 
    MUI_XYA("GC", 5, 37, 1) 
    MUI_XYA("GC", 5, 49, 2) 
    MUI_XYA("GC", 5, 61, 3) 
};
MUIU8G2 mui;

uint8_t u8g2_event_handler(U8G2 display, MUIU8G2 mui) {
    switch(u8g2.getMenuEvent()) {
        case U8X8_MSG_GPIO_MENU_SELECT:
            mui.sendSelect();
            return 1;
        case U8X8_MSG_GPIO_MENU_NEXT:
            mui.nextField();
            return 1;
        case U8X8_MSG_GPIO_MENU_PREV:
            mui.prevField();
            return 1;
    }
    return 0;
}

void display_setup(TaskService *service)
{
    u8g2.begin(MENU_SELECT_PIN, MENU_NEXT_PIN, MENU_PREV_PIN, U8X8_PIN_NONE, U8X8_PIN_NONE, MENU_HOME_PIN);
    u8g2.setFont(u8g2_font_pressstart2p_8f);
    mui.begin(u8g2, fds_data, muif_list, sizeof(muif_list) / sizeof(muif_t));
    mui.gotoForm(1, 0);
}
uint8_t redraw = 1;
void display_loop(TaskService *service)
{
    if (mui.isFormActive())
    {
        if (redraw)
        {
            u8g2.firstPage();
            do
            {
                mui.draw();
            } while (u8g2.nextPage());
        }
        redraw = u8g2_event_handler(u8g2, mui);
    }
}

TaskService svc("task", 1024, 0, display_setup, display_loop);