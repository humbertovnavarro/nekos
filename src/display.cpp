#include "display.hpp"
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, SCL, SDA);

// Lua wrappers
static int luaDisplayClear(lua_State* L) {
    u8g2.clearBuffer();
    return 0;
}

static int luaDisplayDrawStr(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    const char* str = luaL_checkstring(L, 3);

    u8g2.drawStr(x, y, str);
    return 0;
}

static int luaDisplaySendBuffer(lua_State* L) {
    u8g2.sendBuffer();
    return 0;
}

static int luaDisplaySetFont(lua_State* L) {
    const char* fontName = luaL_checkstring(L, 1);
    // Example: only supports a few built-in fonts; expand as needed
    if (strcmp(fontName, "u8g2_font_ncenB08_tr") == 0)
        u8g2.setFont(u8g2_font_ncenB08_tr);
    else if (strcmp(fontName, "u8g2_font_6x10_tr") == 0)
        u8g2.setFont(u8g2_font_6x10_tr);
    else
        Serial.println("Unknown font for Lua");
    return 0;
}

// Register Lua display functions
void luaOpenDisplayLibs(lua_State* L) {
    lua_newtable(L);

    lua_pushcfunction(L, luaDisplayClear);
    lua_setfield(L, -2, "clear");

    lua_pushcfunction(L, luaDisplayDrawStr);
    lua_setfield(L, -2, "drawStr");

    lua_pushcfunction(L, luaDisplaySendBuffer);
    lua_setfield(L, -2, "sendBuffer");

    lua_pushcfunction(L, luaDisplaySetFont);
    lua_setfield(L, -2, "setFont");

    lua_setglobal(L, "display");
}
