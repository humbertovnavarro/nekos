#pragma once
#include "esp32-hal.h"
#define GPIO_UP     9
#define GPIO_DOWN   18
#define GPIO_LEFT   11
#define GPIO_RIGHT  7
#define DEBOUNCE_MS 20

enum Input {
    NONE,
    UP,
    DOWN,
    LEFT,
    RIGHT
};

struct Button {
    uint8_t pin;
    bool stableState;
    bool lastReading;
    unsigned long lastChangeTime;
};

// ==============================
// Button Definitions
// ==============================
static Button buttons[] = {
    {GPIO_UP,    HIGH, HIGH, 0},
    {GPIO_DOWN,  HIGH, HIGH, 0},
    {GPIO_LEFT,  HIGH, HIGH, 0},
    {GPIO_RIGHT, HIGH, HIGH, 0},
};

inline void inputBegin() {
    for (auto &b : buttons)
        pinMode(b.pin, INPUT_PULLUP);
}

inline bool readDebounced(Button &b) {
    bool reading = digitalRead(b.pin);
    if (reading != b.lastReading)
        b.lastChangeTime = millis();

    if ((millis() - b.lastChangeTime) > DEBOUNCE_MS) {
        if (reading != b.stableState)
            b.stableState = reading;
    }

    b.lastReading = reading;
    return b.stableState;
}

inline Input getInputDirection() {
    if (readDebounced(buttons[0]) == LOW) return UP;
    if (readDebounced(buttons[1]) == LOW) return DOWN;
    if (readDebounced(buttons[2]) == LOW) return LEFT;
    if (readDebounced(buttons[3]) == LOW) return RIGHT;
    return NONE;
}
