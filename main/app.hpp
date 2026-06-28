// app.hpp — add a base class
#pragma once
#include "freertos/idf_additions.h"
#include <functional>

namespace nekos {

struct AppBase {
    const char* name = "";
    const char* icon = "";
    virtual bool launch() = 0;
    virtual bool stop()   = 0;
    virtual bool running() const = 0;
    virtual ~AppBase() = default;
};

template<typename T>
class App : public AppBase {
public:
    using TaskFn = void(*)(App*);

    struct CreateOptions {
        T                         references  = {};
        const char*               name        = "";
        const char*               icon        = "";
        TaskFn                    fn          = nullptr;
        std::function<void(App*)> allocater   = nullptr;
        std::function<void(App*)> deleter     = nullptr;
        size_t                    stack_depth = 4096;
        uint8_t                   priority    = 3;
    };

    size_t                    stack_depth = 4096;
    uint8_t                   priority    = 3;
    T                         references;
    TaskFn                    fn          = nullptr;
    std::function<void(App*)> allocater;
    std::function<void(App*)> deleter;

    App() = default;

    App(const App&)            = delete;
    App& operator=(const App&) = delete;

    App(App&& o) noexcept
        : stack_depth(o.stack_depth), priority(o.priority),
          references(std::move(o.references)),
          fn(o.fn),
          allocater(std::move(o.allocater)),
          deleter(std::move(o.deleter)),
          handle_(o.handle_)
    {
        this->name = o.name;
        this->icon = o.icon;
        o.handle_  = nullptr;
    }

    App& operator=(App&& o) noexcept {
        if (this != &o) {
            if (running()) stop();
            this->name  = o.name;
            this->icon  = o.icon;
            stack_depth = o.stack_depth;
            priority    = o.priority;
            references  = std::move(o.references);
            fn          = o.fn;
            allocater   = std::move(o.allocater);
            deleter     = std::move(o.deleter);
            handle_     = o.handle_;
            o.handle_   = nullptr;
        }
        return *this;
    }

    ~App() override {
        if (running()) stop();
    }

    static App create(CreateOptions opts) {
        App a;
        a.name        = opts.name;
        a.icon        = opts.icon;
        a.fn          = opts.fn;
        a.allocater   = std::move(opts.allocater);
        a.deleter     = std::move(opts.deleter);
        a.stack_depth = opts.stack_depth;
        a.priority    = opts.priority;
        return a;
    }

    bool launch() override {
        if (running()) return false;
        allocater(this);
        BaseType_t result = xTaskCreate(
            &trampoline,
            name,
            stack_depth,
            this,
            priority,
            &handle_
        );
        if (result != pdPASS) {
            deleter(this);
            return false;
        }
        return true;
    }

    bool stop() override {
        if (!running()) return false;
        vTaskSuspend(handle_);
        deleter(this);
        vTaskDelete(handle_);
        handle_ = nullptr;
        return true;
    }

    bool running() const override {
        return handle_ != nullptr;
    }

    TaskHandle_t handle() const {
        return handle_;
    }

private:
    TaskHandle_t handle_ = nullptr;

    static void trampoline(void* param) {
        App* self = static_cast<App*>(param);
        self->fn(self);
        self->handle_ = nullptr;
        vTaskDelete(nullptr);
    }
};

} // namespace nekos