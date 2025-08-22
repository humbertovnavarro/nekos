---

# NekosLua – Embedded Lua API for Arduino/ESP32

This module lets you run Lua scripts on an Arduino/ESP32 environment with access to hardware (GPIO, Serial, I²C) and FreeRTOS helpers.

---

## Basics

### `print(...)`

* Works like Lua’s built-in `print`, but writes to the Arduino `Serial`.
* Multiple arguments are separated by **tabs**.
* Appends a newline at the end.

```lua
print("Hello", "world", 123)
```

---

## Timing

### `delay(ms)`

* Pause execution for `ms` milliseconds (blocking).

```lua
delay(1000) -- wait 1 second
```

### `millis() -> integer`

* Returns milliseconds since boot.

```lua
local t = millis()
print("Uptime:", t)
```

### `taskDelay(ms)`

* FreeRTOS aware delay (uses `vTaskDelay` internally).
* Useful when running inside multitasked environments.

```lua
taskDelay(500)
```

---

## GPIO

### `pinMode(pin, mode)`

* Set pin mode.
* Modes: `INPUT`, `OUTPUT`, `INPUT_PULLUP`.

```lua
pinMode(13, OUTPUT)
```

### `digitalWrite(pin, value)`

* Write HIGH/LOW to a digital pin.
* Values: `HIGH`, `LOW`.

```lua
digitalWrite(13, HIGH)
```

### `digitalRead(pin) -> integer`

* Read digital pin value (`0` or `1`).

```lua
if digitalRead(12) == HIGH then
    print("Button pressed")
end
```

### `analogWrite(pin, value)`

* Write PWM/analog value (0–255 typically).

```lua
analogWrite(5, 128)
```

### `analogRead(pin) -> integer`

* Read analog value from a pin.

```lua
local v = analogRead(34)
print("ADC:", v)
```

---

## Serial

### `serialWrite(data)`

* Write raw data/string to serial.

```lua
serialWrite("Hello!\n")
```

### `serialRead() -> integer|nil`

* Read one byte from serial, or `nil` if nothing available.

```lua
local b = serialRead()
if b then print("Got byte:", b) end
```

### `serialReadLine() -> string|nil`

* Reads a full line (ending with `\n`), returns `nil` if not complete.

```lua
local line = serialReadLine()
if line then print("Got line:", line) end
```

---

## I²C

### `i2cBegin(sda, scl)`

* Initialize I²C on given SDA/SCL pins.

```lua
i2cBegin(21, 22)
```

### `i2cWrite(addr, byte1, byte2, ...)`

* Write bytes to an I²C device at `addr`.

```lua
i2cWrite(0x3C, 0x00, 0xAF) -- example OLED command
```

### `i2cRead(addr, len) -> string`

* Read `len` bytes from an I²C device, returns as a Lua string.

```lua
local data = i2cRead(0x50, 16)
print("EEPROM:", data)
```

---

## Commands

### `runCommand(name, args) -> boolean`

* Runs a registered command from C++ `CommandRegistry`.
* `name`: command name.
* `args`: optional string of arguments.
* Returns `true` on success, `false` on failure.

```lua
-- Example: LED control
runCommand("led", "13 HIGH")
delay(500)
runCommand("led", "13 LOW")
```

---

## Constants

* `INPUT`, `OUTPUT`, `INPUT_PULLUP`
* `HIGH`, `LOW`
* `I2C_READ`, `I2C_WRITE`

---

## Example Script

```lua
print("Starting...")

pinMode(13, OUTPUT)

for i=1,5 do
    digitalWrite(13, HIGH)
    delay(500)
    digitalWrite(13, LOW)
    delay(500)
end

print("Blink done!")
```

---
