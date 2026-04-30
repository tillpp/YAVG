#pragma once
#include "vulkan_old/Image2.hpp"
#include "vulkan/setup/Swapchain.hpp"


//TODO: look into stencil tests
class DepthBuffer
{
    static vk::Format findSupportedFormat(Device& device,const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);
    static bool hasStencilComponent(vk::Format format);
    static vk::Format findDepthFormat(Device& device);
public:
    
    Image image;
    vk::Format depthFormat;

    void create(CommandPool& pool,Swapchain& swapchain);
};
