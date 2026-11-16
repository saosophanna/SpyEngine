#include "core/logging/log_manager.hpp"

#include "core/logging/sinks/console_sink.hpp"

#include <chrono>
#include <optional>
#include <thread>
#include <utility>

namespace spy {

LogManager::LogManager() {
    m_config.level = LogLevel::Info;
    m_level.store(m_config.level, std::memory_order_relaxed);
    m_sinks.push_back(std::make_unique<ConsoleSink>(m_config.colorize, true));
    m_running.store(true, std::memory_order_release);
    startWorker(m_config.queueCapacity);
}

LogManager::~LogManager() {
    shutdown();
}

LogManager& LogManager::instance() {
    static LogManager manager;
    return manager;
}

void LogManager::configure(const LogConfig& config) {
    {
        std::lock_guard lock(m_sinkMutex);
        m_config = config;
    }

    m_level.store(config.level, std::memory_order_relaxed);
    m_flushOnError.store(config.flushOnError, std::memory_order_relaxed);
    m_async.store(config.async, std::memory_order_release);

    if (config.async) {
        startWorker(config.queueCapacity);
    } else {
        stopWorker();
    }
}

void LogManager::setLevel(LogLevel level) noexcept {
    m_level.store(level, std::memory_order_relaxed);
    std::lock_guard lock(m_sinkMutex);
    m_config.level = level;
}

LogLevel LogManager::level() const noexcept {
    return m_level.load(std::memory_order_relaxed);
}

void LogManager::addSink(std::unique_ptr<LogSink> sink) {
    if (!sink) {
        return;
    }
    std::lock_guard lock(m_sinkMutex);
    m_sinks.push_back(std::move(sink));
}

void LogManager::clearSinks() {
    std::lock_guard lock(m_sinkMutex);
    m_sinks.clear();
}

std::size_t LogManager::sinkCount() const {
    std::lock_guard lock(m_sinkMutex);
    return m_sinks.size();
}

void LogManager::write(LogLevel level, const LogContext& context, std::string text) {
    if (!logLevelEnabled(level, this->level()) || !running()) {
        return;
    }

    LogMessage message;
    message.level = level;
    message.context = context;
    message.context.category = context.category.empty() ? std::string("engine") : context.category;
    message.text = std::move(text);

    m_submitted.fetch_add(1, std::memory_order_acq_rel);

    if (m_async.load(std::memory_order_acquire)) {
        std::lock_guard lock(m_queueMutex);
        if (m_queue && m_queue->push(std::move(message))) {
            return;
        }
        m_dropped.fetch_add(1, std::memory_order_relaxed);
        m_processed.fetch_add(1, std::memory_order_release);
        return;
    }

    dispatch(message);
    m_processed.fetch_add(1, std::memory_order_release);
}

void LogManager::flush() {
    const std::uint64_t target = m_submitted.load(std::memory_order_acquire);

    while (true) {
        std::optional<LogMessage> message;
        {
            std::lock_guard lock(m_queueMutex);
            if (!m_queue) {
                break;
            }
            message = m_queue->tryPop();
        }
        if (!message) {
            break;
        }
        dispatch(*message);
        m_processed.fetch_add(1, std::memory_order_release);
    }

    const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(500);
    while (m_processed.load(std::memory_order_acquire) < target) {
        if (std::chrono::steady_clock::now() >= deadline) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::microseconds(200));
    }

    std::lock_guard lock(m_sinkMutex);
    for (const auto& sink : m_sinks) {
        if (sink) {
            sink->flush();
        }
    }
}

void LogManager::shutdown() {
    m_running.store(false, std::memory_order_release);
    stopWorker();
    flush();

    std::lock_guard lock(m_sinkMutex);
    m_sinks.clear();
}

void LogManager::dispatch(const LogMessage& message) {
    std::lock_guard lock(m_sinkMutex);
    const std::string formatted = m_formatter.format(message);

    for (const auto& sink : m_sinks) {
        if (sink && sink->accepts(message.level)) {
            sink->write(message, formatted);
        }
    }

    if (m_flushOnError.load(std::memory_order_relaxed) && message.level >= LogLevel::Error) {
        for (const auto& sink : m_sinks) {
            if (sink) {
                sink->flush();
            }
        }
    }
}

void LogManager::startWorker(std::size_t capacity) {
    std::lock_guard lock(m_queueMutex);
    if (m_worker && m_worker->running()) {
        return;
    }

    m_queue = std::make_unique<internal::LogQueue>(capacity);
    m_worker = std::make_unique<internal::LogWorker>(
        *m_queue, [this](const LogMessage& message) {
            dispatch(message);
            m_processed.fetch_add(1, std::memory_order_release);
        });
    m_worker->start();
}

void LogManager::stopWorker() {
    std::unique_ptr<internal::LogQueue> queue;
    std::unique_ptr<internal::LogWorker> worker;

    {
        std::lock_guard lock(m_queueMutex);
        queue = std::move(m_queue);
        worker = std::move(m_worker);
    }

    if (worker) {
        worker->stop();
    }
}

LogManager& logManager() {
    return LogManager::instance();
}

void logWrite(LogLevel level, const LogSource& source, std::string text) {
    LogManager::instance().write(
        level,
        LogContext::fromSource(source, "engine", std::chrono::system_clock::now(), currentThreadId()),
        std::move(text));
}

void logFlush() {
    LogManager::instance().flush();
}

}
