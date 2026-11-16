#pragma once

#include "rhi/pipeline.hpp"

#include "vulkan/vulkan_common.hpp"

#if defined(SPY_RENDERER_VULKAN)

namespace spy::vulkan
{

class VulkanContext;

class VulkanPipeline final : public rhi::Pipeline
{
public:
    VulkanPipeline() = default;
    ~VulkanPipeline() override;

    VulkanPipeline(const VulkanPipeline&) = delete;
    VulkanPipeline& operator=(const VulkanPipeline&) = delete;

    [[nodiscard]] bool initialize(VulkanContext& context,
                                  const rhi::PipelineDesc& desc,
                                  VkRenderPass renderPass,
                                  VkDescriptorSetLayout textureSetLayout);
    void shutdown();

    [[nodiscard]] bool isValid() const override;

    [[nodiscard]] VkPipeline handle() const noexcept;
    [[nodiscard]] VkPipelineLayout layout() const noexcept;

private:
    VulkanContext* m_context = nullptr;
    VkPipeline m_pipeline = VK_NULL_HANDLE;
    VkPipelineLayout m_layout = VK_NULL_HANDLE;
};

}

#endif
