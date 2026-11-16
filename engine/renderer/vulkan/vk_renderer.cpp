#include "rhi/device.hpp"
#include "rhi/render_context.hpp"

#if defined(SPY_RENDERER_VULKAN)
#include <vulkan/vulkan.h>
#endif

#include <cstdio>

namespace spy::rhi {

class VkRenderer final {
public:
    VkRenderer() = default;
    ~VkRenderer() { shutdown(); }

    VkRenderer(const VkRenderer&)            = delete;
    VkRenderer& operator=(const VkRenderer&) = delete;

    bool initialize(const DeviceDesc& desc) {
        m_desc = desc;

#if defined(SPY_RENDERER_VULKAN)
        std::uint32_t version = VK_API_VERSION_1_0;
        if (vkEnumerateInstanceVersion(&version) != VK_SUCCESS) {
            std::fprintf(stderr, "[vk] vkEnumerateInstanceVersion failed\n");
            return false;
        }
        std::printf("[vk] Vulkan %u.%u.%u available\n",
                    VK_VERSION_MAJOR(version),
                    VK_VERSION_MINOR(version),
                    VK_VERSION_PATCH(version));

        m_initialized = true;
        return true;
#else
        std::fprintf(stderr, "[vk] built without Vulkan support\n");
        return false;
#endif
    }

    void shutdown() {
        m_initialized = false;
    }

    [[nodiscard]] bool initialized() const { return m_initialized; }

private:
    DeviceDesc m_desc{};
    bool       m_initialized = false;
};

}
