#include "DepthBuffer.hpp"

vk::Format DepthBuffer::findSupportedFormat(Device& device,const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features) {
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
bool DepthBuffer::hasStencilComponent(vk::Format format) {
    return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
}
vk::Format DepthBuffer::findDepthFormat(Device& device) {
    return findSupportedFormat(
            device,
            {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
            vk::ImageTiling::eOptimal,
            vk::FormatFeatureFlagBits::eDepthStencilAttachment
        );
}
void DepthBuffer::create(CommandPool& pool,Swapchain& swapchain){
    depthFormat = findDepthFormat(pool.getDevice());
    image.createImage(pool,
        swapchain.swapChainExtent.width, 
        swapchain.swapChainExtent.height, 
        depthFormat, 
        vk::ImageTiling::eOptimal, 
        vk::ImageUsageFlagBits::eDepthStencilAttachment, 
        vk::MemoryPropertyFlagBits::eDeviceLocal
    );
    image.imageView = image.createImageView(pool.getDevice(), 
        depthFormat, 
        vk::ImageAspectFlagBits::eDepth
    );
}