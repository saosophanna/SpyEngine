#pragma once

#include "core/logging/log_level.hpp"
#include "core/logging/log_manager.hpp"

#include <format>
#include <string>
#include <utility>

namespace spy {

template <typename... Args>
std::string formatLog(std::format_string<Args...> fmt, Args&&... args) {
    try {
        return std::format(fmt, std::forward<Args>(args)...);
    } catch (...) {
        return std::string("<log format error>");
    }
}

}

#define SPY_LOG_SOURCE ::spy::LogSource{__FILE__, static_cast<std::uint32_t>(__LINE__), __func__}

#define SPY_LOG_TRACE(...) ::spy::logWrite(::spy::LogLevel::Trace, SPY_LOG_SOURCE, ::spy::formatLog(__VA_ARGS__))
#define SPY_LOG_DEBUG(...) ::spy::logWrite(::spy::LogLevel::Debug, SPY_LOG_SOURCE, ::spy::formatLog(__VA_ARGS__))
#define SPY_LOG_INFO(...)  ::spy::logWrite(::spy::LogLevel::Info,  SPY_LOG_SOURCE, ::spy::formatLog(__VA_ARGS__))
#define SPY_LOG_WARN(...)  ::spy::logWrite(::spy::LogLevel::Warn,  SPY_LOG_SOURCE, ::spy::formatLog(__VA_ARGS__))
#define SPY_LOG_ERROR(...) ::spy::logWrite(::spy::LogLevel::Error, SPY_LOG_SOURCE, ::spy::formatLog(__VA_ARGS__))
#define SPY_LOG_FATAL(...) ::spy::logWrite(::spy::LogLevel::Fatal, SPY_LOG_SOURCE, ::spy::formatLog(__VA_ARGS__))

#define SPY_LOG_TRACE_TO(logger, ...) (logger).trace(SPY_LOG_SOURCE, ::spy::formatLog(__VA_ARGS__))
#define SPY_LOG_DEBUG_TO(logger, ...) (logger).debug(SPY_LOG_SOURCE, ::spy::formatLog(__VA_ARGS__))
#define SPY_LOG_INFO_TO(logger, ...)  (logger).info(SPY_LOG_SOURCE, ::spy::formatLog(__VA_ARGS__))
#define SPY_LOG_WARN_TO(logger, ...)  (logger).warn(SPY_LOG_SOURCE, ::spy::formatLog(__VA_ARGS__))
#define SPY_LOG_ERROR_TO(logger, ...) (logger).error(SPY_LOG_SOURCE, ::spy::formatLog(__VA_ARGS__))
#define SPY_LOG_FATAL_TO(logger, ...) (logger).fatal(SPY_LOG_SOURCE, ::spy::formatLog(__VA_ARGS__))
