-- clear.lua
-- Clears the serial console

local function clear()
    -- ANSI escape codes: clear screen and move cursor home
    Serial.print("\27[2J")  -- Clear entire screen
    Serial.print("\27[H")   -- Move cursor to top-left
end

clear()
