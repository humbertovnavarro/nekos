#pragma once
#include <stdint.h>
#include "ssd1306.h"

#ifndef MENU_VISIBLE_ROWS
#define MENU_VISIBLE_ROWS 8
#endif

namespace oled {

struct Menu;

typedef void (*MenuAction)();

struct MenuItem {
    const char *label;
    MenuAction action;
    Menu *submenu;
};

struct MenuStyle {
    int  x = 0;
    int  width = 128;
    int  page_start = 0;
    int  visible_rows = MENU_VISIBLE_ROWS;
    bool show_scrollbar = true;
    bool show_icons = true;
    bool wrap = true;
    bool show_back = true;
    const char* back_label = "Back";
};

struct Menu {

    MenuItem *items = nullptr;
    uint8_t   count = 0;
    MenuStyle style {};

    Menu *active_submenu = nullptr;
    Menu *parent = nullptr;

    uint8_t cursor = 0;
    uint8_t scroll = 0;

    bool open = true;

    Menu() = default;

    Menu(MenuItem *items, uint8_t count, MenuStyle s = {})
        : items(items), count(count), style(s) {}

    void set_items(MenuItem *i, uint8_t c) {
        items = i;
        count = c;
        cursor = 0;
        scroll = 0;
        active_submenu = nullptr;
    }

    int total_items() const {
        return count + (style.show_back ? 1 : 0);
    }

    bool is_back_row(int idx) const {
        return style.show_back && idx == (int)count;
    }

    bool up() {
        if (active_submenu) return active_submenu->up();

        int total = total_items();
        if (cursor > 0)
            cursor--;
        else if (style.wrap && total)
            cursor = total - 1;

        clamp_scroll();
        return true;
    }

    bool down() {
        if (active_submenu) return active_submenu->down();

        int total = total_items();
        if (cursor < total - 1)
            cursor++;
        else if (style.wrap && total)
            cursor = 0;

        clamp_scroll();
        return true;
    }

    bool confirm() {
        if (active_submenu)
            return active_submenu->confirm();

        if (!total_items()) return false;

        if (is_back_row(cursor))
            return back();

        MenuItem &item = items[cursor];

        if (item.submenu) {
            item.submenu->parent = this;
            active_submenu = item.submenu;
            active_submenu->cursor = 0;
            active_submenu->scroll = 0;
            active_submenu->open = true;
            return true;
        }

        if (item.action) {
            item.action();
            return true;
        }

        return false;
    }

    bool back() {
        if (active_submenu) {
            if (active_submenu->active_submenu)
                return active_submenu->back();

            active_submenu->open = false;
            active_submenu = nullptr;
            return true;
        }

        open = false;
        return false;
    }

    void draw() {
        if (!open) return;

        if (active_submenu) {
            active_submenu->draw();
            return;
        }

        constexpr int ROW_H = 8;
        constexpr int ICON_W = 6;
        constexpr int ARROW_W = 6;

        int total = total_items();

        int SBAR_W =
            (style.show_scrollbar && total > style.visible_rows) ? 3 : 0;

        int text_x = style.x + (style.show_icons ? ICON_W : 0);

        for (int row = 0; row < style.visible_rows; row++) {

            int idx = scroll + row;
            int page = style.page_start + row;
            int py = page * ROW_H;

            if (idx >= total) {
                fill_rect(style.x, py, style.width, ROW_H, 0);
                continue;
            }

            bool sel = (idx == (int)cursor);
            bool is_back = is_back_row(idx);

            const char *label = is_back ? style.back_label : items[idx].label;
            bool has_sub = !is_back && items[idx].submenu;

            if (sel) {
                fill_rect(style.x, py, style.width - SBAR_W, ROW_H, 1);

                if (style.show_icons)
                    draw_char(style.x, page, is_back ? '<' : '>', true);

                draw_string(text_x, page, label, true);

                if (has_sub && style.show_icons)
                    draw_char(style.x + style.width - SBAR_W - ARROW_W,
                              page, '>', true);
            } else {
                fill_rect(style.x, py, style.width - SBAR_W, ROW_H, 0);

                if (style.show_icons && is_back)
                    draw_char(style.x, page, '<', false);

                draw_string(text_x, page, label, false);

                if (has_sub && style.show_icons)
                    draw_char(style.x + style.width - SBAR_W - ARROW_W,
                              page, '>', false);
            }
        }

        draw_scrollbar(SBAR_W, total);
    }

private:

    void clamp_scroll() {
        int total = total_items();
        int vis = style.visible_rows;
        if (cursor < scroll)
            scroll = cursor;
        else if (cursor >= scroll + vis)
            scroll = cursor - vis + 1;
        int max_scroll = total > vis ? total - vis : 0;
        if (scroll > max_scroll) scroll = max_scroll;
    }

    void draw_scrollbar(int sbar_w, int total) {
        if (!sbar_w) return;

        int bar_x = style.x + style.width - sbar_w;
        int bar_py = style.page_start * 8;
        int bar_h = style.visible_rows * 8;

        fill_rect(bar_x, bar_py, 1, bar_h, 1);
        fill_rect(bar_x + 1, bar_py, sbar_w - 1, bar_h, 0);

        int thumb_h = bar_h * style.visible_rows / total;
        if (thumb_h < 3) thumb_h = 3;

        int thumb_y =
            bar_py + (bar_h - thumb_h) * scroll / (total - style.visible_rows);

        fill_rect(bar_x + 1, thumb_y, sbar_w - 1, thumb_h, 1);
    }
};

} // namespace oled