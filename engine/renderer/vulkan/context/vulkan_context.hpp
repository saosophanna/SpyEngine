#pragma once

#include <cstdint>

#include "rhi/device.hpp"
#include "rhi/device_capabilities.hpp"
#include "rhi/native_window.hpp"

#include "vulkan/vulkan_common.hpp"

#if defined(SPY_RENDERER_VULKAN)

namespace spy::vulkan
{

class VulkanContext
{
public:
    VulkanContext() = default;
    ~VulkanContext();

    VulkanContext(const VulkanContext&) = delete;
    VulkanContext& operator=(const VulkanContext&) = delete;

    [[nodiscard]] bool initialize(const rhi::DeviceDesc& desc);
    [[nodiscard]] bool initializeSurface(const rhi::NativeWindow& window);
    void shutdown();
    void waitIdle();

    [[nodiscard]] VkInstance instance() const noexcept { return m_instance; }
    [[nodiscard]] VkPhysicalDevice physicalDevice() const noexcept { return m_physicalDevice; }
    [[nodiscard]] VkDevice device() const noexcept { return m_device; }
    [[nodiscard]] VkSurfaceKHR surface() const noexcept { return m_surface; }
    [[nodiscard]] VkQueue graphicsQueue() const noexcept { return m_graphicsQueue; }
    [[nodiscard]] VkQueue presentQueue() const noexcept { return m_presentQueue; }
    [[nodiscard]] const QueueFamilyIndices& queueFamilies() const noexcept { return m_queueFamilies; }
    [[nodiscard]] const rhi::DeviceCapabilities& capabilities() const noexcept { return m_capabilities; }
    [[nodiscard]] bool valid() const noexcept { return m_device != VK_NULL_HANDLE; }

private:
    [[nodiscard]] bool createInstance(const rhi::DeviceDesc& desc);
    [[nodiscard]] bool createDebugMessenger();
    [[nodiscard]] bool createSurface(const rhi::NativeWindow& window);
    [[nodiscard]] bool selectPhysicalDevice();
    [[nodiscard]] bool createLogicalDevice();
    void fillCapabilities(const rhi::DeviceDesc& desc);
    [[nodiscard]] QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) const;
    [[nodiscard]] bool deviceExtensionsSupported(VkPhysicalDevice device) const;

    VkInstance m_instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;
    QueueFamilyIndices m_queueFamilies{};
    rhi::DeviceCapabilities m_capabilities{};
    bool m_validationEnabled = false;
};

}

#endif
