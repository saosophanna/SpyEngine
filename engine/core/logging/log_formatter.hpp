#pragma once

#include "core/logging/log_message.hpp"

#include <string>
#include <string_view>

namespace spy {

class LogFormatter {
public:
    LogFormatter();

    void setPattern(std::string pattern);
    [[nodiscard]] const std::string& pattern() const noexcept { return m_pattern; }

    [[nodiscard]] std::string format(const LogMessage& message) const;

    static std::string_view defaultPattern() noexcept;

private:
    std::string m_pattern;
};

}
