-- Remove a file
local path = ...
if not path then
    print("Usage: rm <file>")
    return
end

if FFat.exists(path) then
    if FFat.remove(path) then
        print("Deleted: " .. path)
    else
        print("Failed to delete: " .. path)
    end
else
    print("File not found: " .. path)
end
