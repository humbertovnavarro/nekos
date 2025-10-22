#pragma once
#include "Arduino.h"
#include "FFat.h"

#define LUA_COMPILE_BUFFER_SIZE 512

// Compile a single Lua file to .luac if needed
bool compileLuaFileIfNeeded(const char* basePath);

// Recursively enumerate a directory and compile all .lua files
bool enumerateAndCompileLuaFiles(const char* basePath);
