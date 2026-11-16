#pragma once

#include "core/math/common/constants.hpp"
#include "core/math/common/math_utils.hpp"

namespace spy::math {

struct Angle
{
    float radians = 0.0f;

    [[nodiscard]] static constexpr Angle fromRadians(float radiansValue)
    {
        return Angle{radiansValue};
    }

    [[nodiscard]] static constexpr Angle fromDegrees(float degreesValue)
    {
        return Angle{degreesToRadians(degreesValue)};
    }

    [[nodiscard]] constexpr float degrees() const
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

[[nodiscard]] inline float angleDifference(float first, float second)
{
    return wrap(second - first + constants::kPi, 0.0f, constants::kTwoPi) - constants::kPi;
}

}
