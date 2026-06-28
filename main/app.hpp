// app.hpp
#pragma once
#include "core/lv_obj.h"
#include "freertos/idf_additions.h"
#include "misc/lv_types.h"
#include "portmacro.h"
#include <cstddef>
#include <cstdint>
#include <functional>

namespace nekos {

class IAppState {
    public:
    bool allocated = false;
    lv_obj_t* root;
    SemaphoreHandle_t semphr;

    inline void lock_from_task() {
        xSemaphoreTake(semphr, portMAX_DELAY);
    }

    inline void unlock_from_task() {
        xSemaphoreGive(semphr);
    }

    inline void unlock_from_isr() {
        xSemaphoreTakeFromISR(semphr, 0);
    }

    inline void lock_from_isr() {
        xSemaphoreGiveFromISR(semphr, 0);
    }

    inline void lock(bool is_isr = false) {
        is_isr ? lock_from_isr() : lock_from_task();
    }

    inline void unlock(bool is_isr = false) {
        is_isr ? unlock_from_isr() : unlock_from_task();
    }
};

template<typename T>
class AppState : public IAppState, public T {};

struct IApp {
    const char* name = "";
    const char* icon = "";
    virtual bool launch(bool is_isr = false)            = 0;
    virtual bool stop(bool is_isr = false)              = 0;
    virtual bool running()     const = 0;
    virtual bool backgrounded() const = 0;
    virtual bool background()        = 0;
    virtual bool foreground()        = 0;
    virtual ~IApp() = default;
};

template<typename T>
class App : public IApp {
public:
    using TaskFn = void(*)(App*);

    struct CreateOptions {
        AppState<T>               state        = {};
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
    AppState<T>               state;
    TaskFn                    fn            = nullptr;
    bool allocated = false;
    std::function<void(App*)> allocater;
    std::function<void(App*)> deleter;
    std::function<void(App*)> on_background;
    std::function<void(App*)> on_foreground;

    App() = default;
    App(const App&)            = delete;
    App& operator=(const App&) = delete;

    App(App&& o) noexcept
        : stack_depth(o.stack_depth), priority(o.priority),
          state(std::move(o.state)),
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
            state    = std::move(o.state);
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

    bool malloc(bool is_isr = false) {
        if(!state.allocated) {
            state.semphr = xSemaphoreCreateMutex();
            state.lock(is_isr);
            allocater(this);
            state.allocated = true;
            state.unlock(is_isr);
        }
        return true;
    }

    bool launch(bool is_isr = false) override {
        if (running()) return false;

        malloc();

        BaseType_t result = xTaskCreate(
            &trampoline,
            name,
            stack_depth,
            this,
            priority,
            &handle_
        );

        if (result != pdPASS) {
            state.lock(is_isr);
            deleter(this);
            state.allocated = false;
            state.unlock(is_isr);
            return false;
        }

        return true;
    }

    bool stop(bool is_isr = false) override {
        if (!running()) return false;
        state.lock(is_isr);
        deleter(this);
        state.unlock(is_isr);
        if(handle_ != nullptr) {
            vTaskDelete(handle_);
        }
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

} // namespace nekos