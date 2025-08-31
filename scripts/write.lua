-- Write string to a file (overwrite)
local path, data = ...
if not path or not data then
    print("Usage: write <file> <data>")
    return
end

if FFat.write(path, data) then
    print("Written to: " .. path)
else
    print("Failed to write: " .. path)
end
