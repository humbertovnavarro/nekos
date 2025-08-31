// Auto-generated Lua scripts in PROGMEM

#pragma once
#include <Arduino.h>
#include <map>
#include <pgmspace.h>
#include "esp_heap_caps.h"

static const char help_script[] PROGMEM = "print(\"\\n=== Lua Scripts ===\")\n"
"if bakedScripts then\n"
"    for name, _ in pairs(bakedScripts) do\n"
"        print(\"- \" .. name)\n"
"    end\n"
"else\n"
"    print(\"No baked scripts available\")\n"
"end";
static const char snake_script[] PROGMEM = "-- Snake game for serial\n"
"-- Controls: w = up, s = down, a = left, d = right, q = quit\n"
"\n"
"local width, height = 20, 10\n"
"local snake = { {x = 5, y = 5} }\n"
"local dir = {x = 1, y = 0}\n"
"local food = {x = math.random(width), y = math.random(height)}\n"
"local running = true\n"
"local score = 0\n"
"\n"
"-- Draw the game board\n"
"local function draw()\n"
"    print(\"\\nScore: \" .. score)\n"
"    for y = 1, height do\n"
"        local row = {}\n"
"        for x = 1, width do\n"
"            local ch = \" \"\n"
"            if x == food.x and y == food.y then\n"
"                ch = \"*\"\n"
"            end\n"
"            for i, seg in ipairs(snake) do\n"
"                if seg.x == x and seg.y == y then\n"
"                    ch = (i == 1) and \"O\" or \"o\"\n"
"                end\n"
"            end\n"
"            row[#row+1] = ch\n"
"        end\n"
"        print(table.concat(row))\n"
"    end\n"
"end\n"
"\n"
"-- Move snake forward\n"
"local function step()\n"
"    local head = snake[1]\n"
"    local newHead = {x = head.x + dir.x, y = head.y + dir.y}\n"
"\n"
"    -- Wrap around edges\n"
"    if newHead.x < 1 then newHead.x = width end\n"
"    if newHead.x > width then newHead.x = 1 end\n"
"    if newHead.y < 1 then newHead.y = height end\n"
"    if newHead.y > height then newHead.y = 1 end\n"
"\n"
"    -- Check collision with self\n"
"    for i = 1, #snake do\n"
"        if snake[i].x == newHead.x and snake[i].y == newHead.y then\n"
"            running = false\n"
"            return\n"
"        end\n"
"    end\n"
"\n"
"    table.insert(snake, 1, newHead)\n"
"\n"
"    if newHead.x == food.x and newHead.y == food.y then\n"
"        score = score + 1\n"
"        food = {x = math.random(width), y = math.random(height)}\n"
"    else\n"
"        table.remove(snake)\n"
"    end\n"
"end\n"
"\n"
"-- Handle input\n"
"local function handleInput(c)\n"
"    if c == \"w\" then dir = {x = 0, y = -1}\n"
"    elseif c == \"s\" then dir = {x = 0, y = 1}\n"
"    elseif c == \"a\" then dir = {x = -1, y = 0}\n"
"    elseif c == \"d\" then dir = {x = 1, y = 0}\n"
"    elseif c == \"q\" then running = false end\n"
"end\n"
"\n"
"-- Main loop\n"
"print(\"Snake Game! Use WASD to move, Q to quit.\")\n"
"while running do\n"
"    draw()\n"
"    local c = serial.read(1)  -- ⬅️ needs a binding that reads 1 char from serial\n"
"    if c then handleInput(c) end\n"
"    step()\n"
"    tmr.delay(200000)  -- sleep 200ms (ESP32-specific; adjust as needed)\n"
"end\n"
"\n"
"print(\"Game Over! Final score: \" .. score)\n"
"";

// Lua script map (runtime map pointing to flash strings)
static std::map<String, const char*>* luaScriptMap = nullptr;

static void initLuaScriptMap() {
    if (luaScriptMap) return;
    luaScriptMap = (std::map<String, const char*>*) heap_caps_malloc(
        sizeof(std::map<String, const char*>), MALLOC_CAP_SPIRAM
    );
    if (!luaScriptMap) {
        luaScriptMap = new std::map<String, const char*>(); // fallback to normal heap
    } else {
        new (luaScriptMap) std::map<String, const char*>(); // placement new in PSRAM
    }
    (*luaScriptMap)["help"] = help_script;
    (*luaScriptMap)["snake"] = snake_script;
}
