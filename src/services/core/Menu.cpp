#include "lib/FrameBuffer.h"
#include "U8g2lib.h"
#include "Menu.h"

MUIU8G2 mui;

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
        MUI_10 "Wifi|"
        MUI_20 "USB|"
        MUI_30 "Evil Portal|"
        MUI_40 "Pong"
    )
    // Menu Button Fields
    MUI_XYA("MB", 5, 25, 0) 
    MUI_XYA("MB", 5, 37, 1) 
    MUI_XYA("MB", 5, 49, 2) 
    MUI_XYA("MB", 5, 61, 3) 
};


void menu_setup() {
    framebuffer_acquire();
    mui.begin(u8g2, fds_data, muif_list, sizeof(muif_list) / sizeof(muif_t));
    mui.gotoForm(1, 0);
}

void menu_loop() {
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
            break;
        case U8X8_MSG_GPIO_MENU_NEXT:
            mui.nextField();
            redraw = 1;
            break;
        case U8X8_MSG_GPIO_MENU_PREV:
            mui.prevField();
            redraw = 1;
            break;
        default:
            redraw = 0;
            break;
    }
}