# VSCodium Arduino and ESP-IDF Starter Template
lib/esp32-arduino-lib-builder is needed for arduino usb api.

# Setup
- If you are not using USB functionality, you may skip this.
- git submodule update --init --recursive

# Requirements
- VSCodium
- ESP-IDF Extension
- Clang Extension

# Changes
- Adds idf_component.yml with espressif/arduino-esp32: ^3.3.7
- Changes main/CMakeLists.txt from main.c to main.cpp (arduino uses cpp features)
- Adds REQUIRES arduino-esp32 arduino_tinyusb to main/CMakeLists.txt
- Adds lib/esp32-arduino-lib-builder/components/arduino_tinyusb to root CMakeLists.txt
- Updates esp idf sdk values to support setup and loop functions.
- Sets FreeRTOS tick rate to 1000

Use ESP-IDF to set the build target. This template targets the esp32s3. If you change this, you may need to update the sdkconfig to set the tick rate of FreeRTOS to 1000, and setup automatic setup and loop. You can do this using menu config. Press ctrl+shift+p. Search for ESP-IDF: SDK Configuration Editor (MenuConfig)

If it fails to build after doing this, do a full clean using esp idf. (Full Clean Project).

To add additional arduino libraries, follow the process used to add arduino_tinyusb.