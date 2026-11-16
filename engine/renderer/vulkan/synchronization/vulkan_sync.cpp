#if defined(SPY_RENDERER_VULKAN)

#include "vulkan/synchronization/vulkan_sync.hpp"

namespace spy::vulkan
{

VulkanFence::~VulkanFence()
{
    shutdown();
}

bool VulkanFence::initialize(VkDevice device, bool signaled)
{
    shutdown();

    if (device == VK_NULL_HANDLE)
    {
        return false;
    }

    VkFenceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    createInfo.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

    if (!checkResult(vkCreateFence(device, &createInfo, nullptr, &m_fence), "vkCreateFence"))
    {
        m_fence = VK_NULL_HANDLE;
        return false;
    }

    m_device = device;
    return true;
}

void VulkanFence::shutdown()
{
    if (m_fence != VK_NULL_HANDLE && m_device != VK_NULL_HANDLE)
    {
        vkDestroyFence(m_device, m_fence, nullptr);
    }

    m_fence = VK_NULL_HANDLE;
    m_device = VK_NULL_HANDLE;
}

bool VulkanFence::isSignaled() const
{
    if (m_fence == VK_NULL_HANDLE)
    {
        return false;
    }

    return vkGetFenceStatus(m_device, m_fence) == VK_SUCCESS;
}

bool VulkanFence::wait(std::uint64_t timeoutNanoseconds)
{
    if (m_fence == VK_NULL_HANDLE)
    {
        return false;
    }

    return vkWaitForFences(m_device, 1, &m_fence, VK_TRUE, timeoutNanoseconds) == VK_SUCCESS;
}

void VulkanFence::reset()
{
    if (m_fence != VK_NULL_HANDLE)
    {
        vkResetFences(m_device, 1, &m_fence);
    }
}

VulkanSemaphore::~VulkanSemaphore()
{
    shutdown();
}

bool VulkanSemaphore::initialize(VkDevice device)
{
    shutdown();

    if (device == VK_NULL_HANDLE)
    {
        return false;
    }

    VkSemaphoreCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    if (!checkResult(vkCreateSemaphore(device, &createInfo, nullptr, &m_semaphore),
                     "vkCreateSemaphore"))
    {
        m_semaphore = VK_NULL_HANDLE;
        return false;
    }

    m_device = device;
    return true;
}

void VulkanSemaphore::shutdown()
{
    if (m_semaphore != VK_NULL_HANDLE && m_device != VK_NULL_HANDLE)
    {
        vkDestroySemaphore(m_device, m_semaphore, nullptr);
    }

    m_semaphore = VK_NULL_HANDLE;
    m_device = VK_NULL_HANDLE;
}

bool VulkanSemaphore::isValid() const
{
    return m_semaphore != VK_NULL_HANDLE;
}

}

#endif
