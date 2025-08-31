-- Append string to a file
local path, data = ...
if not path or not data then
    print("Usage: append <file> <data>")
    return
end

if FFat.append(path, data) then
    print("Appended to: " .. path)
else
    print("Failed to append: " .. path)
end
