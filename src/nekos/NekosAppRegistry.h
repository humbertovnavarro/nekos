#pragma once
#include "NekosArgParse.h"
#include <functional>
#ifdef ESP32
#include "esp_heap_caps.h" // ps_malloc
#endif
#include "NekosMemConfig.h"
#include "NekosApp.h"
namespace nekos {
    constexpr size_t MAX_APPS = MAX_NUM_APPS;
    class AppRegistry {
    public:
        static App* registerApp(const char* name, std::function<void(App*)> cb);
        static bool executeApp(const char* name, const char* args);
        static App* apps[MAX_NUM_APPS];
        static size_t appCount;
    };

}
