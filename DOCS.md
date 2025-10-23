---

# Lua Modules with Examples

## Table of Contents

* [freertos](#freertos)
* [sys](#sys)
* [fs](#fs)
* [display](#display)
* [neopixel](#neopixel)
* [serial](#serial)
* [input](#input)
* [gpio](#gpio)

---

## `freertos`

Provides FreeRTOS timing utilities.

```lua
local freertos = require("freertos")

-- delay 500 ms
freertos.delay(500)

-- delay 10 RTOS ticks
freertos.delayTicks(10)
```

| Function            | Parameters       | Returns | Description                             |
| ------------------- | ---------------- | ------- | --------------------------------------- |
| `delay(ms)`         | `ms: integer`    | none    | Delay execution for `ms` milliseconds.  |
| `delayTicks(ticks)` | `ticks: integer` | none    | Delay execution for `ticks` RTOS ticks. |

---

## `sys`

Provides system info for ESP32/Arduino.

```lua
local sys = require("sys")

print("Uptime:", sys.uptime())
print("Free heap:", sys.freeHeap())
print("CPU cores:", sys.cpuCores())
print("CPU freq:", sys.cpuFreqMHz())
print("Chip ID:", sys.chipId())
print("Flash size:", sys.flashSize())
print("SDK version:", sys.sdkVersion())
```

| Function              | Returns   | Description               |
| --------------------- | --------- | ------------------------- |
| `uptime()`            | `integer` | Milliseconds since boot   |
| `freeHeap()`          | `integer` | Free heap in bytes        |
| `heapFragmentation()` | `number`  | Largest free memory block |
| `cpuCores()`          | `integer` | Number of CPU cores       |
| `cpuFreqMHz()`        | `integer` | CPU frequency in MHz      |
| `chipId()`            | `integer` | ESP32 chip MAC (or 0)     |
| `version()`           | `string`  | Board variant             |
| `flashSize()`         | `integer` | Flash size in bytes       |
| `sdkVersion()`        | `string`  | SDK version               |

---

## `fs`

Filesystem operations with FFat.

```lua
local fs = require("fs")

fs.mount(true)

-- Write and read
fs.write("/test.txt", "Hello Lua!")
print(fs.read("/test.txt"))

-- List directory
local entries = fs.list("/")
for i, entry in ipairs(entries) do
    print(entry.name, entry.size, entry.isDir)
end

fs.unmount()
```

| Function              | Parameters     | Returns       | Description                        |
| --------------------- | -------------- | ------------- | ---------------------------------- |
| `mount(formatIfFail)` | boolean        | boolean       | Mount FFat, optionally format      |
| `unmount()`           | none           | none          | Unmount FFat                       |
| `exists(path)`        | string         | boolean       | Check file/dir exists              |
| `remove(path)`        | string         | boolean       | Delete file                        |
| `mkdir(path)`         | string         | boolean       | Create directory                   |
| `rmdir(path)`         | string         | boolean       | Remove directory                   |
| `rename(from, to)`    | string, string | boolean       | Rename/move file/dir               |
| `read(path)`          | string         | string or nil | Read file contents                 |
| `write(path, data)`   | string, string | boolean       | Write file contents                |
| `append(path, data)`  | string, string | boolean       | Append file contents               |
| `list([path])`        | string         | table         | List entries `{name, size, isDir}` |

---

## `display`

Control U8g2 OLED.

```lua
local display = require("display")

display.clear()
display.setFont("u8g2_font_ncenB08_tr")
display.drawStr(0, 10, "Hello World")
display.drawBox(0, 20, 50, 10)
display.drawFrame(0, 40, 50, 10)
display.sendBuffer()
```

| Function                | Description            |
| ----------------------- | ---------------------- |
| `clear()`               | Clear buffer           |
| `drawStr(x, y, str)`    | Draw text              |
| `sendBuffer()`          | Push buffer to display |
| `setFont(fontName)`     | Set font               |
| `drawBox(x, y, w, h)`   | Draw filled box        |
| `drawFrame(x, y, w, h)` | Draw rectangle frame   |

---

## `neopixel`

Control WS2812 LEDs.

```lua
local np = require("neopixel")

for i=0, np.numPixels()-1 do
    np.setPixel(i, 255, 0, 0)
end
np.show()
np.clear()
```

| Function                   | Description          |
| -------------------------- | -------------------- |
| `setPixel(index, r, g, b)` | Set LED color        |
| `show()`                   | Update LEDs          |
| `clear()`                  | Turn off all LEDs    |
| `setBrightness(level)`     | Set brightness 0–255 |
| `numPixels()`              | Number of LEDs       |

---

## `serial`

Serial communication.

```lua
local serial = require("serial")

serial.println("Hello")
serial.print(123)
local b = serial.read()
local line = serial.readLine()
local str = serial.readString(10)
```

| Function            | Description                         |
| ------------------- | ----------------------------------- |
| `print(value)`      | Print without newline               |
| `println(str)`      | Print with newline                  |
| `available()`       | Bytes available                     |
| `read()`            | Read one byte                       |
| `readLine()`        | Read until newline                  |
| `readString([len])` | Read string (optional length limit) |

---

## `input`

Read GPIO buttons with debounce.

```lua
local input = require("input")

local dir = input.read()
print("Direction:", dir) -- "up", "down", "left", "right", "none"
```

| Function | Description                                             |
| -------- | ------------------------------------------------------- |
| `read()` | Returns `"up"`, `"down"`, `"left"`, `"right"`, `"none"` |

---

## `gpio`

General-purpose pin control.

```lua
local gpio = require("gpio")

gpio.mode(5, "output")
gpio.write(5, true)
print(gpio.read(5))
gpio.toggle(5)
```

| Function            | Description                                           |
| ------------------- | ----------------------------------------------------- |
| `mode(pin, mode)`   | Set pin mode: `"input"`, `"output"`, `"input_pullup"` |
| `write(pin, value)` | Write pin HIGH/LOW                                    |
| `read(pin)`         | Read pin state                                        |
| `toggle(pin)`       | Toggle pin state                                      |

---