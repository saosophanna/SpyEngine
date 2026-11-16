#include "core/logging/log_formatter.hpp"

#include <array>
#include <chrono>
#include <cstdio>
#include <ctime>
#include <utility>

namespace spy {
namespace {

void appendTime(std::string& out, std::chrono::system_clock::time_point time) {
    const std::time_t seconds = std::chrono::system_clock::to_time_t(time);
    const auto milliseconds =
        std::chrono::duration_cast<std::chrono::milliseconds>(time.time_since_epoch()) % 1000;

    std::tm local{};
#if defined(_WIN32)
    localtime_s(&local, &seconds);
#else
    localtime_r(&seconds, &local);
#endif

    std::array<char, 16> buffer{};
    std::snprintf(buffer.data(), buffer.size(), "%02d:%02d:%02d.%03d",
                  local.tm_hour, local.tm_min, local.tm_sec,
                  static_cast<int>(milliseconds.count()));
    out.append(buffer.data());
}

void appendToken(std::string& out, std::string_view token, const LogMessage& message) {
    if (token == "time") {
        appendTime(out, message.context.time);
    } else if (token == "level") {
        out.append(logLevelName(message.level));
    } else if (token == "level_short") {
        out.append(logLevelShortName(message.level));
    } else if (token == "category") {
        out.append(message.context.category);
    } else if (token == "thread") {
        out.append(std::to_string(message.context.threadId));
    } else if (token == "file") {
        out.append(message.context.file);
    } else if (token == "line") {
        out.append(std::to_string(message.context.line));
    } else if (token == "function") {
        out.append(message.context.function);
    } else if (token == "message") {
        out.append(message.text);
    } else {
        out.push_back('{');
        out.append(token);
        out.push_back('}');
    }
}

}

LogFormatter::LogFormatter()
    : m_pattern(defaultPattern()) {
}

void LogFormatter::setPattern(std::string pattern) {
    m_pattern = pattern.empty() ? std::string(defaultPattern()) : std::move(pattern);
}

std::string LogFormatter::format(const LogMessage& message) const {
    std::string result;
    result.reserve(m_pattern.size() + message.text.size() + 64);

    std::size_t index = 0;
    while (index < m_pattern.size()) {
        if (m_pattern[index] != '{') {
            result.push_back(m_pattern[index]);
            ++index;
            continue;
        }

        const std::size_t close = m_pattern.find('}', index + 1);
        if (close == std::string::npos) {
            result.append(m_pattern.substr(index));
            break;
        }

        const std::string_view token(m_pattern.data() + index + 1, close - index - 1);
        appendToken(result, token, message);
        index = close + 1;
    }
    return result;
}

std::string_view LogFormatter::defaultPattern() noexcept {
    return "[{time}] [{level_short}] [{category}] {message} ({file}:{line})";
}

}
