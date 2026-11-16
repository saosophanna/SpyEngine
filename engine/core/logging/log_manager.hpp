#pragma once

#include "core/logging/log_config.hpp"
#include "core/logging/log_context.hpp"
#include "core/logging/log_formatter.hpp"
#include "core/logging/log_level.hpp"
#include "core/logging/log_message.hpp"
#include "core/logging/log_sink.hpp"
#include "core/logging/internal/log_queue.hpp"
#include "core/logging/internal/log_worker.hpp"

#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <vector>

namespace spy {

class LogManager final {
public:
    static LogManager& instance();

    void configure(const LogConfig& config);

    [[nodiscard]] const LogConfig& config() const noexcept { return m_config; }
    void setLevel(LogLevel level) noexcept;
    [[nodiscard]] LogLevel level() const noexcept;

    void addSink(std::unique_ptr<LogSink> sink);
    void clearSinks();
    [[nodiscard]] std::size_t sinkCount() const;

    void write(LogLevel level, const LogContext& context, std::string text);
    void flush();
    void shutdown();

    [[nodiscard]] std::uint64_t droppedMessages() const noexcept {
        return m_dropped.load(std::memory_order_relaxed);
    }

    [[nodiscard]] bool running() const noexcept {
        return m_running.load(std::memory_order_acquire);
    }

private:
    LogManager();
    ~LogManager();

    LogManager(const LogManager&) = delete;
    LogManager& operator=(const LogManager&) = delete;

    void dispatch(const LogMessage& message);
    void startWorker(std::size_t capacity);
    void stopWorker();

    mutable std::mutex m_sinkMutex;
    mutable std::mutex m_queueMutex;
    std::vector<std::unique_ptr<LogSink>> m_sinks;
    std::unique_ptr<internal::LogQueue> m_queue;
    std::unique_ptr<internal::LogWorker> m_worker;
    LogFormatter m_formatter;
    LogConfig m_config;
    std::atomic<LogLevel> m_level{LogLevel::Info};
    std::atomic<std::uint64_t> m_dropped{0};
    std::atomic<std::uint64_t> m_submitted{0};
    std::atomic<std::uint64_t> m_processed{0};
    std::atomic<bool> m_running{false};
    std::atomic<bool> m_async{true};
    std::atomic<bool> m_flushOnError{true};
};

LogManager& logManager();
void logWrite(LogLevel level, const LogSource& source, std::string text);
void logFlush();

}
