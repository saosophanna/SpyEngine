#pragma once

#include <cstdint>
#include <optional>
#include <string_view>

namespace spy {

enum class LogLevel : std::uint8_t {
    Trace,
    Debug,
    Info,
    Warn,
    Error,
    Fatal,
    Off,
};

constexpr bool logLevelEnabled(LogLevel message, LogLevel threshold) noexcept {
    return message != LogLevel::Off &&
           threshold != LogLevel::Off &&
           static_cast<std::uint8_t>(message) >= static_cast<std::uint8_t>(threshold);
}

constexpr std::string_view logLevelName(LogLevel level) noexcept {
    switch (level) {
    case LogLevel::Trace: return "trace";
    case LogLevel::Debug: return "debug";
    case LogLevel::Info:  return "info";
    case LogLevel::Warn:  return "warn";
    case LogLevel::Error: return "error";
    case LogLevel::Fatal: return "fatal";
    case LogLevel::Off:   return "off";
    }
    return "unknown";
}

constexpr std::string_view logLevelShortName(LogLevel level) noexcept {
    switch (level) {
    case LogLevel::Trace: return "TRC";
    case LogLevel::Debug: return "DBG";
    case LogLevel::Info:  return "INF";
    case LogLevel::Warn:  return "WRN";
    case LogLevel::Error: return "ERR";
    case LogLevel::Fatal: return "FTL";
    case LogLevel::Off:   return "OFF";
    }
    return "UNK";
}

inline std::optional<LogLevel> logLevelFromString(std::string_view name) {
    if (name == "trace") return LogLevel::Trace;
    if (name == "debug") return LogLevel::Debug;
    if (name == "info")  return LogLevel::Info;
    if (name == "warn")  return LogLevel::Warn;
    if (name == "error") return LogLevel::Error;
    if (name == "fatal") return LogLevel::Fatal;
    if (name == "off")   return LogLevel::Off;
    return std::nullopt;
}

}
