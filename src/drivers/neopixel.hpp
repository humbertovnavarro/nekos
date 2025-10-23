#pragma once
#include "Adafruit_NeoPixel.h"
// =======================================
// NeoPixel Setup
// =======================================
#ifndef LED_PIN
#define LED_PIN 4
#endif

#ifndef LED_COUNT
#define LED_COUNT 8
#endif

inline static Adafruit_NeoPixel neopixel(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
