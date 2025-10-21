#include "Arduino.h"

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

// Four button objects
Button buttons[] = {
    {GPIO_UP,    HIGH, HIGH, 0},
    {GPIO_DOWN,  HIGH, HIGH, 0},
    {GPIO_LEFT,  HIGH, HIGH, 0},
    {GPIO_RIGHT, HIGH, HIGH, 0},
};

void begin() {
    for (auto &b : buttons)
        pinMode(b.pin, INPUT_PULLUP);
    Serial.begin(115200);
}

// Helper function: update each button and return debounced state
bool readDebounced(Button &b) {
    bool reading = digitalRead(b.pin);
    if (reading != b.lastReading) {
        b.lastChangeTime = millis();  // reset timer
    }

    if ((millis() - b.lastChangeTime) > DEBOUNCE_MS) {
        if (reading != b.stableState) {
            b.stableState = reading;
        }
    }

    b.lastReading = reading;
    return b.stableState;
}

Input getInputDirection() {
    uint8_t dir = NONE;
    if (readDebounced(buttons[0]) == LOW) dir = UP;
    if (readDebounced(buttons[1]) == LOW) dir = DOWN;
    if (readDebounced(buttons[2]) == LOW) dir = LEFT;
    if (readDebounced(buttons[3]) == LOW) dir = RIGHT;
    return static_cast<Input>(dir);
}