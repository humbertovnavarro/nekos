# Build Instructions
- Download PlatformIO
- setup [udev rules](https://docs.platformio.org/en/stable/core/installation/udev-rules.html) if you haven't already
- Clone Submodules `git submodule clone --recursive`
- Set the default environment for your device in platformio.ini
- Build. (ctrl + shift + p) PlatformIO Build

# ⛰️🐈 Nek OS – Design Goals 🐈‍⬛⛰️

## 1. Cross-Device Support
- Target as many boards as possible using the Arduino HAL where feasible.
- Implement zero cost abstractions to support hardware when available.

## 2. Developer-Friendly
- Keep it easy for non-embedded developers.
- Balance ease of use with reasonable performance.

## 3. Modular Abstractions
- Prefer modular APIs over macros and port-specific code.
- Use trivial macros where they make sense.
- Aim for API stability to reduce breaking changes.

## 4. Optimized Scripting
- Lua is the default environment.
- Use C++ only for performance-critical sections.

## 5. Flexible UI Layers
  - Support multiple display styles:
  - Scrolling menu* for single-button devices that run CLI commands.
  - Heads-up display for devices built to run CLI-style workflows.

## 6. Remote Access (Optional)
- Consider adding SSH support for remote control.

## 7. Future Plans
- Meshtastic API
- Wifi attack and reconnaissance library and commands
- Binary ELF execution


## 8. Overall Vision
A "cute cat" OS that feels familiar to Unix, but is a first-class platform for embedded projects.
