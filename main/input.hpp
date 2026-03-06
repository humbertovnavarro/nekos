#pragma once
#include <Arduino.h>

#define BTN_MAX_INPUTS 8

class Input {
public:
    Input(uint8_t pin, int debounce_ms = 50, int long_press_ms = 500);

    void begin();

    static void pollInputs();

    bool    pressed()     const;
    bool    released()    const;
    bool    short_press() const;
    bool    long_press()  const;
    bool    is_held()     const;
    bool    long_fired()  const;

    int32_t held_ms()     const;

    static bool    get_pressed(int index);
    static int32_t get_held(int index);
private:
    void poll();

    uint8_t _pin;
    int     _debounce_ms;
    int     _long_press_ms;

    bool    _last_raw    = HIGH;
    bool    _debounced   = HIGH;
    bool    _pressed     = false;
    bool    _long_fired  = false;

    bool    _fell        = false;
    bool    _rose        = false;
    bool    _short_evt   = false;
    bool    _long_evt    = false;

    unsigned long _change_at = 0;
    unsigned long _down_at   = 0;

    static Input* _instances[BTN_MAX_INPUTS];
    static int    _count;
};
