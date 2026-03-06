idf_component_register(SRCS "lua_state_pool.cpp" "input.cpp" "stdin.cpp" "lua_runner.cpp" "lua_cli.cpp" "main.cpp"
                    INCLUDE_DIRS "." "../lib/etl/include"
                    REQUIRES lua spiffs nvs_flash wear_levelling esp_wifi esp_timer arduino-esp32
                    )
