#include "NekosApp.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <cstring>
#include "NekosMemConfig.h"
#include "NekosAppRegistry.h"

namespace nekos {
App* AppRegistry::apps[MAX_NUM_APPS];
size_t AppRegistry::appCount = 0;

App* AppRegistry::registerApp(const char* name, std::function<void(App*)> cb) {
    if (appCount >= MAX_APPS) {
        return nullptr;
    }
    // Allocate app in PSRAM if available
    App* app = (App*) PSMALLOC_OR_MALLOC(sizeof(App));
    app->appSignalQueue = xQueueCreate(STDIO_BUFFER_COUNT, sizeof(AppSignal));
    app->inQueue    = xQueueCreate(STDIO_BUFFER_COUNT, sizeof(char*));
    app->outQueue   = xQueueCreate(STDIO_BUFFER_COUNT, sizeof(char*));
    apps[appCount++] = app;
    return app;
}

bool AppRegistry::executeApp(const char* name, const char* args) {
    for (size_t i = 0; i < appCount; i++) {
        if (strcmp(apps[i]->name.c_str(), name) == 0) {
            App* app = apps[i];
            if (!app->cb) {
                return false;
            }
            if (args && !app->args.parse(args)) {
                return false;
            }
            app->cb(app);
            return true;
        }
    }
    return false;
}

}
