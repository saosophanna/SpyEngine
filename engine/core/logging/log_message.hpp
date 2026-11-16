#pragma once

#include "core/logging/log_context.hpp"
#include "core/logging/log_level.hpp"

#include <string>

namespace spy {

struct LogMessage {
    LogLevel level = LogLevel::Info;
    LogContext context{};
    std::string text{};
};

}
