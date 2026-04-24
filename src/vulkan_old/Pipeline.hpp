#pragma once
#include <fstream>
#include <filesystem>
#include "vulkan/Swapchain.hpp"
#include "VertexBuffer.hpp"
#include "DepthBuffer.hpp"
#include "UBO.hpp"

class Pipeline
{
    [[nodiscard]] vk::raii::ShaderModule createShaderModule(
        Device& device,
        const std::vector<char>& code) const;
public:
    vk::raii::PipelineLayout pipelineLayout = nullptr;
    vk::raii::Pipeline graphicsPipeline = nullptr;

       
    void create(
        Device& device,
        std::filesystem::path shaderFile, 
        std::string entryFnVertex, 
        std::string entryFnFragment, 
        Swapchain& swapChain,
        class DescriptorSetLayout& dsLayout,
        DepthBuffer& depthBuffer);
};
