#include "core/logging/logger.hpp"

#include "core/logging/log_manager.hpp"

#include <chrono>
#include <utility>

namespace spy {

Logger::Logger(std::string name)
    : m_name(std::move(name)) {
}

bool Logger::shouldLog(LogLevel level) const noexcept {
    return logLevelEnabled(level, effectiveLevel());
}

LogLevel Logger::effectiveLevel() const noexcept {
    return m_level.value_or(LogManager::instance().level());
}

void Logger::log(LogLevel level, const LogSource& source, std::string text) {
    if (!shouldLog(level)) {
        return;
    }

    LogContext context = LogContext::fromSource(source, m_name,
                                                std::chrono::system_clock::now(),
                                                currentThreadId());
    LogManager::instance().write(level, context, std::move(text));
}

}
