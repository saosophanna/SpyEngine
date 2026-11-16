#pragma once

#include "core/logging/log_level.hpp"
#include "core/logging/log_message.hpp"

#include <atomic>
#include <string_view>

namespace spy {

class LogSink {
public:
    virtual ~LogSink() = default;

    LogSink(const LogSink&) = delete;
    LogSink& operator=(const LogSink&) = delete;

    void setLevel(LogLevel level) noexcept {
        m_level.store(level, std::memory_order_relaxed);
    }

    [[nodiscard]] LogLevel level() const noexcept {
        return m_level.load(std::memory_order_relaxed);
    }

    [[nodiscard]] bool accepts(LogLevel messageLevel) const noexcept {
        return logLevelEnabled(messageLevel, level());
    }

    virtual void write(const LogMessage& message, std::string_view formatted) = 0;
    virtual void flush() {}

protected:
    LogSink() = default;

private:
    std::atomic<LogLevel> m_level{LogLevel::Trace};
};

}
