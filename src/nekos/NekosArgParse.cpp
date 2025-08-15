#include "NekosArgParse.h"
#include <string.h>

namespace nekos {

ArgParse::ArgParse() : argCount(0), valueCount(0) {}

ArgParse* ArgParse::addArgument(const char* name, bool required,
                                const char* defaultValue,
                                const char* help,
                                bool isFlag) {
    if (argCount >= NEKOS_ARGPARSE_MAX_ARGS) return this;
    argsOrder[argCount++] = { name, required, defaultValue, help, isFlag };
    return this;
}

bool ArgParse::parse(const char* input) {
    clearValues();

    String tokens[NEKOS_ARGPARSE_MAX_ARGS * 2];
    size_t tokenCount = tokenize(input, tokens, NEKOS_ARGPARSE_MAX_ARGS * 2);
    size_t i = 0;

    while (i < tokenCount) {
        String token = tokens[i];

        // Handle --key or --flag style
        if (token.startsWith("--")) {
            token.remove(0, 2);
            ArgSpec* spec = findSpec(token);
            if (!spec) return false;

            if (spec->isFlag) {
                setValue(spec->name, "true");
            } else {
                if (i + 1 >= tokenCount) return false; // Missing value
                setValue(spec->name, tokens[++i]);
            }
            i++;
            continue;
        }

        // Handle positional args
        for (size_t j = 0; j < argCount; j++) {
            ArgSpec& spec = argsOrder[j];
            if (!hasValue(spec.name) && !spec.isFlag) {
                setValue(spec.name, token);
                break;
            }
        }
        i++;
    }

    // Apply defaults and check required
    for (size_t j = 0; j < argCount; j++) {
        ArgSpec& spec = argsOrder[j];
        if (!hasValue(spec.name)) {
            if (spec.required) return false;
            setValue(spec.name, spec.defaultValue);
        }
    }

    return true;
}

const char* ArgParse::get(const char* name) {
    const String* val = findValue(name);
    return val ? val->c_str() : "";
}

bool ArgParse::getFlag(const char* name) {
    const String* val = findValue(name);
    return (val && *val == "true");
}

String ArgParse::usage(const char* progName) {
    String out;
    out += progName;
    for (size_t j = 0; j < argCount; j++) {
        ArgSpec& spec = argsOrder[j];
        if (spec.isFlag) {
            out += " [--";
            out += spec.name;
            out += "]";
        } else if (spec.required) {
            out += " <";
            out += spec.name;
            out += ">";
        } else {
            out += " [";
            out += spec.name;
            out += "]";
        }
    }
    return out;
}

size_t ArgParse::tokenize(const char* str, String tokens[], size_t maxTokens) {
    String s(str);
    size_t count = 0;
    int start = 0;

    while (true) {
        int idx = s.indexOf(' ', start);
        if (idx == -1) {
            if (start < s.length() && count < maxTokens) {
                tokens[count++] = s.substring(start);
            }
            break;
        }
        if (idx > start && count < maxTokens) {
            tokens[count++] = s.substring(start, idx);
        }
        start = idx + 1;
    }

    return count;
}

ArgParse::ArgSpec* ArgParse::findSpec(const String& name) {
    for (size_t j = 0; j < argCount; j++) {
        if (name == argsOrder[j].name) {
            return &argsOrder[j];
        }
    }
    return nullptr;
}

void ArgParse::clearValues() {
    valueCount = 0;
}

bool ArgParse::hasValue(const char* name) {
    for (size_t j = 0; j < valueCount; j++) {
        if (strcmp(values[j].name, name) == 0) return true;
    }
    return false;
}

void ArgParse::setValue(const char* name, const String& val) {
    for (size_t j = 0; j < valueCount; j++) {
        if (strcmp(values[j].name, name) == 0) {
            values[j].value = val;
            return;
        }
    }
    if (valueCount < NEKOS_ARGPARSE_MAX_ARGS) {
        values[valueCount].name = name;
        values[valueCount].value = val;
        valueCount++;
    }
}

const String* ArgParse::findValue(const char* name) const {
    for (size_t j = 0; j < valueCount; j++) {
        if (strcmp(values[j].name, name) == 0) {
            return &values[j].value;
        }
    }
    return nullptr;
}

}
