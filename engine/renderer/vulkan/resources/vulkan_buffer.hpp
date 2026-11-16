#pragma once

#include <cstddef>
#include <cstdint>

#include "rhi/buffer.hpp"

#include "vulkan/vulkan_common.hpp"

#if defined(SPY_RENDERER_VULKAN)

namespace spy::vulkan
{

class VulkanContext;

class VulkanBuffer final : public rhi::Buffer
{
public:
    VulkanBuffer() = default;
    ~VulkanBuffer() override;

    VulkanBuffer(const VulkanBuffer&) = delete;
    VulkanBuffer& operator=(const VulkanBuffer&) = delete;

    [[nodiscard]] bool initialize(VulkanContext& context, const rhi::BufferDesc& desc);
    void shutdown();

    [[nodiscard]] std::size_t size() const override;

    [[nodiscard]] bool write(const void* data, std::size_t bytes, std::size_t offset) override;

    [[nodiscard]] void* map() override;
    void unmap() override;

    [[nodiscard]] VkBuffer handle() const noexcept;

private:
    VulkanContext* m_context = nullptr;
    VkBuffer m_buffer = VK_NULL_HANDLE;
    VkDeviceMemory m_memory = VK_NULL_HANDLE;
    std::size_t m_size = 0;
    bool m_hostVisible = false;
    void* m_mapped = nullptr;
};

}

#endif
