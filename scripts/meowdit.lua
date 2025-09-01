-- Serial Lua Text Editor with SPIFFS I/O
-- Commands:
--  :w          save buffer to file
--  :q          quit
--  :wq         save and quit
--  :ls         list files
--  :load <file> load file
--  Ctrl+C      quit

local buffer = {}
local filename = ...

local f, err = io.open(filename, "r")
if not f then
    Serial.println("Error opening file: " .. (err or "unknown"))
    return false
end

-- Clear screen
local function cls()
    Serial.print(string.char(27) .. "[2J" .. string.char(27) .. "[H")
end

-- Draw editor UI
local function draw()
    cls()
    Serial.println("Lua Serial Editor - " .. filename)
    Serial.println("Commands: :w, :q, :wq, :ls, :load <file>")
    Serial.println("=================================")
    for i, line in ipairs(buffer) do
        Serial.println(i .. ": " .. line)
    end
    Serial.print("> ")
end

-- Save buffer to file
local function saveFile()
    local f, err = io.open(filename, "w")
    if not f then
        Serial.println("Error saving file: " .. (err or "unknown"))
        return false
    end
    for _, line in ipairs(buffer) do
        f:write(line, "\n")
    end
    f:close()
    Serial.println("File saved: " .. filename)
    return true
end

-- Load file into buffer
local function loadFile(name)
    local f, err = io.open(name, "r")
    if not f then
        Serial.println("Error opening file: " .. (err or "unknown"))
        return false
    end
    buffer = {}
    while true do
        local line = f:read("*l")
        if not line then break end
        table.insert(buffer, line)
    end
    f:close()
    filename = name
    Serial.println("File loaded: " .. filename)
    return true
end

-- List files in SPIFFS
local function listFiles()
    local files = io.ls("/")
    if not files then
        Serial.println("No files found.")
        return
    end
    Serial.println("Files:")
    for i, name in ipairs(files) do
        Serial.println("  " .. name)
    end
end

-- Read line from Serial
local function readLine()
    local line = ""
    while true do
        while Serial.available() > 0 do
            local c = Serial.read()
            if c == 13 or c == 10 then  -- CR/LF
                if #line > 0 then return line end
            else
                line = line .. string.char(c)
            end
        end
    end
end

-- Main editor loop
local running = true
while running do
    draw()
    local line = readLine()
    if not line then break end

    if line == ":q" then
        running = false
    elseif line == ":w" then
        saveFile()
    elseif line == ":wq" then
        if saveFile() then running = false end
    elseif line == ":ls" then
        listFiles()
    elseif line:match("^:load%s+") then
        local name = line:match("^:load%s+(.+)")
        if name then loadFile(name) end
    else
        table.insert(buffer, line)
    end
end

Serial.println("Editor exited.")
