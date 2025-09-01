print("=== Lua Scripts ===")
scripts = getScripts()
if scripts then
    for _, name in pairs(scripts) do
        print("- " .. name)
    end
else
    print("No baked scripts available")
end