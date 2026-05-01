#include "Pipeline.hpp"
#include "vulkan/DescriptorLayout.hpp"
#include "vulkan/Descriptor.hpp"
#include "Data/Filebasic.hpp"
#include "vulkan/PushContant.hpp"

[[nodiscard]] vk::raii::ShaderModule Pipeline::createShaderModule(
    Device& device,
    const std::vector<char>& code) const{

    vk::ShaderModuleCreateInfo createInfo{ .codeSize = code.size() * sizeof(char), .pCode = reinterpret_cast<const uint32_t*>(code.data()) };
    vk::raii::ShaderModule shaderModule{ device.device, createInfo };
    return shaderModule;
}
void Pipeline::create(
    Device& device,
    std::filesystem::path shaderFile, 
    std::string entryFnVertex, 
    std::string entryFnFragment, 
    Swapchain& swapChain,
    DescriptorSetLayout& dsLayout,
    DepthBuffer& depthBuffer,bool depthTesting,
    std::optional<PushConstant*> pushConstant
) {

    // shader
    vk::raii::ShaderModule shaderModule = createShaderModule(device,readFileOrThrow(shaderFile));
    vk::PipelineShaderStageCreateInfo vertShaderStageInfo{ 
        .stage = vk::ShaderStageFlagBits::eVertex,
        .module = shaderModule,
        .pName = entryFnVertex.c_str(),
    };
    vk::PipelineShaderStageCreateInfo fragShaderStageInfo{ 
        .stage = vk::ShaderStageFlagBits::eFragment,
        .module = shaderModule,
        .pName = entryFnFragment.c_str(),
    };
    vk::PipelineShaderStageCreateInfo shaderStages[] = {
        vertShaderStageInfo,
        fragShaderStageInfo
    };
    
    //dynamic states
    std::vector<vk::DynamicState> dynamicStates = {
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor
    };
    vk::PipelineDynamicStateCreateInfo dynamicState{
        .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
        .pDynamicStates = dynamicStates.data()
    };
    
    // basicly VAO        
    auto bindingDescription    = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();
    vk::PipelineVertexInputStateCreateInfo vertexInputInfo{
        .vertexBindingDescriptionCount   = 1,
        .pVertexBindingDescriptions      = &bindingDescription,
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
        .pVertexAttributeDescriptions    = attributeDescriptions.data()};

    // assembly
    vk::PipelineInputAssemblyStateCreateInfo inputAssembly{
        .topology = vk::PrimitiveTopology::eTriangleList
    };
    
    // viewport & scissors
    // vk::Viewport viewport{0.0f, 0.0f, static_cast<float>(swapChainExtent.width), static_cast<float>(swapChainExtent.height), 0.0f, 1.0f};
    // vk::Rect2D scissor{vk::Offset2D{ 0, 0 }, swapChainExtent};
    vk::PipelineViewportStateCreateInfo viewportState{
        .viewportCount = 1,
        .scissorCount = 1
    };

    vk::PipelineRasterizationStateCreateInfo rasterizer{
        .depthClampEnable        = vk::False,
        .rasterizerDiscardEnable = vk::False,
        .polygonMode             = vk::PolygonMode::eFill,
        .cullMode                = vk::CullModeFlagBits::eBack,
        .frontFace               = vk::FrontFace::eCounterClockwise,
        .depthBiasEnable         = vk::False,
        .lineWidth               = 1.0f
    };

    vk::PipelineMultisampleStateCreateInfo multisampling{
        .rasterizationSamples = vk::SampleCountFlagBits::e1,
        .sampleShadingEnable = vk::False
    };

    // alpha blending
    vk::PipelineColorBlendAttachmentState colorBlendAttachment{
        .blendEnable         = vk::True,
        .srcColorBlendFactor = vk::BlendFactor::eSrcAlpha,
        .dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
        .colorBlendOp        = vk::BlendOp::eAdd,
        .srcAlphaBlendFactor = vk::BlendFactor::eOne,
        .dstAlphaBlendFactor = vk::BlendFactor::eZero,
        .alphaBlendOp        = vk::BlendOp::eAdd,
        .colorWriteMask      = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
    };

    vk::PipelineColorBlendStateCreateInfo colorBlending{
        .logicOpEnable = vk::False, 
        .logicOp = vk::LogicOp::eCopy, 
        .attachmentCount = 1, 
        .pAttachments = &colorBlendAttachment
    };
    
    // PipelineLayout (for uniforms later)
    {
        vk::PipelineLayoutCreateInfo pipelineLayoutInfo{ 
            .setLayoutCount = 1, 
            .pSetLayouts = &*dsLayout.descriptorSetLayout, 
            .pushConstantRangeCount = 0 
        };
        if(pushConstant.has_value()){
            pipelineLayoutInfo.pushConstantRangeCount = 1;
            pipelineLayoutInfo.pPushConstantRanges = &pushConstant.value()->pushConstantRange;
        }
        pipelineLayout = vk::raii::PipelineLayout(device.device, pipelineLayoutInfo);
    }
    

    //enable depthtesting
    vk::PipelineDepthStencilStateCreateInfo depthStencil{
        .depthBoundsTestEnable = vk::False,
        .stencilTestEnable     = vk::False
    };
    if(depthTesting){
        depthStencil.depthTestEnable       = vk::True;
        depthStencil.depthWriteEnable      = vk::True;
        depthStencil.depthCompareOp        = vk::CompareOp::eLess;
    }else{
        depthStencil.depthTestEnable       = vk::False;
        depthStencil.depthWriteEnable      = vk::False;
        depthStencil.depthCompareOp        = vk::CompareOp::eAlways;

    }

    
    // dynamic rendering
    vk::PipelineRenderingCreateInfo pipelineRenderingCreateInfo{ 
        .colorAttachmentCount = 1, 
        .pColorAttachmentFormats = &swapChain.surfaceFormat.format, 
        .depthAttachmentFormat = depthBuffer.depthFormat,
    };

    // everything together
    vk::GraphicsPipelineCreateInfo  graphicsPipelineCreateInfo{
        .stageCount          = 2,
        .pStages             = shaderStages,
        .pVertexInputState   = &vertexInputInfo,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState      = &viewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState   = &multisampling,
        .pDepthStencilState  = &depthStencil,
        .pColorBlendState    = &colorBlending,
        .pDynamicState       = &dynamicState,
        .layout              = pipelineLayout,
        .renderPass          = nullptr // we are using dynamic rendering instead
    };
    graphicsPipelineCreateInfo.pNext = &pipelineRenderingCreateInfo;

    graphicsPipeline = vk::raii::Pipeline(device.device, nullptr, graphicsPipelineCreateInfo);
    // TODO: learn more about pipeline caching.
}