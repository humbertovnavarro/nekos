#pragma once
#include "ssd1306.h"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

namespace lua_oled {

// oled.clear()
static int l_clear(lua_State *L) {
    oled::clear();
    return 0;
}

// oled.flush()
static int l_flush(lua_State *L) {
    oled::flush();
    return 0;
}

// oled.set_pixel(x, y, on)
static int l_set_pixel(lua_State *L) {
    int x  = luaL_checkinteger(L, 1);
    int y  = luaL_checkinteger(L, 2);
    int on = luaL_checkinteger(L, 3);
    oled::set_pixel(x, y, (uint8_t)on);
    return 0;
}

// oled.draw_string(x, page, str [, invert])
static int l_draw_string(lua_State *L) {
    int         x      = luaL_checkinteger(L, 1);
    int         page   = luaL_checkinteger(L, 2);
    const char *s      = luaL_checkstring(L, 3);
    bool        invert = lua_toboolean(L, 4);
    int         end_x  = oled::draw_string(x, page, s, invert);
    lua_pushinteger(L, end_x);
    return 1;
}

// oled.draw_string_centred(x_start, w, page, str [, invert])
static int l_draw_string_centred(lua_State *L) {
    int         x_start = luaL_checkinteger(L, 1);
    int         w       = luaL_checkinteger(L, 2);
    int         page    = luaL_checkinteger(L, 3);
    const char *s       = luaL_checkstring(L, 4);
    bool        invert  = lua_toboolean(L, 5);
    oled::draw_string_centred(x_start, w, page, s, invert);
    return 0;
}

// oled.draw_string_right(x_end, page, str [, invert])
static int l_draw_string_right(lua_State *L) {
    int         x_end  = luaL_checkinteger(L, 1);
    int         page   = luaL_checkinteger(L, 2);
    const char *s      = luaL_checkstring(L, 3);
    bool        invert = lua_toboolean(L, 4);
    oled::draw_string_right(x_end, page, s, invert);
    return 0;
}

// oled.fill_rect(x, y, w, h, on)
static int l_fill_rect(lua_State *L) {
    int x  = luaL_checkinteger(L, 1);
    int y  = luaL_checkinteger(L, 2);
    int w  = luaL_checkinteger(L, 3);
    int h  = luaL_checkinteger(L, 4);
    int on = luaL_checkinteger(L, 5);
    oled::fill_rect(x, y, w, h, (uint8_t)on);
    return 0;
}

// oled.draw_rect(x, y, w, h, on)
static int l_draw_rect(lua_State *L) {
    int x  = luaL_checkinteger(L, 1);
    int y  = luaL_checkinteger(L, 2);
    int w  = luaL_checkinteger(L, 3);
    int h  = luaL_checkinteger(L, 4);
    int on = luaL_checkinteger(L, 5);
    oled::draw_rect(x, y, w, h, (uint8_t)on);
    return 0;
}

// oled.draw_hline(x, y, w, on)
static int l_draw_hline(lua_State *L) {
    int x  = luaL_checkinteger(L, 1);
    int y  = luaL_checkinteger(L, 2);
    int w  = luaL_checkinteger(L, 3);
    int on = luaL_checkinteger(L, 4);
    oled::draw_hline(x, y, w, (uint8_t)on);
    return 0;
}

// oled.draw_vline(x, y, h, on)
static int l_draw_vline(lua_State *L) {
    int x  = luaL_checkinteger(L, 1);
    int y  = luaL_checkinteger(L, 2);
    int h  = luaL_checkinteger(L, 3);
    int on = luaL_checkinteger(L, 4);
    oled::draw_vline(x, y, h, (uint8_t)on);
    return 0;
}

// oled.draw_line(x0, y0, x1, y1, on)
static int l_draw_line(lua_State *L) {
    int x0 = luaL_checkinteger(L, 1);
    int y0 = luaL_checkinteger(L, 2);
    int x1 = luaL_checkinteger(L, 3);
    int y1 = luaL_checkinteger(L, 4);
    int on = luaL_checkinteger(L, 5);
    oled::draw_line(x0, y0, x1, y1, (uint8_t)on);
    return 0;
}

// oled.draw_circle(cx, cy, r, on)
static int l_draw_circle(lua_State *L) {
    int cx = luaL_checkinteger(L, 1);
    int cy = luaL_checkinteger(L, 2);
    int r  = luaL_checkinteger(L, 3);
    int on = luaL_checkinteger(L, 4);
    oled::draw_circle(cx, cy, r, (uint8_t)on);
    return 0;
}

// oled.fill_circle(cx, cy, r, on)
static int l_fill_circle(lua_State *L) {
    int cx = luaL_checkinteger(L, 1);
    int cy = luaL_checkinteger(L, 2);
    int r  = luaL_checkinteger(L, 3);
    int on = luaL_checkinteger(L, 4);
    oled::fill_circle(cx, cy, r, (uint8_t)on);
    return 0;
}

// oled.draw_progress(x, y, w, h, value, max_val, on)
static int l_draw_progress(lua_State *L) {
    int x       = luaL_checkinteger(L, 1);
    int y       = luaL_checkinteger(L, 2);
    int w       = luaL_checkinteger(L, 3);
    int h       = luaL_checkinteger(L, 4);
    int value   = luaL_checkinteger(L, 5);
    int max_val = luaL_checkinteger(L, 6);
    int on      = luaL_checkinteger(L, 7);
    oled::draw_progress(x, y, w, h, value, max_val, (uint8_t)on);
    return 0;
}

// oled.draw_statusbar(left, right [, separator])
static int l_draw_statusbar(lua_State *L) {
    const char *left      = luaL_checkstring(L, 1);
    const char *right     = luaL_optstring(L, 2, nullptr);
    bool        separator = lua_isboolean(L, 3) ? lua_toboolean(L, 3) : true;
    oled::draw_statusbar(left, right, separator);
    return 0;
}

// oled.draw_bitmap(x, page, w, w_pages, data)
// data = string of raw bytes (use string.char(...) or a binary literal)
static int l_draw_bitmap(lua_State *L) {
    int    x       = luaL_checkinteger(L, 1);
    int    page    = luaL_checkinteger(L, 2);
    int    w       = luaL_checkinteger(L, 3);
    int    w_pages = luaL_checkinteger(L, 4);
    size_t len;
    const char *data = luaL_checklstring(L, 5, &len);
    if ((int)len < w * w_pages)
        return luaL_error(L, "bitmap data too short");
    oled::draw_bitmap(x, page, w, w_pages, (const uint8_t *)data);
    return 0;
}

// oled.xor_bitmap(x, page, w, w_pages, data)
static int l_xor_bitmap(lua_State *L) {
    int    x       = luaL_checkinteger(L, 1);
    int    page    = luaL_checkinteger(L, 2);
    int    w       = luaL_checkinteger(L, 3);
    int    w_pages = luaL_checkinteger(L, 4);
    size_t len;
    const char *data = luaL_checklstring(L, 5, &len);
    if ((int)len < w * w_pages)
        return luaL_error(L, "bitmap data too short");
    oled::xor_bitmap(x, page, w, w_pages, (const uint8_t *)data);
    return 0;
}

// ── Constants exposed to Lua ──────────────────────────────────────────────────
static void push_constants(lua_State *L) {
    lua_pushinteger(L, oled::WIDTH);  lua_setfield(L, -2, "WIDTH");
    lua_pushinteger(L, oled::HEIGHT); lua_setfield(L, -2, "HEIGHT");
    lua_pushinteger(L, oled::PAGES);  lua_setfield(L, -2, "PAGES");
}

static const luaL_Reg LIB[] = {
    { "clear",               l_clear              },
    { "flush",               l_flush              },
    { "set_pixel",           l_set_pixel          },
    { "draw_string",         l_draw_string        },
    { "draw_string_centred", l_draw_string_centred},
    { "draw_string_right",   l_draw_string_right  },
    { "fill_rect",           l_fill_rect          },
    { "draw_rect",           l_draw_rect          },
    { "draw_hline",          l_draw_hline         },
    { "draw_vline",          l_draw_vline         },
    { "draw_line",           l_draw_line          },
    { "draw_circle",         l_draw_circle        },
    { "fill_circle",         l_fill_circle        },
    { "draw_progress",       l_draw_progress      },
    { "draw_statusbar",      l_draw_statusbar     },
    { "draw_bitmap",         l_draw_bitmap        },
    { "xor_bitmap",          l_xor_bitmap         },
    { nullptr, nullptr }
};

inline void open(lua_State *L) {
    luaL_newlib(L, LIB);
    push_constants(L);
    lua_setglobal(L, "oled");
}

} // namespace lua_oled