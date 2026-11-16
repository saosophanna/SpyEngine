#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <string>
#include <string_view>
#include <thread>

namespace spy {

struct LogSource {
    std::string_view file{};
    std::uint32_t line = 0;
    std::string_view function{};
};

struct LogContext {
    std::string category{};
    std::string_view file{};
    std::uint32_t line = 0;
    std::string_view function{};
    std::chrono::system_clock::time_point time{};
    std::uint64_t threadId = 0;

    static LogContext fromSource(const LogSource& source,
                                 std::string_view category,
                                 std::chrono::system_clock::time_point time,
                                 std::uint64_t threadId) {
        LogContext context;
        context.category = std::string(category);
        context.file = source.file;
        context.line = source.line;
        context.function = source.function;
        context.time = time;
        context.threadId = threadId;
        return context;
    }
};

inline std::uint64_t currentThreadId() noexcept {
    thread_local const std::uint64_t id = [] {
        const auto hashed = std::hash<std::thread::id>{}(std::this_thread::get_id());
        return static_cast<std::uint64_t>(hashed);
    }();
    return id;
}

}
