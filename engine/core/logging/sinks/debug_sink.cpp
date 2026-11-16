#include "core/logging/sinks/debug_sink.hpp"

#include <cstdio>
#include <mutex>
#include <string>

#if defined(_WIN32)
#include <windows.h>
#endif

namespace spy {
namespace {

std::mutex g_debugMutex;

}

void DebugSink::write(const LogMessage&, std::string_view formatted) {
    std::string line(formatted);
    line.push_back('\n');

    std::lock_guard lock(g_debugMutex);
#if defined(_WIN32)
    OutputDebugStringA(line.c_str());
#else
    std::fputs(line.c_str(), stderr);
#endif
}

void DebugSink::flush() {
}

}
