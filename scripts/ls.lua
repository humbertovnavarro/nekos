-- List files in root
local files = FFat.ls()
for i, f in ipairs(files) do
    print(f)
end
