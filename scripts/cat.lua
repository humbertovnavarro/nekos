-- Read file content and print
local path = ...  -- first argument
if not path then
    print("Usage: cat <file>")
    return
end

if not FFat.exists(path) then
    print("File not found: " .. path)
    return
end

local content = FFat.read(path)
print(content)
