// app.hpp
#pragma once
#include "freertos/idf_additions.h"
#include <functional>

namespace nekos {

struct AppBase {
    const char* name = "";
    const char* icon = "";
    virtual bool launch()            = 0;
    virtual bool stop()              = 0;
    virtual bool running()     const = 0;
    virtual bool backgrounded() const = 0;
    virtual bool background()        = 0;
    virtual bool foreground()        = 0;
    virtual ~AppBase() = default;
};

template<typename T>
class App : public AppBase {
public:
    using TaskFn = void(*)(App*);

    struct CreateOptions {
        T                         references   = {};
        const char*               name         = "";
        const char*               icon         = "";
        TaskFn                    fn           = nullptr;
        std::function<void(App*)> allocater    = nullptr;
        std::function<void(App*)> deleter      = nullptr;
        std::function<void(App*)> on_background = nullptr;
        std::function<void(App*)> on_foreground = nullptr;
        size_t                    stack_depth  = 4096;
        uint8_t                   priority     = 3;
    };

    size_t                    stack_depth   = 4096;
    uint8_t                   priority      = 3;
    T                         references;
    TaskFn                    fn            = nullptr;
    std::function<void(App*)> allocater;
    std::function<void(App*)> deleter;
    std::function<void(App*)> on_background;
    std::function<void(App*)> on_foreground;

    App() = default;
    App(const App&)            = delete;
    App& operator=(const App&) = delete;

    App(App&& o) noexcept
        : stack_depth(o.stack_depth), priority(o.priority),
          references(std::move(o.references)),
          fn(o.fn),
          allocater(std::move(o.allocater)),
          deleter(std::move(o.deleter)),
          on_background(std::move(o.on_background)),
          on_foreground(std::move(o.on_foreground)),
          handle_(o.handle_),
          backgrounded_(o.backgrounded_)
    {
        this->name  = o.name;
        this->icon  = o.icon;
        o.handle_   = nullptr;
        o.backgrounded_ = false;
    }

    App& operator=(App&& o) noexcept {
        if (this != &o) {
            if (running()) stop();
            this->name    = o.name;
            this->icon    = o.icon;
            stack_depth   = o.stack_depth;
            priority      = o.priority;
            references    = std::move(o.references);
            fn            = o.fn;
            allocater     = std::move(o.allocater);
            deleter       = std::move(o.deleter);
            on_background = std::move(o.on_background);
            on_foreground = std::move(o.on_foreground);
            handle_       = o.handle_;
            backgrounded_ = o.backgrounded_;
            o.handle_     = nullptr;
            o.backgrounded_ = false;
        }
        return *this;
    }

    ~App() override {
        if (running()) stop();
    }

    static App create(CreateOptions opts) {
        App a;
        a.name         = opts.name;
        a.icon         = opts.icon;
        a.fn           = opts.fn;
        a.allocater    = std::move(opts.allocater);
        a.deleter      = std::move(opts.deleter);
        a.on_background = std::move(opts.on_background);
        a.on_foreground = std::move(opts.on_foreground);
        a.stack_depth  = opts.stack_depth;
        a.priority     = opts.priority;
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
        handle_       = nullptr;
        backgrounded_ = false;
        return true;
    }

    bool background() override {
        if (!running() || backgrounded_) return false;
        backgrounded_ = true;
        if (on_background) on_background(this);
        return true;
    }

    bool foreground() override {
        if (!running() || !backgrounded_) return false;
        backgrounded_ = false;
        if (on_foreground) on_foreground(this);
        return true;
    }

    bool running()      const override { return handle_ != nullptr; }
    bool backgrounded() const override { return backgrounded_; }

    TaskHandle_t handle() const { return handle_; }

private:
    TaskHandle_t handle_      = nullptr;
    bool         backgrounded_ = false;

    static void trampoline(void* param) {
        App* self     = static_cast<App*>(param);
        self->fn(self);
        self->handle_ = nullptr;
        vTaskDelete(nullptr);
    }
};

#define NEKOS_DISPLAY_BACKGROUND \
    [](auto self) {                            \
        bsp_display_lock(portMAX_DELAY);       \
        lv_obj_del(self->references.root);     \
        bsp_display_unlock();                  \
    }

#define NEKOS_DISPLAY_FOREGROUND(draw_fn)  \
    [](auto self) {                          \
        bsp_display_lock(portMAX_DELAY);     \
        draw_fn(self);                          \
        lv_screen_load(self->references.root); \
        bsp_display_unlock();                \
    }
} // namespace nekos