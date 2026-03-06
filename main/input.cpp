#include <Arduino.h>
#include "input.hpp"

Input* Input::_instances[BTN_MAX_INPUTS] = {};
int    Input::_count                     = 0;

Input::Input(uint8_t pin, int debounce_ms, int long_press_ms)
    : _pin(pin), _debounce_ms(debounce_ms), _long_press_ms(long_press_ms) {}

void Input::begin()
{
    pinMode(_pin, INPUT_PULLUP);
    if (_count < BTN_MAX_INPUTS)
        _instances[_count++] = this;
}

void Input::pollInputs()
{
    for (int i = 0; i < _count; i++)
        _instances[i]->poll();
}

void Input::poll()
{
    _fell      = false;
    _rose      = false;
    _short_evt = false;
    _long_evt  = false;

    const bool          raw = digitalRead(_pin);
    const unsigned long t   = millis();

    if (raw != _last_raw) { _change_at = t; _last_raw = raw; }

    if ((t - _change_at) < (unsigned long)_debounce_ms)
        return;

    const bool prev = _debounced;
    _debounced = raw;

    if (prev == HIGH && _debounced == LOW) {    // HIGH → LOW : press
        _pressed    = true;
        _down_at    = t;
        _long_fired = false;
        _fell       = true;
    }

    if (prev == LOW && _debounced == HIGH) {    // LOW → HIGH : release
        _rose      = true;
        _short_evt = _pressed && !_long_fired;
        _pressed   = false;
    }

    if (_pressed && !_long_fired && (t - _down_at) >= (unsigned long)_long_press_ms) {
        _long_fired = true;
        _long_evt   = true;
    }
}

bool    Input::pressed()     const { return _fell;      }
bool    Input::released()    const { return _rose;      }
bool    Input::short_press() const { return _short_evt; }
bool    Input::long_press()  const { return _long_evt;  }
bool    Input::is_held()     const { return _pressed;   }
bool    Input::long_fired()  const { return _long_fired;}

int32_t Input::held_ms() const
{
    return _pressed ? (int32_t)(millis() - _down_at) : 0;
}

bool Input::get_pressed(int index)
{
    if (index < 0 || index >= _count) return false;
    return _instances[index]->pressed();
}

int32_t Input::get_held(int index)
{
    if (index < 0 || index >= _count) return 0;
    return _instances[index]->held_ms();
}