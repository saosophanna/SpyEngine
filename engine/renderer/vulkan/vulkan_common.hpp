#pragma once

#include <cstdint>
#include <string_view>

#if defined(VK_USE_PLATFORM_WIN32_KHR)
#include <windows.h>

#if defined(DeviceCapabilities)
#undef DeviceCapabilities
#endif
#endif

#if defined(SPY_RENDERER_VULKAN)
#include <vulkan/vulkan.h>
#endif

#if defined(SPY_RENDERER_VULKAN)

namespace spy::vulkan
{

struct QueueFamilyIndices
{
    static constexpr std::uint32_t kInvalid = 0xFFFFFFFFu;

    std::uint32_t graphics = kInvalid;
    std::uint32_t present = kInvalid;

    [[nodiscard]] bool hasGraphics() const noexcept
    {
        return graphics != kInvalid;
    }

    [[nodiscard]] bool hasPresent() const noexcept
    {
        return present != kInvalid;
    }

    [[nodiscard]] bool complete() const noexcept
    {
        return hasGraphics() && hasPresent();
    }
};

[[nodiscard]] bool checkResult(VkResult result, std::string_view operation);

}
#endif
