#pragma once
#include <fstream>
#include "vulkan/Swapchain.hpp"
#include "VertexBuffer.hpp"
#include "DepthBuffer.hpp"
#include "UBO.hpp"

class Pipeline
{
    std::vector<char> readFile(const std::string& filename);
    [[nodiscard]] vk::raii::ShaderModule createShaderModule(
        Device& device,
        const std::vector<char>& code) const;
public:
    vk::raii::PipelineLayout pipelineLayout = nullptr;
    vk::raii::Pipeline graphicsPipeline = nullptr;

       
    void create(
        Device& device,
        std::string shaderFile, 
        std::string entryFnVertex, 
        std::string entryFnFragment, 
        Swapchain& swapChain,
        vk::raii::DescriptorSetLayout& descriptorSetLayout,
        DepthBuffer& depthBuffer);
};
