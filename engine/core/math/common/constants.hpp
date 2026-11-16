#pragma once

#include "core/utilities/types/types.hpp"

namespace spy::math::constants {

inline constexpr float32 kPi = 3.14159265358979323846f;
inline constexpr float32 kTwoPi = 2.0f * kPi;
inline constexpr float32 kHalfPi = 0.5f * kPi;
inline constexpr float32 kQuarterPi = 0.25f * kPi;
inline constexpr float32 kDegToRad = kPi / 180.0f;
inline constexpr float32 kRadToDeg = 180.0f / kPi;
inline constexpr float32 kEpsilon = 1.0e-6f;
inline constexpr float32 kTolerance = 1.0e-5f;

}
