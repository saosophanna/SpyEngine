#if defined(SPY_RENDERER_VULKAN)

#include "vulkan/resources/vulkan_pipeline.hpp"

#include <array>
#include <vector>

#include "vulkan/context/vulkan_context.hpp"

namespace spy::vulkan
{

namespace
{

VkFormat toVulkanFormat(rhi::VertexFormat format)
{
    switch (format)
    {
        case rhi::VertexFormat::Float2:
            return VK_FORMAT_R32G32_SFLOAT;
        case rhi::VertexFormat::Float4:
            return VK_FORMAT_R32G32B32A32_SFLOAT;
        case rhi::VertexFormat::Float3:
        default:
            return VK_FORMAT_R32G32B32_SFLOAT;
    }
}

VkShaderStageFlagBits toVulkanStage(rhi::ShaderStage stage)
{
    return stage == rhi::ShaderStage::Fragment ? VK_SHADER_STAGE_FRAGMENT_BIT
                                               : VK_SHADER_STAGE_VERTEX_BIT;
}

VkPrimitiveTopology toVulkanTopology(rhi::PrimitiveTopology topology)
{
    switch (topology)
    {
        case rhi::PrimitiveTopology::TriangleStrip:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        case rhi::PrimitiveTopology::LineList:
            return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        case rhi::PrimitiveTopology::TriangleList:
        default:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    }
}

VkCullModeFlags toVulkanCull(rhi::CullMode cull)
{
    switch (cull)
    {
        case rhi::CullMode::Back:
            return VK_CULL_MODE_BACK_BIT;
        case rhi::CullMode::Front:
            return VK_CULL_MODE_FRONT_BIT;
        case rhi::CullMode::None:
        default:
            return VK_CULL_MODE_NONE;
    }
}

VkPipelineColorBlendAttachmentState blendAttachment(rhi::BlendMode blend)
{
    VkPipelineColorBlendAttachmentState attachment{};
    attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    switch (blend)
    {
        case rhi::BlendMode::None:
            attachment.blendEnable = VK_FALSE;
            return attachment;
        case rhi::BlendMode::Additive:
            attachment.blendEnable = VK_TRUE;
            attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
            attachment.colorBlendOp = VK_BLEND_OP_ADD;
            attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            attachment.alphaBlendOp = VK_BLEND_OP_ADD;
            return attachment;
        case rhi::BlendMode::Alpha:
        default:
            attachment.blendEnable = VK_TRUE;
            attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            attachment.colorBlendOp = VK_BLEND_OP_ADD;
            attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            attachment.alphaBlendOp = VK_BLEND_OP_ADD;
            return attachment;
    }
}

VkShaderModule createShaderModule(VulkanContext& context, const rhi::ShaderModule& module)
{
    if (module.code == nullptr || module.size == 0)
    {
        return VK_NULL_HANDLE;
    }

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = module.size;
    createInfo.pCode = static_cast<const std::uint32_t*>(module.code);

    VkShaderModule shaderModule = VK_NULL_HANDLE;

    if (!checkResult(vkCreateShaderModule(context.device(), &createInfo, nullptr, &shaderModule),
                     "vkCreateShaderModule"))
    {
        return VK_NULL_HANDLE;
    }

    return shaderModule;
}

}

VulkanPipeline::~VulkanPipeline()
{
    shutdown();
}

bool VulkanPipeline::initialize(VulkanContext& context,
                                const rhi::PipelineDesc& desc,
                                VkRenderPass renderPass,
                                VkDescriptorSetLayout textureSetLayout)
{
    shutdown();

    if (desc.stages == nullptr || desc.stageCount == 0 || renderPass == VK_NULL_HANDLE)
    {
        return false;
    }

    m_context = &context;

    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages{};
    std::array<VkShaderModule, 2> shaderModules{};
    std::uint32_t shaderStageCount = 0;

    for (std::uint32_t index = 0; index < desc.stageCount && index < 2; ++index)
    {
        const VkShaderModule shaderModule = createShaderModule(context, desc.stages[index]);

        if (shaderModule == VK_NULL_HANDLE)
        {
            for (std::uint32_t created = 0; created < shaderStageCount; ++created)
            {
                vkDestroyShaderModule(context.device(), shaderModules[created], nullptr);
            }

            m_context = nullptr;
            return false;
        }

        shaderModules[shaderStageCount] = shaderModule;

        VkPipelineShaderStageCreateInfo& stage = shaderStages[shaderStageCount];
        stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage.stage = toVulkanStage(desc.stages[index].stage);
        stage.module = shaderModule;
        stage.pName = "main";
        ++shaderStageCount;
    }

    std::vector<VkVertexInputAttributeDescription> attributes(desc.attributeCount);

    for (std::uint32_t index = 0; index < desc.attributeCount; ++index)
    {
        attributes[index].location = desc.attributes[index].location;
        attributes[index].binding = 0;
        attributes[index].format = toVulkanFormat(desc.attributes[index].format);
        attributes[index].offset = desc.attributes[index].offset;
    }

    VkVertexInputBindingDescription binding{};
    binding.binding = 0;
    binding.stride = desc.vertexStride;
    binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkPipelineVertexInputStateCreateInfo vertexInput{};
    vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInput.vertexBindingDescriptionCount = desc.vertexStride > 0 ? 1 : 0;
    vertexInput.pVertexBindingDescriptions = desc.vertexStride > 0 ? &binding : nullptr;
    vertexInput.vertexAttributeDescriptionCount = desc.attributeCount;
    vertexInput.pVertexAttributeDescriptions = attributes.empty() ? nullptr : attributes.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = toVulkanTopology(desc.topology);
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterization{};
    rasterization.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterization.depthClampEnable = VK_FALSE;
    rasterization.rasterizerDiscardEnable = VK_FALSE;
    rasterization.polygonMode = VK_POLYGON_MODE_FILL;
    rasterization.lineWidth = 1.0f;
    rasterization.cullMode = toVulkanCull(desc.cull);
    rasterization.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterization.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.sampleShadingEnable = VK_FALSE;

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = desc.depth != rhi::DepthMode::Disabled ? VK_TRUE : VK_FALSE;
    depthStencil.depthWriteEnable =
        desc.depth == rhi::DepthMode::ReadWrite ? VK_TRUE : VK_FALSE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

    const VkPipelineColorBlendAttachmentState colorAttachment = blendAttachment(desc.blend);

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorAttachment;

    const std::array<VkDynamicState, 2> dynamicStates{VK_DYNAMIC_STATE_VIEWPORT,
                                                     VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<std::uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    VkPushConstantRange pushRange{};
    pushRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushRange.offset = 0;
    pushRange.size = desc.pushConstantSize;

    VkPipelineLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.setLayoutCount = desc.hasTexture ? 1 : 0;
    layoutInfo.pSetLayouts = desc.hasTexture ? &textureSetLayout : nullptr;
    layoutInfo.pushConstantRangeCount = desc.pushConstantSize > 0 ? 1 : 0;
    layoutInfo.pPushConstantRanges = desc.pushConstantSize > 0 ? &pushRange : nullptr;

    if (!checkResult(vkCreatePipelineLayout(context.device(), &layoutInfo, nullptr, &m_layout),
                     "vkCreatePipelineLayout"))
    {
        for (std::uint32_t index = 0; index < shaderStageCount; ++index)
        {
            vkDestroyShaderModule(context.device(), shaderModules[index], nullptr);
        }

        m_layout = VK_NULL_HANDLE;
        m_context = nullptr;
        return false;
    }

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = shaderStageCount;
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.pVertexInputState = &vertexInput;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterization;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = m_layout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;

    const VkResult result = vkCreateGraphicsPipelines(context.device(),
                                                      VK_NULL_HANDLE,
                                                      1,
                                                      &pipelineInfo,
                                                      nullptr,
                                                      &m_pipeline);

    for (std::uint32_t index = 0; index < shaderStageCount; ++index)
    {
        vkDestroyShaderModule(context.device(), shaderModules[index], nullptr);
    }

    if (!checkResult(result, "vkCreateGraphicsPipelines"))
    {
        shutdown();
        return false;
    }

    return true;
}

void VulkanPipeline::shutdown()
{
    if (m_context == nullptr)
    {
        return;
    }

    if (m_pipeline != VK_NULL_HANDLE)
    {
        vkDestroyPipeline(m_context->device(), m_pipeline, nullptr);
        m_pipeline = VK_NULL_HANDLE;
    }

    if (m_layout != VK_NULL_HANDLE)
    {
        vkDestroyPipelineLayout(m_context->device(), m_layout, nullptr);
        m_layout = VK_NULL_HANDLE;
    }

    m_context = nullptr;
}

bool VulkanPipeline::isValid() const
{
    return m_pipeline != VK_NULL_HANDLE;
}

VkPipeline VulkanPipeline::handle() const noexcept
{
    return m_pipeline;
}

VkPipelineLayout VulkanPipeline::layout() const noexcept
{
    return m_layout;
}

}

#endif
