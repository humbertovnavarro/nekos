#pragma once

// Forward declarations for task functions
void display_setup(void* pvparams);
void display_loop(void* pvparams);

// TaskService declaration via macro (if macro creates an extern or instantiates a TaskService object, adapt accordingly)
//CREATE_TASK(display_service, display_setup, display_loop, 8192, 0, 0);
