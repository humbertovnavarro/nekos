#pragma once
#include <U8g2lib.h>
#include <MUIU8g2.h>
#include "FreeRTOS.h"
#include "../../lib/TaskService.h"
void display_setup(TaskService* pvparams);
void display_loop(TaskService* pvparams);
