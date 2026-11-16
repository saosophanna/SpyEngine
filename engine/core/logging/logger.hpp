#pragma once

#include "core/logging/log_context.hpp"
#include "core/logging/log_level.hpp"

#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace spy {

class Logger {
public:
    explicit Logger(std::string name);

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    [[nodiscard]] std::string_view name() const noexcept { return m_name; }

    void setLevel(LogLevel level) noexcept { m_level = level; }
    void resetLevel() noexcept { m_level.reset(); }

    [[nodiscard]] bool shouldLog(LogLevel level) const noexcept;
    [[nodiscard]] LogLevel effectiveLevel() const noexcept;

    void log(LogLevel level, const LogSource& source, std::string text);

    void trace(const LogSource& source, std::string text) { log(LogLevel::Trace, source, std::move(text)); }
    void debug(const LogSource& source, std::string text) { log(LogLevel::Debug, source, std::move(text)); }
    void info(const LogSource& source, std::string text) { log(LogLevel::Info, source, std::move(text)); }
    void warn(const LogSource& source, std::string text) { log(LogLevel::Warn, source, std::move(text)); }
    void error(const LogSource& source, std::string text) { log(LogLevel::Error, source, std::move(text)); }
    void fatal(const LogSource& source, std::string text) { log(LogLevel::Fatal, source, std::move(text)); }

private:
    std::string m_name;
    std::optional<LogLevel> m_level;
};

}
