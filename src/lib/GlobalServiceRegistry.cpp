#include "GlobalServiceRegistry.h"

std::vector<TaskService*>& GlobalServiceRegistry::getServices() {
    static std::vector<TaskService*> services;
    return services;
}

void GlobalServiceRegistry::addService(TaskService* svc) {
    auto& services = getServices();
    for (auto s : services) {
        if (s == svc) return;
    }
    services.push_back(svc);
}

const std::vector<TaskService*>& GlobalServiceRegistry::getAllServices() {
    return getServices();
}
