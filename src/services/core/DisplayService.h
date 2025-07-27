#pragma once
#include <U8g2lib.h>
#include <MUIU8g2.h>
#include "FreeRTOS.h"
#include <lib/TaskService.h>
// Forward declarations of your setup and loop functions
void display_setup(void* pvparams);
void display_loop(void* pvparams);
// Extern declaration of the global MUI instance used by the display service
extern MUIU8G2 mui;
// The task service instance, declared as extern so it can be linked
extern TaskService display_service;
