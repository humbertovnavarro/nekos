#include <U8g2lib.h>
#include <MUIU8g2.h>
#include "FreeRTOS.h"
#include "config/device.h"

#ifdef SSD1306I2C
    #ifdef GPIO_CLK
        U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, GPIO_CLK, GPIO_DATA, GPIO_RESET);
    #else
        U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
    #endif
#else
    #ifdef GPIO_CLK
        U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, GPIO_CLK, GPIO_DATA, GPIO_RESET);
    #else
        U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
    #endif
#endif

uint16_t choice = 0;

muif_t muif_list[] = {
    MUIF_RO("MM",mui_u8g2_goto_data),
    MUIF_BUTTON("MB", mui_u8g2_goto_form_w1_pi),
};

fds_t fds_data[] = {
    // Main Menu
    MUI_FORM(1)
    MUI_STYLE(0)
    // Menu Gotos
    MUI_DATA("MM", 
        MUI_10 "Services|"
    )
    // Menu Button Fields
    MUI_XYA("MB", 5, 25, 0) 
    MUI_XYA("MB", 5, 37, 1) 
    MUI_XYA("MB", 5, 49, 2) 
    MUI_XYA("MB", 5, 61, 3) 
    // Service List
    MUI_FORM(10)
    MUI_STYLE(0)
    MUI_XYA("SL", 5, 25, 50)
};

MUIU8G2 mui;

void display_setup()
{
    u8g2.begin(MENU_SELECT_PIN, MENU_NEXT_PIN, MENU_PREV_PIN, U8X8_PIN_NONE, U8X8_PIN_NONE, MENU_HOME_PIN);
    u8g2.setFont(u8g2_font_pressstart2p_8f);
    mui.begin(u8g2, fds_data, muif_list, sizeof(muif_list) / sizeof(muif_t));
    #ifndef MONO_INPUT
        mui.gotoForm(1, 0);
    #else
        // Just open the console
        mui.gotoForm(100,0);
    #endif
}

uint8_t redraw = 1;
void display_loop()
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
        switch(u8g2.getMenuEvent()) {
            case U8X8_MSG_GPIO_MENU_SELECT:
                mui.sendSelect();
                redraw = 1;
            case U8X8_MSG_GPIO_MENU_NEXT:
                mui.nextField();
                redraw = 1;
            case U8X8_MSG_GPIO_MENU_PREV:
                mui.prevField();
                redraw = 1;
            default:
                redraw = 0;
        }
    }
}
