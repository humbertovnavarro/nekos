#pragma once
#include <Arduino.h>

#ifndef NEKOS_ARGPARSE_MAX_ARGS
#define NEKOS_ARGPARSE_MAX_ARGS 16
#endif

namespace nekos {

class ArgParse {
public:
    struct ArgSpec {
        const char* name;
        bool required;
        const char* defaultValue;
        const char* help;
        bool isFlag;
    };

    ArgParse();

    ArgParse* addArgument(const char* name, bool required,
                          const char* defaultValue,
                          const char* help,
                          bool isFlag);

    bool parse(const char* input);
    const char* get(const char* name);
    bool getFlag(const char* name);
    String usage(const char* progName);

private:
    ArgSpec argsOrder[NEKOS_ARGPARSE_MAX_ARGS];
    size_t argCount;

    struct ArgValue {
        const char* name;
        String value;
    };
    ArgValue values[NEKOS_ARGPARSE_MAX_ARGS];
    size_t valueCount;

    void clearValues();
    bool hasValue(const char* name);
    void setValue(const char* name, const String& val);
    const String* findValue(const char* name) const;

    ArgSpec* findSpec(const String& name);
    size_t tokenize(const char* str, String tokens[], size_t maxTokens);
};

} // namespace nekos
