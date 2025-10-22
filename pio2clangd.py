#!/usr/bin/env python3
"""
Generate .clangd from .vscode/c_cpp_properties.json and platformio.ini,
with system includes for Arduino/FreeRTOS to suppress indirect include warnings.
"""

import json, re, os
from pathlib import Path
import configparser

workspace = Path(__file__).parent
vscode_path = workspace / ".vscode/c_cpp_properties.json"
pio_ini_path = workspace / "platformio.ini"
clangd_path = workspace / ".clangd"
settings_path = workspace / ".vscode/settings.json"

# --- Load JSON with comment stripping ---
def load_json_with_comments(path: Path):
    text = path.read_text(encoding="utf-8").strip()
    text = re.sub(r"//.*", "", text)
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    return json.loads(text)

# --- Parse platformio.ini to get board name ---
def detect_board_from_ini(path: Path):
    parser = configparser.ConfigParser()
    parser.read(path)
    for section in parser.sections():
        if parser.has_option(section, "board"):
            return parser.get(section, "board").strip()
    return None

# --- Map board -> HAL variant ---
def map_board_to_variant(board: str):
    board = board.lower()
    if "s3" in board:
        return "esp32s3"
    elif "c3" in board:
        return "esp32c3"
    elif "s2" in board:
        return "esp32s2"
    else:
        return "esp32"

# --- Detect include paths ---
def detect_includes(board_variant: str):
    includes = []
    system_includes = []
    pio = Path.home() / ".platformio/packages"

    # Arduino framework includes
    for fw in pio.glob("framework-arduinoespressif32"):
        for d in ["cores/esp32", f"variants/{board_variant}", "libraries"]:
            path = fw / d
            if path.exists():
                includes.append(f"-I{path}")
                system_includes.append(f"-isystem {path}")

    # ESP-IDF FreeRTOS includes
    for fw in pio.glob("framework-espidf"):
        base = fw / "components/freertos/include"
        if base.exists():
            system_includes.append(f"-isystem {base}")

    # ESP-IDF general includes
    for fw in pio.glob("framework-espidf"):
        base = fw / "components"
        subdirs = [
            "hal/include",
            f"hal/{board_variant}/include",
            "esp_common/include",
            "esp_hw_support/include",
            "esp_rom/include",
            "soc/include",
            f"soc/{board_variant}/include",
        ]
        for s in subdirs:
            p = base / s
            if p.exists():
                includes.append(f"-I{p}")
    return includes, system_includes

# --- Load c_cpp_properties.json ---
data = load_json_with_comments(vscode_path)
cfg = data.get("configurations", [])[0]
include_paths = cfg.get("includePath", [])
defines = cfg.get("defines", [])
c_std = cfg.get("cStandard", "gnu11")
cpp_std = cfg.get("cppStandard", "gnu++17")

# --- Detect PlatformIO board ---
board_name = detect_board_from_ini(pio_ini_path)
variant = map_board_to_variant(board_name or "esp32")
print(f"🔍 Detected board: {board_name or 'unknown'} → variant: {variant}")

# --- Normalize user include paths ---
normalized_includes = [
    f"-I{os.path.expanduser(p).replace('${workspaceFolder}', '${workspaceFolder}')}"
    for p in include_paths
]

# --- Auto-detect includes ---
auto_includes, system_includes = detect_includes(variant)
normalized_includes.extend(auto_includes)

# --- Forced HAL include based on board variant ---
hal_include_map = {
    "esp32s3": "esp32-hal.h",
    "esp32c3": "esp32c3-hal.h",
    "esp32s2": "esp32s2-hal.h",
    "esp32": "esp32-hal.h",
}
forced_include = f"-include {hal_include_map.get(variant, 'esp32-hal.h')}"

# --- Write .clangd ---
lines = [
    "CompileFlags:",
    "  Add:",
    f"    - -std={cpp_std}",
    "    - -Wall",
    "    - -Wextra",
    "    - -Wno-unknown-warning-option",
    "    - -Wno-pragma-once-outside-header",
]

# Add defines
for d in defines:
    lines.append(f"    - -D{d}")

# Add normal includes
for inc in normalized_includes:
    lines.append(f"    - {inc}")

# Add system includes to suppress Arduino/FreeRTOS warnings
for sys_inc in system_includes:
    lines.append(f"    - {sys_inc}")

# Add forced HAL include
lines.append(f"    - {forced_include}")

# Cleanup and fallback flags
lines += [
    "  Remove: [-m*, -f*, -O*]",
    "  CompilationDatabase: .",
    "Diagnostics:",
    "  UnusedIncludes: Strict",
    "  Suppress: [unknown-warning-option]",
    "Index:",
    "  Background: true",
    "  StandardLibrary: Yes",
    "FallbackFlags:",
    f"  - -std={cpp_std}",
    "---",
    "If:",
    "  PathMatch: .*\\.c$",
    "CompileFlags:",
    f"  Add: [-std={c_std}]",
    "---",
    "If:",
    "  PathMatch: .*\\.cpp$",
    "CompileFlags:",
    f"  Add: [-std={cpp_std}]",
]

clangd_path.write_text("\n".join(lines) + "\n", encoding="utf-8")
print(f"✅ .clangd generated successfully at {clangd_path}")

# --- Update settings.json ---
clangd_config = {
    "clangd.arguments": [
        "--compile-commands-dir=.",
        "--background-index",
        "--header-insertion=never",
        "--pch-storage=memory"
    ],
    "C_Cpp.intelliSenseEngine": "disabled",
    "editor.defaultFormatter": "llvm-vs-code-extensions.vscode-clangd"
}

if settings_path.exists():
    try:
        settings = json.loads(settings_path.read_text(encoding="utf-8"))
    except json.JSONDecodeError:
        settings = {}
else:
    settings = {}

settings.update(clangd_config)
settings_path.parent.mkdir(parents=True, exist_ok=True)
settings_path.write_text(json.dumps(settings, indent=2), encoding="utf-8")
print(f"✅ Updated {settings_path} with clangd settings")
