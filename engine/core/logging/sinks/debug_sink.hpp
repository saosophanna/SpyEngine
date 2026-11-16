#pragma once

#include "core/logging/log_sink.hpp"

#include <string_view>

namespace spy {

class DebugSink final : public LogSink {
public:
    DebugSink() = default;

    DebugSink(const DebugSink&) = delete;
    DebugSink& operator=(const DebugSink&) = delete;

    void write(const LogMessage& message, std::string_view formatted) override;
    void flush() override;
};

}
