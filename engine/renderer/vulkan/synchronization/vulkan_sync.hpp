#pragma once

#include <cstdint>

#include "rhi/fence.hpp"
#include "rhi/semaphore.hpp"

#include "vulkan/vulkan_common.hpp"

#if defined(SPY_RENDERER_VULKAN)

namespace spy::vulkan
{

class VulkanFence final : public rhi::Fence
{
public:
    VulkanFence() = default;
    ~VulkanFence() override;

    VulkanFence(const VulkanFence&) = delete;
    VulkanFence& operator=(const VulkanFence&) = delete;

    [[nodiscard]] bool initialize(VkDevice device, bool signaled);
    void shutdown();

    [[nodiscard]] bool isSignaled() const override;
    [[nodiscard]] bool wait(
        std::uint64_t timeoutNanoseconds = std::numeric_limits<std::uint64_t>::max()) override;
    void reset() override;

    [[nodiscard]] VkFence handle() const noexcept { return m_fence; }

private:
    VkDevice m_device = VK_NULL_HANDLE;
    VkFence m_fence = VK_NULL_HANDLE;
};

class VulkanSemaphore final : public rhi::Semaphore
{
public:
    VulkanSemaphore() = default;
    ~VulkanSemaphore() override;

    VulkanSemaphore(const VulkanSemaphore&) = delete;
    VulkanSemaphore& operator=(const VulkanSemaphore&) = delete;

    [[nodiscard]] bool initialize(VkDevice device);
    void shutdown();

    [[nodiscard]] bool isValid() const override;

    [[nodiscard]] VkSemaphore handle() const noexcept { return m_semaphore; }

private:
    VkDevice m_device = VK_NULL_HANDLE;
    VkSemaphore m_semaphore = VK_NULL_HANDLE;
};

}

#endif
