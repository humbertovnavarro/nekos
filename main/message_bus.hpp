#pragma once

#include "esp_err.h"
#include "etl/vector.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/idf_additions.h"
#include <cassert>
#include <cstdint>
#include <optional>
#include <type_traits>

template <typename T>
class TypedQueue {
    static_assert(std::is_trivially_copyable<T>::value,
                  "TypedQueue<T>: T must be trivially copyable");

    QueueHandle_t handle = nullptr;

public:
    TypedQueue() = default;

    TypedQueue(const TypedQueue&)            = delete;
    TypedQueue& operator=(const TypedQueue&) = delete;

    TypedQueue(TypedQueue&& other) noexcept : handle(other.handle) {
        other.handle = nullptr;
    }
    TypedQueue& operator=(TypedQueue&& other) noexcept {
        handle       = other.handle;
        other.handle = nullptr;
        return *this;
    }

    QueueHandle_t& _raw_ref() { return handle; }

    bool    valid()              const { return handle != nullptr; }
    explicit operator bool()     const { return valid(); }

    std::optional<T> receive(TickType_t ticks = portMAX_DELAY) {
        T item;
        if (xQueueReceive(handle, &item, ticks) == pdPASS)
            return item;
        return std::nullopt;
    }

    std::optional<T> peek(TickType_t ticks = 0) const {
        T item;
        if (xQueuePeek(handle, &item, ticks) == pdPASS)
            return item;
        return std::nullopt;
    }

    std::optional<T> receive_from_isr(BaseType_t* higher_priority_task_woken = nullptr) {
        T item;
        BaseType_t woken = pdFALSE;
        if (xQueueReceiveFromISR(handle, &item, &woken) == pdPASS) {
            if (higher_priority_task_woken) *higher_priority_task_woken |= woken;
            return item;
        }
        return std::nullopt;
    }

    UBaseType_t waiting() const { return uxQueueMessagesWaiting(handle); }
    UBaseType_t spaces()  const { return uxQueueSpacesAvailable(handle); }
    bool        empty()   const { return waiting() == 0; }
};

template <typename E, uint8_t max_subscribers, uint8_t queue_depth>
class MessageBus {
    static_assert(std::is_trivially_copyable<E>::value,
                  "MessageBus: event type must be trivially copyable");

    QueueHandle_t                               pool[max_subscribers];
    etl::vector<QueueHandle_t, max_subscribers> active;
    etl::vector<QueueHandle_t, max_subscribers> inactive;
    portMUX_TYPE                                lock;

public:
    MessageBus() : lock(portMUX_INITIALIZER_UNLOCKED) {
        for (uint8_t i = 0; i < max_subscribers; i++) {
            pool[i] = xQueueCreate(queue_depth, sizeof(E));
            assert(pool[i] != nullptr && "MessageBus: xQueueCreate failed (out of memory)");
            inactive.push_back(pool[i]);
        }
    }

    ~MessageBus() {
        for (uint8_t i = 0; i < max_subscribers; i++) {
            if (pool[i] != nullptr)
                vQueueDelete(pool[i]);
        }
    }

    MessageBus(const MessageBus&)            = delete;
    MessageBus& operator=(const MessageBus&) = delete;

    esp_err_t sub(TypedQueue<E>& tq) {
        portENTER_CRITICAL(&lock);
        if (inactive.empty()) {
            portEXIT_CRITICAL(&lock);
            return ESP_ERR_NO_MEM;
        }
        QueueHandle_t q = inactive.back();
        inactive.pop_back();
        active.push_back(q);
        portEXIT_CRITICAL(&lock);

        xQueueReset(q);
        tq._raw_ref() = q;
        return ESP_OK;
    }

    esp_err_t sub_from_isr(TypedQueue<E>& tq) {
        portENTER_CRITICAL_ISR(&lock);
        if (inactive.empty()) {
            portEXIT_CRITICAL_ISR(&lock);
            return ESP_ERR_NO_MEM;
        }
        QueueHandle_t q = inactive.back();
        inactive.pop_back();
        active.push_back(q);
        portEXIT_CRITICAL_ISR(&lock);

        tq._raw_ref() = q;
        return ESP_OK;
    }

    esp_err_t unsub(TypedQueue<E>& tq) {
        portENTER_CRITICAL(&lock);
        for (size_t i = 0; i < active.size(); i++) {
            if (active[i] == tq._raw_ref()) {
                active.erase(active.begin() + i);
                inactive.push_back(tq._raw_ref());
                portEXIT_CRITICAL(&lock);
                tq._raw_ref() = nullptr;
                return ESP_OK;
            }
        }
        portEXIT_CRITICAL(&lock);
        return ESP_ERR_NOT_FOUND;
    }

    esp_err_t unsub_from_isr(TypedQueue<E>& tq) {
        portENTER_CRITICAL_ISR(&lock);
        for (size_t i = 0; i < active.size(); i++) {
            if (active[i] == tq._raw_ref()) {
                active.erase(active.begin() + i);
                inactive.push_back(tq._raw_ref());
                portEXIT_CRITICAL_ISR(&lock);
                tq._raw_ref() = nullptr;
                return ESP_OK;
            }
        }
        portEXIT_CRITICAL_ISR(&lock);
        return ESP_ERR_NOT_FOUND;
    }

    esp_err_t post(E event) {
        esp_err_t result = ESP_OK;
        portENTER_CRITICAL(&lock);
        if (active.empty()) {
            portEXIT_CRITICAL(&lock);
            return ESP_OK;
        }
        for (auto q : active) {
            if (xQueueSend(q, &event, 0) != pdPASS)
                result = ESP_ERR_TIMEOUT;
        }
        portEXIT_CRITICAL(&lock);
        return result;
    }

    esp_err_t post_from_isr(E event) {
        BaseType_t woken  = pdFALSE;
        esp_err_t  result = ESP_OK;

        portENTER_CRITICAL_ISR(&lock);
        if (active.empty()) {
            portEXIT_CRITICAL_ISR(&lock);
            return ESP_OK;
        }
        for (auto q : active) {
            BaseType_t task_woken = pdFALSE;
            if (xQueueSendFromISR(q, &event, &task_woken) != pdPASS)
                result = ESP_ERR_TIMEOUT;
            woken |= task_woken;
        }
        portEXIT_CRITICAL_ISR(&lock);

        portYIELD_FROM_ISR(woken);
        return result;
    }
};