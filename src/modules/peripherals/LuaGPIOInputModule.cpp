#include "Arduino.h"
#include "lua.hpp"

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

// ==============================
// Setup and Debounce Logic
// ==============================
void inputBegin() {
    for (auto &b : buttons)
        pinMode(b.pin, INPUT_PULLUP);
    Serial.begin(115200);
}

static bool readDebounced(Button &b) {
    bool reading = digitalRead(b.pin);
    if (reading != b.lastReading) {
        b.lastChangeTime = millis();
    }

    if ((millis() - b.lastChangeTime) > DEBOUNCE_MS) {
        if (reading != b.stableState) {
            b.stableState = reading;
        }
    }

    b.lastReading = reading;
    return b.stableState;
}

static Input getInputDirection() {
    if (readDebounced(buttons[0]) == LOW) return UP;
    if (readDebounced(buttons[1]) == LOW) return DOWN;
    if (readDebounced(buttons[2]) == LOW) return LEFT;
    if (readDebounced(buttons[3]) == LOW) return RIGHT;
    return NONE;
}

static int luaInputRead(lua_State* L) {
    Input dir = getInputDirection();
    const char* result = "none";
    switch (dir) {
        case UP:    result = "up"; break;
        case DOWN:  result = "down"; break;
        case LEFT:  result = "left"; break;
        case RIGHT: result = "right"; break;
        default: break;
    }
    lua_pushstring(L, result);
    return 1;
}

void luaOpenInputLibs(lua_State* L) {
    lua_newtable(L);
    lua_pushcfunction(L, luaInputRead);
    lua_setfield(L, -2, "read");
    lua_setglobal(L, "input");
}
