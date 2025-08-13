#include "FreeRTOS.h"

namespace nekos {
    using CommandCallback = void(*)(const char* args);
    class Console {
    public:
        static constexpr int MAX_COMMANDS = 64;
        struct Command {
            char name[32];
            CommandCallback cb;
        };
        static Command commands[MAX_COMMANDS];
        static constexpr int SHELL_INPUT_BUFFER_SIZE = 256;
        static constexpr int QUEUE_MSG_SIZE = 256;
        // Environment variables support
        static constexpr int MAX_ENV_VARS = 32;
        static constexpr int MAX_ENV_NAME_LEN = 32;
        static constexpr int MAX_ENV_VALUE_LEN = 64;
        static constexpr int MAX_LOG_CALLBACKS = 64;
        static void exec(const char* command, const char* args);
        // Initialize serial console at given baud rate
        static void begin(unsigned long baud = 115200);
        // Call repeatedly in your loop()
        static void poll();
        // Register a command by name and callback (callback receives args string)
        static bool registerCommand(const char* name, CommandCallback cb);
        // Helpers for listing commands
        static int getCommandCount();
        static const char* getCommandName(int index);
        // Logging helpers
        static void log(const char *message);
        static void logf(const char *fmt, ...);
        // Environment variable API
        static bool setEnv(const char* name, const char* value);
        static const char* getEnv(const char* name);
        static bool unsetEnv(const char* name);
    private:
        struct EnvVar {
            char name[MAX_ENV_NAME_LEN];
            char value[MAX_ENV_VALUE_LEN];
            bool inUse = false;
        };
        static void printPrompt();
        static void _dispatchLine(const char* line);
        static void _parseAndExpandEnvVars(const char* input, char* output, size_t maxLen);
        static char _lineBuf[SHELL_INPUT_BUFFER_SIZE];
        static size_t _lineIndex;
        static int _commandCount;
        static EnvVar _envVars[MAX_ENV_VARS];
        static int _logCallbackCount;
    };
}