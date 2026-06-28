#include "driver/i2c_types.h"
#include "esp32_s3_touch_amoled_2_06.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "applications/launcher.hpp"
#include "peripherals.hpp"

i2c_master_bus_handle_t i2c_master_bus;

#define BOOT_BTN_GPIO   GPIO_NUM_0
#define LONG_PRESS_MS   1000

static const uint32_t BUF_BYTES = 410 * 502 * sizeof(lv_color_t);

extern "C" void app_main(void) {
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  esp_err_t ret = nvs_flash_init();

  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ESP_ERROR_CHECK(nvs_flash_init());
  }

  bsp_display_cfg_t bsp_display_cfg = {
      .lvgl_port_cfg =
        {
            .task_priority = 4,
            .task_stack = 16384,
            .task_affinity = 1,
            .task_max_sleep_ms = 64,
            .task_stack_caps = MALLOC_CAP_SPIRAM & MALLOC_CAP_DMA,
            .timer_period_ms = 33,
        },
      .buffer_size = BUF_BYTES,
      .trans_size = BUF_BYTES,
      .double_buffer = true,
      .flags =
          {
              .buff_dma = true,
              .buff_spiram = true,
          },
  };
  
  bsp_display_start_with_config(&bsp_display_cfg);
  i2c_master_bus = bsp_i2c_get_handle();
  nekos::app::launcher::app.launch();
}