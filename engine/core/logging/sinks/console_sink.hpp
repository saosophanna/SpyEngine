#pragma once

#include "core/logging/log_sink.hpp"

#include <mutex>
#include <string_view>

namespace spy {

class ConsoleSink final : public LogSink {
public:
    explicit ConsoleSink(bool colorize = true, bool splitStreams = true);

    ConsoleSink(const ConsoleSink&) = delete;
    ConsoleSink& operator=(const ConsoleSink&) = delete;

    [[nodiscard]] bool colorized() const noexcept { return m_colorize; }
    [[nodiscard]] bool splitStreams() const noexcept { return m_splitStreams; }

    void write(const LogMessage& message, std::string_view formatted) override;
    void flush() override;

private:
    bool m_colorize;
    bool m_splitStreams;
    std::mutex m_mutex;
};

}
