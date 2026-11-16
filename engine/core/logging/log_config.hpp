#pragma once

#include "core/logging/log_level.hpp"

#include <cstddef>

namespace spy {

struct LogConfig {
    LogLevel level = LogLevel::Info;
    bool async = true;
    std::size_t queueCapacity = 8192;
    bool colorize = true;
    bool flushOnError = true;
};

}
