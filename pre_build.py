import os
import subprocess
import sys

INPUT_DIR = "webrepl/build"
OUTPUT_CPP = "src/embedded_files.cpp"
OUTPUT_H = "src/embedded_files.h"

def to_cpp_identifier(path):
    return path.replace("/", "_").replace(".", "_").replace("-", "_")

def bytes_to_cpp_array(data):
    return ', '.join(f'0x{b:02x}' for b in data)

def run_bun_build():
    print("🚀 Running 'bun run build' in webrepl...")
    result = subprocess.run(["bun", "run", "build"], cwd="webrepl")
    if result.returncode != 0:
        print("❌ Bun build failed!")
        sys.exit(1)
    print("✅ Bun build completed successfully.")

def main():
    # Step 1: Build frontend
    run_bun_build()

    # Step 2: Collect files
    files = []
    for root, _, filenames in os.walk(INPUT_DIR):
        for filename in filenames:
            full_path = os.path.join(root, filename)
            rel_path = os.path.relpath(full_path, INPUT_DIR).replace("\\", "/")
            files.append((rel_path, full_path))

    # Step 3: Generate C++ header and source
    with open(OUTPUT_H, "w") as h_file, open(OUTPUT_CPP, "w") as cpp_file:
        h_file.write("#pragma once\n\n")
        h_file.write("#include <Arduino.h>\n\n")
        h_file.write("struct EmbeddedFile {\n")
        h_file.write("    const char* path;\n")
        h_file.write("    const unsigned char* data;\n")
        h_file.write("    const size_t size;\n")
        h_file.write("};\n\n")
        h_file.write("extern const EmbeddedFile embeddedFiles[];\n")
        h_file.write("extern const size_t embeddedFileCount;\n")

        cpp_file.write('#include "embedded_files.h"\n\n')

        for rel_path, full_path in files:
            with open(full_path, "rb") as f:
                data = f.read()
            identifier = to_cpp_identifier(rel_path)
            cpp_file.write(f"const unsigned char {identifier}[] PROGMEM = {{ {bytes_to_cpp_array(data)} }};\n")
            cpp_file.write(f"const size_t {identifier}_len = sizeof({identifier});\n\n")

        cpp_file.write("const EmbeddedFile embeddedFiles[] = {\n")
        for rel_path, _ in files:
            identifier = to_cpp_identifier(rel_path)
            cpp_file.write(f'    {{"{rel_path}", {identifier}, {identifier}_len}},\n')
        cpp_file.write("};\n\n")
        cpp_file.write(f"const size_t embeddedFileCount = {len(files)};\n")

    print(f"✅ Generated {OUTPUT_CPP} and {OUTPUT_H} with {len(files)} embedded file(s) using PROGMEM.")

if __name__ == "__main__":
    main()
