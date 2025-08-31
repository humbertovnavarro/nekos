import os

folder = "scripts"
output = "include/LuaScripts.h"

with open(output, "w") as f:
    f.write("// Auto-generated Lua scripts in PROGMEM\n\n")
    f.write("#pragma once\n#include <Arduino.h>\n#include <map>\n#include <pgmspace.h>\n#include \"esp_heap_caps.h\"\n\n")

    # Generate PROGMEM strings for each script
    for file in os.listdir(folder):
        if file.endswith(".lua"):
            path = os.path.join(folder, file)
            with open(path, "r") as lua:
                content = lua.read()
                content = content.replace("\\", "\\\\").replace("\"", "\\\"").replace("\n", "\\n\"\n\"")
                name = os.path.splitext(file)[0]
                f.write(f'static const char {name}_script[] PROGMEM = "{content}";\n')

    f.write("\n// Lua script map (runtime map pointing to flash strings)\n")
    f.write("static std::map<String, const char*>* luaScriptMap = nullptr;\n\n")

    # Init function to populate map in PSRAM if available
    f.write("static void initLuaScriptMap() {\n")
    f.write("    if (luaScriptMap) return;\n")
    f.write("    luaScriptMap = (std::map<String, const char*>*) heap_caps_malloc(\n")
    f.write("        sizeof(std::map<String, const char*>), MALLOC_CAP_SPIRAM\n")
    f.write("    );\n")
    f.write("    if (!luaScriptMap) {\n")
    f.write("        luaScriptMap = new std::map<String, const char*>(); // fallback to normal heap\n")
    f.write("    } else {\n")
    f.write("        new (luaScriptMap) std::map<String, const char*>(); // placement new in PSRAM\n")
    f.write("    }\n")

    for file in os.listdir(folder):
        if file.endswith(".lua"):
            name = os.path.splitext(file)[0]
            f.write(f'    (*luaScriptMap)["{name}"] = {name}_script;\n')
    f.write("}\n")
