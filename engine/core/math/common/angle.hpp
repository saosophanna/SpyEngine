#pragma once

#include "core/math/common/constants.hpp"
#include "core/math/common/math_utils.hpp"
#include "core/utilities/types/types.hpp"

namespace spy::math {

struct Angle
{
    float32 radians = 0.0f;

    [[nodiscard]] static constexpr Angle fromRadians(float32 radiansValue)
    {
        return Angle{radiansValue};
    }

    [[nodiscard]] static constexpr Angle fromDegrees(float32 degreesValue)
    {
        return Angle{degreesToRadians(degreesValue)};
    }

    [[nodiscard]] constexpr float32 degrees() const
    {
        return radiansToDegrees(radians);
    }

    [[nodiscard]] inline Angle normalized() const
    {
        return Angle{wrap(radians, 0.0f, constants::kTwoPi)};
    }

    [[nodiscard]] constexpr Angle operator+(Angle other) const
    {
        return Angle{radians + other.radians};
    }

    [[nodiscard]] constexpr Angle operator-(Angle other) const
    {
        return Angle{radians - other.radians};
    }

    [[nodiscard]] constexpr Angle operator-() const
    {
        return Angle{-radians};
    }

    constexpr Angle& operator+=(Angle other)
    {
        radians += other.radians;
        return *this;
    }

    constexpr Angle& operator-=(Angle other)
    {
        radians -= other.radians;
        return *this;
    }

    [[nodiscard]] constexpr bool operator==(Angle other) const
    {
        return radians == other.radians;
    }

    [[nodiscard]] constexpr bool operator!=(Angle other) const
    {
        return radians != other.radians;
    }
};

[[nodiscard]] inline float32 angleDifference(float32 first, float32 second)
{
    return wrap(second - first + constants::kPi, 0.0f, constants::kTwoPi) - constants::kPi;
}

}
