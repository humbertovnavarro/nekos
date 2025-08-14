#include "Arduino.h"
#define STDIO_BUFFER_COUNT 4
#define STDIO_BUFFER_SIZE 32
#define SHELL_INPUT_BUFFER_SIZE 128
#define SHELL_QUEUE_MSG_SIZE 512
#define MAX_NUM_APPS 32
#define MAX_REGISTRY_ENTRIES 512
#define MAX_REGISTRY_KEY_LEN 16
#define MAX_REGISTRY_VALUE_LEN 32
#ifdef ESP32
  #define PSMALLOC_OR_MALLOC(size) \
    ((psramFound()) ? ps_malloc(size) : malloc(size))
#else
  #define PSMALLOC_OR_MALLOC(size) malloc(size)
#endif
