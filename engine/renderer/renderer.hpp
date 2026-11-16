#pragma once

#include <memory>

#include "rhi/device.hpp"

namespace spy::renderer
{

[[nodiscard]] std::unique_ptr<rhi::Device> createDevice(rhi::Backend backend);

}
