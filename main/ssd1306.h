#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <math.h>
#include <string.h>
#include "font.h"

#ifndef OLED_SDA
  #define OLED_SDA 17
#endif
#ifndef OLED_SCL
  #define OLED_SCL 18
#endif
#ifndef OLED_RST
  #define OLED_RST 21
#endif

namespace oled {

constexpr uint8_t I2C_ADDR = 0x3C;
constexpr int     WIDTH    = 128;
constexpr int     HEIGHT   = 64;
constexpr int     PAGES    = HEIGHT / 8;
constexpr int     FB_SIZE  = WIDTH * PAGES;

namespace _internal {
    static uint8_t fb[FB_SIZE];

    inline void cmd(uint8_t c)
    {
        Wire.beginTransmission(I2C_ADDR);
        Wire.write(0x00);
        Wire.write(c);
        Wire.endTransmission();
    }
}

inline void flush()
{
    using namespace _internal;
    for (uint8_t c : {0x21, 0x00, 0x7F,
                      0x22, 0x00, 0x07}) cmd(c);
    static uint8_t page_buf[1 + WIDTH];
    page_buf[0] = 0x40;
    for (int p = 0; p < PAGES; p++) {
        memcpy(&page_buf[1], &fb[p * WIDTH], WIDTH);
#if defined(BUFFER_LENGTH)
        constexpr int CHUNK = BUFFER_LENGTH - 1;
#elif defined(I2C_BUFFER_LENGTH)
        constexpr int CHUNK = I2C_BUFFER_LENGTH - 1;
#else
        constexpr int CHUNK = 31;
#endif
        int sent = 0;
        while (sent < WIDTH) {
            int n = min((int)(WIDTH - sent), CHUNK);
            Wire.beginTransmission(I2C_ADDR);
            Wire.write(0x40);
            Wire.write(&fb[p * WIDTH + sent], n);
            Wire.endTransmission();
            sent += n;
        }
    }
}

inline bool init()
{
    pinMode(OLED_RST, OUTPUT);
    digitalWrite(OLED_RST, LOW);
    delay(10);
    digitalWrite(OLED_RST, HIGH);
    delay(10);

    Wire.begin(OLED_SDA, OLED_SCL);
    Wire.setClock(400000);

    static const uint8_t INIT_SEQ[] = {
        0xAE,           // display off
        0xD5, 0x80,     // clock div
        0xA8, 0x3F,     // mux ratio (63)
        0xD3, 0x00,     // display offset
        0x40,           // start line 0
        0x8D, 0x14,     // charge pump on
        0x20, 0x00,     // horizontal addressing mode
        0xA1,           // seg remap
        0xC8,           // com scan direction
        0xDA, 0x12,     // com pins
        0x81, 0xCF,     // contrast
        0xD9, 0xF1,     // pre-charge
        0xDB, 0x40,     // vcomh deselect
        0xA4,           // display from RAM
        0xA6,           // normal (non-inverted)
        0xAF,           // display on
    };
    for (uint8_t c : INIT_SEQ) _internal::cmd(c);
    memset(_internal::fb, 0, FB_SIZE);
    flush();
    return true;
}

inline void deinit()
{
    Wire.end();
}

inline void     clear()           { memset(_internal::fb, 0, FB_SIZE); }
inline uint8_t *framebuffer()     { return _internal::fb; }

inline void set_pixel(int x, int y, uint8_t on)
{
    if ((unsigned)x >= (unsigned)WIDTH || (unsigned)y >= (unsigned)HEIGHT) return;
    uint8_t &byte = _internal::fb[(y >> 3) * WIDTH + x];
    uint8_t  mask = 1u << (y & 7);
    if (on) byte |= mask; else byte &= ~mask;
}

inline void draw_char(int x, int page, char c, bool invert = false)
{
    if ((unsigned)page >= (unsigned)PAGES) return;
    if (c < 0x20 || c > 0x7E) c = '?';
    const uint8_t *glyph = FONT5x8[(uint8_t)c - 0x20];
    uint8_t *row = &_internal::fb[page * WIDTH];
    for (int col = 0; col < 5; col++) {
        if (x + col >= WIDTH) return;
        uint8_t b = glyph[col];
        row[x + col] = invert ? ~b : b;
    }
    if (x + 5 < WIDTH)
        row[x + 5] = invert ? 0xFF : 0x00;
}

inline int draw_string(int x, int page, const char *s, bool invert = false)
{
    while (*s && x < WIDTH) {
        draw_char(x, page, *s++, invert);
        x += 6;
    }
    return x;
}

inline void draw_string_centred(int x_start, int w, int page,
                                const char *s, bool invert = false)
{
    int len    = (int)strlen(s);
    int text_w = len * 6 - 1;
    int x      = x_start + (w - text_w) / 2;
    if (x < x_start) x = x_start;
    draw_string(x, page, s, invert);
}

inline void draw_string_right(int x_end, int page,
                              const char *s, bool invert = false)
{
    int x = x_end - (int)strlen(s) * 6 + 1;
    if (x < 0) x = 0;
    draw_string(x, page, s, invert);
}

inline void fill_rect(int x, int y, int w, int h, uint8_t on)
{
    int x2 = x + w, y2 = y + h;
    if (x  < 0)      x  = 0;
    if (y  < 0)      y  = 0;
    if (x2 > WIDTH)  x2 = WIDTH;
    if (y2 > HEIGHT) y2 = HEIGHT;
    if (x >= x2 || y >= y2) return;

    int p0 = y >> 3, p1 = (y2 - 1) >> 3;
    for (int p = p0; p <= p1; p++) {
        int     lo   = (p == p0) ? (y  & 7) : 0;
        int     hi   = (p == p1) ? ((y2 - 1) & 7) : 7;
        uint8_t mask = (uint8_t)(((0x100 << hi) - 1) & ~((1 << lo) - 1));
        uint8_t *row = &_internal::fb[p * WIDTH];
        if (on) {
            for (int cx = x; cx < x2; cx++) row[cx] |= mask;
        } else {
            uint8_t inv = ~mask;
            for (int cx = x; cx < x2; cx++) row[cx] &= inv;
        }
    }
}

inline void draw_rect(int x, int y, int w, int h, uint8_t on)
{
    if (w <= 0 || h <= 0) return;
    fill_rect(x,         y,         w, 1, on);
    fill_rect(x,         y + h - 1, w, 1, on);
    fill_rect(x,         y,         1, h, on);
    fill_rect(x + w - 1, y,         1, h, on);
}

inline void draw_hline(int x, int y, int w, uint8_t on) { fill_rect(x, y, w, 1, on); }
inline void draw_vline(int x, int y, int h, uint8_t on) { fill_rect(x, y, 1, h, on); }

inline void draw_line(int x0, int y0, int x1, int y1, uint8_t on)
{
    int dx =  (x1 > x0 ? x1 - x0 : x0 - x1);
    int dy = -(y1 > y0 ? y1 - y0 : y0 - y1);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;
    for (;;) {
        set_pixel(x0, y0, on);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

inline void draw_circle(int cx, int cy, int r, uint8_t on)
{
    int x = r, y = 0, err = 1 - r;
    while (x >= y) {
        set_pixel(cx + x, cy + y, on); set_pixel(cx - x, cy + y, on);
        set_pixel(cx + x, cy - y, on); set_pixel(cx - x, cy - y, on);
        set_pixel(cx + y, cy + x, on); set_pixel(cx - y, cy + x, on);
        set_pixel(cx + y, cy - x, on); set_pixel(cx - y, cy - x, on);
        y++;
        if (err < 0) err += 2 * y + 1;
        else       { x--; err += 2 * (y - x) + 1; }
    }
}

inline void fill_circle(int cx, int cy, int r, uint8_t on)
{
    for (int dy = -r; dy <= r; dy++) {
        int dx = (int)sqrtf((float)(r * r - dy * dy));
        draw_hline(cx - dx, cy + dy, 2 * dx + 1, on);
    }
}

inline void draw_bitmap(int x, int page, int w, int w_pages, const uint8_t *bmp)
{
    for (int p = 0; p < w_pages && (page + p) < PAGES; p++)
        for (int col = 0; col < w && (x + col) < WIDTH; col++)
            _internal::fb[(page + p) * WIDTH + x + col] = bmp[p * w + col];
}

inline void xor_bitmap(int x, int page, int w, int w_pages, const uint8_t *bmp)
{
    for (int p = 0; p < w_pages && (page + p) < PAGES; p++)
        for (int col = 0; col < w && (x + col) < WIDTH; col++)
            _internal::fb[(page + p) * WIDTH + x + col] ^= bmp[p * w + col];
}

inline void draw_progress(int x, int y, int w, int h,
                          int value, int max_val, uint8_t on)
{
    draw_rect(x, y, w, h, on);
    if (max_val <= 0) return;
    int fill = (value * (w - 2)) / max_val;
    if (fill > w - 2) fill = w - 2;
    if (fill > 0) fill_rect(x + 1, y + 1, fill, h - 2, on);
}

inline void draw_statusbar(const char *left_text, const char *right_text,
                           bool separator = true)
{
    fill_rect(0, 0, WIDTH, 8, 0);
    draw_string(0, 0, left_text);
    if (right_text) draw_string_right(WIDTH - 1, 0, right_text);
    if (separator)  draw_hline(0, 8, WIDTH, 1);
}

} // namespace oled