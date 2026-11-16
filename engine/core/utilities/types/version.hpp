#pragma once

#include "core/utilities/types/types.hpp"

#include <compare>
#include <string>

namespace spy {

struct Version
{
    std::uint32_t major = 0;
    std::uint32_t minor = 0;
    std::uint32_t patch = 0;

    friend constexpr bool operator==(const Version& lhs, const Version& rhs) noexcept = default;
    friend constexpr auto operator<=>(const Version& lhs, const Version& rhs) noexcept = default;

    [[nodiscard]] constexpr bool isCompatibleWith(const Version& required) const noexcept
    {
        if (major != required.major)
        {
            return false;
        }

        if (minor != required.minor)
        {
            return minor > required.minor;
        }

        return patch >= required.patch;
    }

    [[nodiscard]] std::string toString() const
    {
        return std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);
    }
};

}
