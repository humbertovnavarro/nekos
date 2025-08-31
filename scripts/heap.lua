local ok, output = run_command("heap")
if ok then
    print("Heap info: ", output)
else
    print("Error running command: ", output)
end
