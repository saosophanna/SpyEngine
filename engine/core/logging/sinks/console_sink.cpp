#include "core/logging/sinks/console_sink.hpp"

#include <cstdio>

#if defined(_WIN32)
#include <windows.h>
#endif

namespace spy {
namespace {

#if defined(_WIN32)
void enableVirtualTerminal() {
    const DWORD handles[] = {STD_OUTPUT_HANDLE, STD_ERROR_HANDLE};
    for (const DWORD handleId : handles) {
        [[maybe_unused]] const HANDLE handle = GetStdHandle(handleId);
        if (handle == nullptr || handle == INVALID_HANDLE_VALUE) {
            continue;
        }
        DWORD mode = 0;
        if (GetConsoleMode(handle, &mode) == 0) {
            continue;
        }
        SetConsoleMode(handle, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
}
#endif

const char* colorFor(LogLevel level) {
    switch (level) {
    case LogLevel::Trace: return "\x1b[90m";
    case LogLevel::Debug: return "\x1b[36m";
    case LogLevel::Info:  return "\x1b[32m";
    case LogLevel::Warn:  return "\x1b[33m";
    case LogLevel::Error: return "\x1b[31m";
    case LogLevel::Fatal: return "\x1b[1;31m";
    case LogLevel::Off:   return "\x1b[0m";
    }
    return "\x1b[0m";
}

}

ConsoleSink::ConsoleSink(bool colorize, bool splitStreams)
    : m_colorize(colorize), m_splitStreams(splitStreams) {
#if defined(_WIN32)
    if (m_colorize) {
        enableVirtualTerminal();
    }
#endif
}

void ConsoleSink::write(const LogMessage& message, std::string_view formatted) {
    std::FILE* stream = (m_splitStreams && message.level >= LogLevel::Warn) ? stderr : stdout;

    std::lock_guard lock(m_mutex);
    if (m_colorize) {
        std::fputs(colorFor(message.level), stream);
    }
    std::fwrite(formatted.data(), 1, formatted.size(), stream);
    if (m_colorize) {
        std::fputs("\x1b[0m", stream);
    }
    std::fputc('\n', stream);

    if (message.level >= LogLevel::Warn) {
        std::fflush(stream);
    }
}

void ConsoleSink::flush() {
    std::lock_guard lock(m_mutex);
    std::fflush(stdout);
    std::fflush(stderr);
}

}
