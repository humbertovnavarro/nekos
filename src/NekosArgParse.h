#pragma once
#include <Arduino.h>
#include <map>
#include <vector>
class NekosArgParse {
public:
    struct ArgSpec {
        String name;
        bool required;
        String defaultValue;
        String help;
        bool isFlag;
    };
    void addArgument(const char* name, bool required = false,
                     const char* defaultValue = "",
                     const char* help = "",
                     bool isFlag = false);
    bool parse(const char* input);
    const char* get(const char* name);
    bool getFlag(const char* name);
    String usage(const char* progName);

private:
    std::vector<ArgSpec> argsOrder;
    std::map<String, String> values;
    std::vector<String> tokenize(const char* str);
    ArgSpec* findSpec(const String& name);
};
