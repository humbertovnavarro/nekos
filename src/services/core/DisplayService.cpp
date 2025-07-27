#include <U8g2lib.h>
#include <MUIU8g2.h>
#include "FreeRTOS.h"
#include "config/device.h"
#include "../lib/GlobalServiceRegistry.h"
#ifdef SSD1306I2C
U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
#else
U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
#endif

muif_t muif_list[] = {
    MUIF_RO("MM", mui_u8g2_goto_data),
    MUIF_BUTTON("MF", mui_u8g2_goto_form_w1_pi),
};
fds_t fds_data[] =
    // Form 1: Main Menu
    MUI_FORM(1)
        MUI_STYLE(0)
            MUI_DATA("MM",
                     MUI_2 "WIFI|" MUI_2 "BLE|" MUI_2 "USB|" MUI_2 "GPIO|" MUI_2 "NEOPIXEL|")
                MUI_XYA("MF", 4, 14, 0)
                    MUI_XYA("MF", 4, 28, 1)
                        MUI_XYA("MF", 4, 42, 2)
                            MUI_XYA("MF", 4, 56, 3)
    // Form 2: Placeholder.
    MUI_FORM(2)
        MUI_STYLE(0)
            MUI_XYT("GB", 0, 0, "Back")
    // Form 3: Console Output
    MUI_FORM(3)
        MUI_STYLE(0)
            MUI_XYT("CO", 0, 0, " OK ");
MUIU8G2 mui;
uint8_t is_redraw = 1;
void display_setup(void *pvparams)
{
    u8g2.begin(MENU_SELECT_PIN, MENU_NEXT_PIN, MENU_PREV_PIN, U8X8_PIN_NONE, U8X8_PIN_NONE, MENU_HOME_PIN);
    u8g2.setFont(u8g2_font_pressstart2p_8f);
    mui.begin(u8g2, fds_data, muif_list, sizeof(muif_list) / sizeof(muif_t));
    mui.gotoForm(1, 0);
}
void display_loop(void *pvparams)
{
    u8g2.setFont(u8g2_font_helvR08_tr);
    if (mui.isFormActive())
    {
        if (is_redraw)
        { // is any redraw of the menu required?
            u8g2.firstPage();
            do
            {
                mui.draw();
            } while (u8g2.nextPage());
            is_redraw = 0; // menu is now up to date, no redraw required at the moment
        }
    }
}
REGISTER_TASK_SERVICE(display, 4096, 0, 1)