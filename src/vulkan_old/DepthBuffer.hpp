#pragma once
#include "Image.hpp"
#include "Swapchain.hpp"


//TODO: look into stencil tests
class DepthBuffer
{
    public:
    vk::Format findSupportedFormat(Device& device,const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features) {
        for (const auto format : candidates) {
            vk::FormatProperties props = device.physicalDevice.getFormatProperties(format);

            if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) {
                return format;
            }
            if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }

        throw std::runtime_error("failed to find supported format!");
    }
    bool hasStencilComponent(vk::Format format) {
        return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
    }
    vk::Format findDepthFormat(Device& device) {
        return findSupportedFormat(
                device,
                {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
                vk::ImageTiling::eOptimal,
                vk::FormatFeatureFlagBits::eDepthStencilAttachment
            );
    }
    Image image;
    vk::Format depthFormat;
    void create(CommandPool& pool,Swapchain& swapchain){
        depthFormat = findDepthFormat(pool.device);
        image.createImage(pool,swapchain.swapChainExtent.width, swapchain.swapChainExtent.height, depthFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal);
        image.imageView = image.createImageView(pool.device, depthFormat, vk::ImageAspectFlagBits::eDepth);
    }
};
