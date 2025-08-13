#include "NekosArgParse.h"

void NekosArgParse::addArgument(const char* name, bool required,
                                const char* defaultValue,
                                const char* help,
                                bool isFlag) {
    ArgSpec spec = { name, required, defaultValue, help, isFlag };
    argsOrder.push_back(spec);
}

bool NekosArgParse::parse(const char* input) {
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

const char* NekosArgParse::get(const char* name) {
    auto it = values.find(name);
    if (it != values.end()) return it->second.c_str();
    return "";
}

bool NekosArgParse::getFlag(const char* name) {
    auto it = values.find(name);
    return (it != values.end() && it->second == "true");
}

String NekosArgParse::usage(const char* progName) {
    String out = "Usage: ";
    out += progName;
    out += " ";
    for (auto& spec : argsOrder) {
        if (spec.isFlag) {
            out += "[--" + spec.name + "] ";
        } else if (spec.required) {
            out += "<" + spec.name + "> ";
        } else {
            out += "[" + spec.name + "] ";
        }
    }
    out += "\n\nArguments:\n";
    for (auto& spec : argsOrder) {
        out += "  ";
        if (spec.isFlag) out += "--";
        out += spec.name + "\t" + spec.help;
        if (spec.defaultValue.length()) {
            out += " (default: " + spec.defaultValue + ")";
        }
        out += "\n";
    }
    return out;
}

std::vector<String> NekosArgParse::tokenize(const char* str) {
    std::vector<String> result;
    String token;
    bool inQuotes = false;
    char quoteChar = '\0';
    bool escape = false;

    for (const char* p = str; *p; ++p) {
        char c = *p;

        if (escape) {
            token += c;       // Add char literally
            escape = false;
            continue;
        }

        if (c == '\\') {      // Start escape sequence
            escape = true;
            continue;
        }

        if (inQuotes) {
            if (c == quoteChar) {
                inQuotes = false; // End of quoted token
            } else {
                token += c;
            }
            continue;
        }

        if (c == '"' || c == '\'') {
            inQuotes = true;
            quoteChar = c;
            continue;
        }

        if (c == ' ' || c == '\t') {
            if (token.length()) {
                result.push_back(token);
                token = "";
            }
            continue;
        }

        token += c;
    }

    if (token.length()) {
        result.push_back(token);
    }

    return result;
}

NekosArgParse::ArgSpec* NekosArgParse::findSpec(const String& name) {
    for (auto& spec : argsOrder) {
        if (spec.name == name) return &spec;
    }
    return nullptr;
}