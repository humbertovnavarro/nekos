#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <U8g2lib.h>
#include <MUIU8g2.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

// External U8G2 display object
extern U8G2 u8g2;

// Framebuffer redraw flag
extern uint8_t redraw;

// Semaphore for framebuffer locking
extern SemaphoreHandle_t framebuffer_lock_handle;
extern bool framebuffer_lock;

// Function declarations
void framebuffer_init();
void acquire_framebuffer();
void release_framebuffer();

#endif // FRAMEBUFFER_H
