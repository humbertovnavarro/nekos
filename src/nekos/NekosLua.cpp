extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}
#include "FreeRTOS.h"
#include "task.h"
#include "Arduino.h"
#include "NekosLua.h"
struct LuaTaskData {
    lua_State* L;
    int ref; // Lua function reference
};

namespace nekos {

    void registerArduinoConstants(lua_State* L) {
        // Pin modes
        LUA_CONST(L, "INPUT", INPUT);
        LUA_CONST(L, "OUTPUT", OUTPUT);
        LUA_CONST(L, "INPUT_PULLUP", INPUT_PULLUP);

        // Digital values
        LUA_CONST(L, "HIGH", HIGH);
        LUA_CONST(L, "LOW", LOW);
    }

    void registerI2CConstants(lua_State* L) {
        LUA_CONST(L, "I2C_READ", 1);
        LUA_CONST(L, "I2C_WRITE", 0);
    }


        // --- Serial ---
    static int luaSerialWrite(lua_State *L) {
        size_t len;
        const char* s = luaL_tolstring(L, 1, &len);
        Serial.write(s, len);
        lua_pop(L, 1); // remove string from stack
        return 0;
    }

    static int luaSerialRead(lua_State *L) {
        if (Serial.available()) {
            lua_pushinteger(L, Serial.read());
            return 1;
        }
        lua_pushnil(L);
        return 1;
    }

    static int luaSerialReadLine(lua_State *L) {
        static String line;
        while (Serial.available()) {
            char c = Serial.read();
            if (c == '\n') {
                lua_pushstring(L, line.c_str());
                line = "";
                return 1;
            } else if (c != '\r') {
                line += c;
            }
        }
        lua_pushnil(L);
        return 1;
    }

    // --- I²C ---


    // print(string)
    static int luaSerialPrint(lua_State *L) {
        int nargs = lua_gettop(L);  // number of arguments
        for (int i = 1; i <= nargs; i++) {
            size_t len;
            const char *s = luaL_tolstring(L, i, &len); // convert to string
            if (s) {
                Serial.write(s, len);
                lua_pop(L, 1); // remove luaL_tolstring result
            }
            if (i < nargs) Serial.write('\t'); // tab between args
        }
        Serial.write('\n');
        return 0;
    }
    // delay(ms)
    static int luaDelay(lua_State *L) {
        int ms = luaL_checkinteger(L, 1);
        delay(ms);
        return 0;
    }

    // millis()
    static int luaMillis(lua_State *L) {
        lua_pushinteger(L, millis());
        return 1;
    }

    // pinMode(pin, mode)
    static int luaPinMode(lua_State *L) {
        int pin = luaL_checkinteger(L, 1);
        int mode = luaL_checkinteger(L, 2);
        pinMode(pin, mode);
        return 0;
    }

    // digitalWrite(pin, value)
    static int luaDigitalWrite(lua_State *L) {
        int pin = luaL_checkinteger(L, 1);
        int val = luaL_checkinteger(L, 2);
        digitalWrite(pin, val);
        return 0;
    }

    // digitalRead(pin) -> value
    static int luaDigitalRead(lua_State *L) {
        int pin = luaL_checkinteger(L, 1);
        lua_pushinteger(L, digitalRead(pin));
        return 1;
    }

    // analogWrite(pin, value)
    static int luaAnalogWrite(lua_State *L) {
        int pin = luaL_checkinteger(L, 1);
        int val = luaL_checkinteger(L, 2);
        analogWrite(pin, val);
        return 0;
    }

    // analogRead(pin) -> value
    static int luaAnalogRead(lua_State *L) {
        int pin = luaL_checkinteger(L, 1);
        lua_pushinteger(L, analogRead(pin));
        return 1;
    }

    // --- Register function helper ---
    static void registerFunction(lua_State *L, const char* name, lua_CFunction func) {
        lua_pushcfunction(L, func);
        lua_setglobal(L, name);
    }

    // FreeRTOS
    static int luaTaskDelay(lua_State* L) {
        int ms = luaL_checkinteger(L, 1);
        vTaskDelay(pdMS_TO_TICKS(ms));
        return 0;
    }

    void luaExec(const char* script) {
        lua_State* L = luaL_newstate();
        luaL_openlibs(L);
        // Override print
        registerFunction(L, "print", luaSerialPrint);
        // Arduino basics
        registerFunction(L, "delay", luaDelay);
        registerFunction(L, "millis", luaMillis);
        registerFunction(L, "pinMode", luaPinMode);
        registerFunction(L, "digitalWrite", luaDigitalWrite);
        registerFunction(L, "digitalRead", luaDigitalRead);
        registerFunction(L, "analogWrite", luaAnalogWrite);
        registerFunction(L, "analogRead", luaAnalogRead);
        // Serial helpers
        registerFunction(L, "serialWrite", luaSerialWrite);
        registerFunction(L, "serialRead", luaSerialRead);
        registerFunction(L, "serialReadLine", luaSerialReadLine);
        // I²C helpers
        // Constants
        registerArduinoConstants(L);
        registerI2CConstants(L);
        if (luaL_dostring(L, script) != LUA_OK) {
            Serial.printf("Lua Error: %s\n", lua_tostring(L, -1));
        }
        lua_close(L);
    }
}
