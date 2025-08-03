#include <U8g2lib.h>
#include <MUIU8g2.h>
#include "FreeRTOS.h"
#include "config/device.h"
#include "lib/Tasks.h"
#include "freertos/task.h"

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

SemaphoreHandle_t framebuffer_lock_handle;
bool framebuffer_lock = false;

void framebuffer_init()
{
    u8g2.begin(MENU_SELECT_PIN, MENU_NEXT_PIN, MENU_PREV_PIN, U8X8_PIN_NONE, U8X8_PIN_NONE, MENU_HOME_PIN);
    u8g2.setFont(u8g2_font_pressstart2p_8f);
    framebuffer_lock_handle = xSemaphoreCreateMutex();
}

void acquire_framebuffer() {
    xSemaphoreTake(framebuffer_lock_handle, 100);
    framebuffer_lock = true;
}

void release_framebuffer() {
    framebuffer_lock = false;
    xSemaphoreGive(framebuffer_lock_handle);
}

uint8_t redraw = 1;