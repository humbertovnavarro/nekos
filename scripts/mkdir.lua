-- Create a pseudo-directory (dummy file)
local path = ...
if not path then
    print("Usage: mkdir <dir>")
    return
end

local dummy = path .. "/.keep"
if FFat.write(dummy, "") then
    print("Created dir: " .. path)
else
    print("Failed to create dir: " .. path)
end
