#pragma once

#include "core/logging/log_message.hpp"

#include <condition_variable>
#include <cstddef>
#include <deque>
#include <mutex>
#include <optional>
#include <utility>

namespace spy::internal {

class LogQueue {
public:
    explicit LogQueue(std::size_t capacity)
        : m_capacity(capacity == 0 ? 1 : capacity) {
    }

    LogQueue(const LogQueue&) = delete;
    LogQueue& operator=(const LogQueue&) = delete;

    bool push(LogMessage message) {
        {
            std::lock_guard lock(m_mutex);
            if (m_stopped) {
                return false;
            }
            if (m_items.size() >= m_capacity) {
                ++m_dropped;
                return false;
            }
            m_items.push_back(std::move(message));
        }
        m_ready.notify_one();
        return true;
    }

    std::optional<LogMessage> pop() {
        std::unique_lock lock(m_mutex);
        m_ready.wait(lock, [this] { return m_stopped || !m_items.empty(); });
        if (m_items.empty()) {
            return std::nullopt;
        }
        LogMessage message = std::move(m_items.front());
        m_items.pop_front();
        return message;
    }

    std::optional<LogMessage> tryPop() {
        std::lock_guard lock(m_mutex);
        if (m_items.empty()) {
            return std::nullopt;
        }
        LogMessage message = std::move(m_items.front());
        m_items.pop_front();
        return message;
    }

    void stop() {
        {
            std::lock_guard lock(m_mutex);
            m_stopped = true;
        }
        m_ready.notify_all();
    }

    void clear() {
        std::lock_guard lock(m_mutex);
        m_items.clear();
    }

    [[nodiscard]] std::size_t size() const {
        std::lock_guard lock(m_mutex);
        return m_items.size();
    }

    [[nodiscard]] std::size_t dropped() const {
        std::lock_guard lock(m_mutex);
        return m_dropped;
    }

    [[nodiscard]] bool stopped() const {
        std::lock_guard lock(m_mutex);
        return m_stopped;
    }

private:
    mutable std::mutex m_mutex;
    std::condition_variable m_ready;
    std::deque<LogMessage> m_items;
    std::size_t m_capacity;
    std::size_t m_dropped = 0;
    bool m_stopped = false;
};

}
