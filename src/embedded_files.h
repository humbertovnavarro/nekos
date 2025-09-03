#pragma once

#include <Arduino.h>

struct EmbeddedFile {
    const char* path;
    const unsigned char* data;
    const size_t size;
};

extern const EmbeddedFile embeddedFiles[];
extern const size_t embeddedFileCount;
