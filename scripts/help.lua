print("\n=== Lua Scripts ===")
if bakedScripts then
    for name, _ in pairs(bakedScripts) do
        print("- " .. name)
    end
else
    print("No baked scripts available")
end