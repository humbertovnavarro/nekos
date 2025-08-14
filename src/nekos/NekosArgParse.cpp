#include "NekosArgParse.h"
namespace nekos {
ArgParse* ArgParse::addArgument(const char* name, bool required,
                                const char* defaultValue,
                                const char* help,
                                bool isFlag) {
    ArgSpec spec = { name, required, defaultValue, help, isFlag };
    argsOrder.push_back(spec);
    return this;
}

bool ArgParse::parse(const char* input) {
    values.clear();
    std::vector<String> tokens = tokenize(input);
    size_t i = 0;

    while (i < tokens.size()) {
        String token = tokens[i];

        // Handle --key or --flag style
        if (token.startsWith("--")) {
            token.remove(0, 2); // strip "--"
            ArgSpec* spec = findSpec(token);
            if (!spec) return false;

            if (spec->isFlag) {
                values[spec->name] = "true";
            } else {
                if (i + 1 >= tokens.size()) return false; // Missing value
                values[spec->name] = tokens[++i];
            }
            i++;
            continue;
        }

        // Handle positional args
        for (auto& spec : argsOrder) {
            if (!values.count(spec.name) && !spec.isFlag) {
                values[spec.name] = token;
                break;
            }
        }
        i++;
    }

    // Apply defaults and check required
    for (auto& spec : argsOrder) {
        if (!values.count(spec.name)) {
            if (spec.required) return false;
            values[spec.name] = spec.defaultValue;
        }
    }

    return true;
}

const char* ArgParse::get(const char* name) {
    auto it = values.find(name);
    if (it != values.end()) return it->second.c_str();
    return "";
}

bool ArgParse::getFlag(const char* name) {
    auto it = values.find(name);
    return (it != values.end() && it->second == "true");
}

String ArgParse::usage(const char* progName) {
    String out = "";
    out += progName;
    // List arguments in usage line
    for (auto& spec : argsOrder) {
        if (spec.isFlag) {
            out += " [--" + spec.name + "]";
        } else if (spec.required) {
            out += " <" + spec.name + ">";
        } else {
            out += " [" + spec.name + "]";
        }
    }
    return out;
}


std::vector<String> ArgParse::tokenize(const char* str) {
    std::vector<String> result;
    String s(str);
    int start = 0;
    while (true) {
        int idx = s.indexOf(' ', start);
        if (idx == -1) {
            if (start < s.length()) result.push_back(s.substring(start));
            break;
        }
        if (idx > start) result.push_back(s.substring(start, idx));
        start = idx + 1;
    }
    return result;
}

ArgParse::ArgSpec* ArgParse::findSpec(const String& name) {
    for (auto& spec : argsOrder) {
        if (spec.name == name) return &spec;
    }
    return nullptr;
}

}
