#include "renderer/renderer.hpp"

#if defined(SPY_RENDERER_VULKAN)
#include "vulkan/device/vulkan_device.hpp"
#endif

namespace spy::renderer
{

std::unique_ptr<rhi::Device> createDevice(rhi::Backend backend)
{
#if defined(SPY_RENDERER_VULKAN)
    if (backend == rhi::Backend::Vulkan)
    {
        return std::make_unique<vulkan::VulkanDevice>();
    }
#endif

    (void)backend;
    return nullptr;
}

}
