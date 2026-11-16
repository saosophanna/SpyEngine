#pragma once

#include <cstdint>
#include <string>

namespace spy::rhi
{

struct DeviceCapabilities
{
    std::string deviceName;
    std::uint32_t apiVersion = 0;
    std::uint32_t maxTextureSize = 0;
    std::uint32_t maxFramesInFlight = 0;
    std::uint32_t maxDescriptorSets = 0;
    std::uint32_t maxPushConstantsSize = 0;
    bool supportsCompute = false;
    bool supportsGeometry = false;
    bool supportsSamplerAnisotropy = false;
};

}
