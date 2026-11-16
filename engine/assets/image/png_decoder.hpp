#pragma once

#include <span>
#include <string_view>

#include "assets/image/image.hpp"

namespace spy::assets
{

[[nodiscard]] Image decodePng(std::span<const std::uint8_t> data);
[[nodiscard]] Image decodePngFile(std::string_view path);

}
