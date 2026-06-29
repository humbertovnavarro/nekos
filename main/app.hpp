// app.hpp
#pragma once
#include "core/lv_obj.h"
#include "freertos/idf_additions.h"
#include "freertos/task.h"
#include "misc/lv_types.h"
#include "portmacro.h"
#include <algorithm>
#include <cstddef>
#include <cstdint>

namespace nekos {

// ─── Signals ─────────────────────────────────────────────────────────────────
enum class AppSignal : uint32_t {
    Stop       = (1 << 0),
    Background = (1 << 1),
    Foreground = (1 << 2),
    StateDirty = (1 << 3),
};

// ─── IAppState ───────────────────────────────────────────────────────────────
class IAppState {
public:
    lv_obj_t*         root   = nullptr;
    SemaphoreHandle_t semphr = nullptr;

    inline void lock_from_task()  { xSemaphoreTake(semphr, portMAX_DELAY); }
    inline void unlock_from_task(){ xSemaphoreGive(semphr); }
    inline void lock_from_isr()   { xSemaphoreGiveFromISR(semphr, nullptr); }
    inline void unlock_from_isr() { xSemaphoreTakeFromISR(semphr, nullptr); }

    inline void lock  (bool is_isr = false){ is_isr ? lock_from_isr()   : lock_from_task();   }
    inline void unlock(bool is_isr = false){ is_isr ? unlock_from_isr() : unlock_from_task(); }
};

template<typename T>
class AppState : public IAppState, public T {};

// ─── IApp ────────────────────────────────────────────────────────────────────
struct IApp {
    const char* name = "";
    const char* icon = "";
    virtual bool launch(bool is_isr = false) = 0;
    virtual bool stop(bool is_isr = false)   = 0;
    virtual bool running()      const        = 0;
    virtual bool backgrounded() const        = 0;
    virtual bool background()                = 0;
    virtual bool foreground()                = 0;
    virtual bool dirty(bool is_isr = false)  = 0;
    virtual ~IApp() = default;
};

// ─── App ─────────────────────────────────────────────────────────────────────
template<typename T>
class App : public IApp {
public:
    // Canonical task fn:
    //
    //   [](App* self) {
    //       // setup
    //       while (self->poll()) {
    //           if (self->backgrounded()) continue;
    //           if (self->signalled(AppSignal::StateDirty)) {
    //               // re-render or act on changed state
    //           }
    //       }
    //       // teardown
    //   }
    using TaskFn = void(*)(App*);

    struct CreateOptions {
        const char* name        = "";
        const char* icon        = "";
        TaskFn      fn          = nullptr;
        size_t      stack_depth = 4096;
        uint8_t     priority    = 3;
        TickType_t  poll_ticks  = pdMS_TO_TICKS(100);
    };

    AppState<T> state;
    TaskFn      fn          = nullptr;
    size_t      stack_depth = 4096;
    uint8_t     priority    = 3;
    TickType_t  poll_ticks  = pdMS_TO_TICKS(100);

    // ── Construction ──────────────────────────────────────────────────────────
    App() = default;
    App(const App&)            = delete;
    App& operator=(const App&) = delete;

    App(App&& o) noexcept
        : state(std::move(o.state)),
          fn(o.fn),
          stack_depth(o.stack_depth),
          priority(o.priority),
          poll_ticks(o.poll_ticks),
          handle_(o.handle_),
          last_bits_(o.last_bits_),
          backgrounded_(o.backgrounded_)
    {
        this->name      = o.name;
        this->icon      = o.icon;
        o.handle_       = nullptr;
        o.last_bits_    = 0;
        o.backgrounded_ = false;
    }

    App& operator=(App&& o) noexcept {
        if (this != &o) {
            if (running()) stop();
            this->name    = o.name;
            this->icon    = o.icon;
            stack_depth   = o.stack_depth;
            priority      = o.priority;
            poll_ticks    = o.poll_ticks;
            state         = std::move(o.state);
            fn            = o.fn;
            handle_       = o.handle_;
            last_bits_    = o.last_bits_;
            backgrounded_ = o.backgrounded_;
            o.handle_     = nullptr;
            o.last_bits_  = 0;
            o.backgrounded_ = false;
        }
        return *this;
    }

    ~App() override { if (running()) stop(); }

    static App create(CreateOptions opts) {
        App a;
        a.name        = opts.name;
        a.icon        = opts.icon;
        a.fn          = opts.fn;
        a.stack_depth = opts.stack_depth;
        a.priority    = opts.priority;
        a.poll_ticks  = opts.poll_ticks;
        return a;
    }

    bool launch(bool /*is_isr*/ = false) override {
        if (running()) return false;
        return xTaskCreate(&trampoline, name, stack_depth, this, priority, &handle_) == pdPASS;
    }

    bool stop(bool is_isr = false) override {
        return signal(AppSignal::Stop, is_isr);
    }

    bool background() override { return signal(AppSignal::Background, false); }
    bool foreground() override { return signal(AppSignal::Foreground, false); }

    // Notify the task that state has changed and it should re-evaluate.
    bool dirty(bool is_isr = false) override {
        return signal(AppSignal::StateDirty, is_isr);
    }

    bool running()      const override { return handle_ != nullptr; }
    bool backgrounded() const override { return backgrounded_; }

    bool signalled(AppSignal sig) {
        uint32_t bit = static_cast<uint32_t>(sig);
        bool set = last_bits_ & bit;
        last_bits_ &= ~bit;
        return set;
    }

    TaskHandle_t handle() const { return handle_; }

    bool poll() {
        last_bits_ = 0;
        xTaskNotifyWait(0, UINT32_MAX, &last_bits_, poll_ticks);

        if (last_bits_ & static_cast<uint32_t>(AppSignal::Stop)) {
            last_bits_ = 0;
            return false;
        }

        if (last_bits_ & static_cast<uint32_t>(AppSignal::Background)) {
            backgrounded_ = true;
        }

        if (last_bits_ & static_cast<uint32_t>(AppSignal::Foreground)) {
            backgrounded_ = false;
        }

        return true;
    }

private:
    TaskHandle_t handle_       = nullptr;
    uint32_t     last_bits_    = 0;
    bool         backgrounded_ = false;

    bool signal(AppSignal sig, bool is_isr) {
        if (!running()) return false;
        uint32_t bits = static_cast<uint32_t>(sig);
        if (is_isr) {
            BaseType_t woken = pdFALSE;
            xTaskNotifyFromISR(handle_, bits, eSetBits, &woken);
            portYIELD_FROM_ISR(woken);
        } else {
            xTaskNotify(handle_, bits, eSetBits);
        }
        return true;
    }

    static void trampoline(void* param) {
        App* self = static_cast<App*>(param);
        self->state.semphr = xSemaphoreCreateMutex();
        self->fn(self);
        vSemaphoreDelete(self->state.semphr);
        self->state.semphr  = nullptr;
        self->handle_       = nullptr;
        self->last_bits_    = 0;
        self->backgrounded_ = false;
        vTaskDelete(nullptr);
    }
};

} // namespace nekos