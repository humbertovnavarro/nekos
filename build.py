import os

folder = "scripts"
output = "include/LuaScripts.h"

with open(output, "w") as f:
    f.write("// Auto-generated Lua scripts\n\n")
    f.write("#pragma once\n#include <Arduino.h>\n#include <map>\n\n")
    f.write("static std::map<String, const char*> luaScriptMap = {\n")

    for file in os.listdir(folder):
        if file.endswith(".lua"):
            path = os.path.join(folder, file)
            with open(path, "r") as lua:
                content = lua.read()
                # Escape backslashes, quotes, and newlines
                content = content.replace("\\", "\\\\").replace("\"", "\\\"").replace("\n", "\\n\"\n\"")
                name = os.path.splitext(file)[0]
                f.write(f'    {{"{name}", "{content}"}},\n')

    f.write("};\n")
