#pragma once

#include "core/logging/internal/log_queue.hpp"

#include <atomic>
#include <functional>
#include <mutex>
#include <optional>
#include <thread>
#include <utility>

namespace spy::internal {

class LogWorker {
public:
    using DispatchFn = std::function<void(const LogMessage&)>;

    LogWorker(LogQueue& queue, DispatchFn dispatch)
        : m_queue(queue), m_dispatch(std::move(dispatch)) {
    }

    ~LogWorker() {
        stop();
    }

    LogWorker(const LogWorker&) = delete;
    LogWorker& operator=(const LogWorker&) = delete;

    void start() {
        if (m_running.exchange(true)) {
            return;
        }
        m_thread = std::thread([this] { run(); });
    }

    void stop() {
        {
            std::lock_guard lock(m_mutex);
            if (!m_running.exchange(false)) {
                if (m_thread.joinable()) {
                    m_thread.join();
                }
                return;
            }
        }

        m_queue.stop();
        if (m_thread.joinable()) {
            m_thread.join();
        }
    }

    [[nodiscard]] bool running() const noexcept {
        return m_running.load(std::memory_order_acquire);
    }

private:
    void run() {
        while (m_running.load(std::memory_order_acquire)) {
            std::optional<LogMessage> message = m_queue.pop();
            if (!message) {
                break;
            }
            m_dispatch(*message);
        }

        while (std::optional<LogMessage> leftover = m_queue.tryPop()) {
            m_dispatch(*leftover);
        }
    }

    LogQueue& m_queue;
    DispatchFn m_dispatch;
    std::thread m_thread;
    std::atomic<bool> m_running{false};
    std::mutex m_mutex;
};

}
