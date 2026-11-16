#if defined(SPY_RENDERER_VULKAN)

#include "vulkan/vulkan_common.hpp"

#include "core/logging/log_macros.hpp"

namespace spy::vulkan
{

bool checkResult(VkResult result, std::string_view operation)
{
    if (result == VK_SUCCESS)
    {
        return true;
    }

    SPY_LOG_ERROR("Vulkan operation {} failed with result {}", operation,
                  static_cast<std::int32_t>(result));
    return false;
}

}

#endif
