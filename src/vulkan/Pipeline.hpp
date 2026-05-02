#pragma once
#include <fstream>
#include <filesystem>
#include "vulkan/setup/RenderSync.hpp"
#include "vulkan/setup/Swapchain.hpp"
#include "vulkan/DepthBuffer.hpp"
#include "vulkan_old/Buffer.hpp"
#include "vulkan_old/UBO.hpp"

class Pipeline
{
    [[nodiscard]] vk::raii::ShaderModule createShaderModule(
        Device& device,
        const std::vector<char>& code) const;

    RenderSync* render = nullptr;
public:
    vk::raii::PipelineLayout pipelineLayout = nullptr;
    vk::raii::Pipeline graphicsPipeline = nullptr;

       
    void create(
        RenderSync* render,
        Device& device,
        std::filesystem::path shaderFile, 
        std::string entryFnVertex, 
        std::string entryFnFragment, 
        Swapchain& swapChain,
        class DescriptorSetLayout& dsLayout,
        DepthBuffer& depthBuffer, bool depthTesting = true,
        std::optional<class PushConstant*> pushConstant = {});
    ~Pipeline(){
        if(render)
            render->trash(std::move(pipelineLayout),std::move(graphicsPipeline));
    }
};

