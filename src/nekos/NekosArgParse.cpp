#include "NekosArgParse.h"
namespace nekos {

ArgParse* ArgParse::addArgument(const char* name, bool required,
                                const char* defaultValue,
                                const char* help,
                                bool isFlag) {
    ArgSpec spec = { name, required, defaultValue ? defaultValue : "", help, isFlag };
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
            if (spec.isFlag) {
                values[spec.name] = "false"; // default for flags
            } else if (spec.required) {
                return false; // missing required arg
            } else {
                values[spec.name] = spec.defaultValue ? spec.defaultValue : "";
            }
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
    String out = progName;
    // List arguments in usage line
    for (auto& spec : argsOrder) {
        if (spec.isFlag) {
            out += " [--" + String(spec.name) + "]";
        } else if (spec.required) {
            out += " <" + String(spec.name) + ">";
        } else {
            out += " [" + String(spec.name) + "]";
        }
    }
    return out;
}

std::vector<String> ArgParse::tokenize(const char* str) {
    std::vector<String> result;
    String s(str);
    int i = 0;

    while (i < s.length()) {
        // Skip whitespace
        while (i < s.length() && isspace((unsigned char)s[i])) i++;
        if (i >= s.length()) break;

        char quote = 0;
        if (s[i] == '"' || s[i] == '\'') {
            quote = s[i++];
        }

        String token = "";
        while (i < s.length()) {
            char c = s[i];

            if (quote) {
                // Quoted string mode
                if (c == quote) {
                    i++; // consume closing quote
                    break;
                }
                if (c == '\\' && i + 1 < s.length()) {
                    // Escaped character
                    token += s[i + 1];
                    i += 2;
                } else {
                    token += c;
                    i++;
                }
            } else {
                // Unquoted
                if (isspace((unsigned char)c)) {
                    break;
                }
                if (c == '\\' && i + 1 < s.length()) {
                    token += s[i + 1];
                    i += 2;
                } else {
                    token += c;
                    i++;
                }
            }
        }

        if (token.length() > 0) {
            result.push_back(token);
        }

        // Skip trailing whitespace before next token
        while (i < s.length() && isspace((unsigned char)s[i])) i++;
    }

    return result;
}

ArgParse::ArgSpec* ArgParse::findSpec(const String& name) {
    for (auto& spec : argsOrder) {
        if (String(spec.name) == name) return &spec;
    }
    return nullptr;
}

} // namespace nekos
