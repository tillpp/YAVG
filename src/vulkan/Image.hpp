#pragma once
#include <stb_image.h>
#include "Device.hpp"
#include "VertexBuffer.hpp"
#include "CommandBuffer.hpp"

//TODO learn more about ktx
//TODO: try to reread https://docs.vulkan.org/tutorial/latest/06_Texture_mapping/00_Images.html#_layout_transitions , cause i didnt understand everything.
class Image
{
    vk::raii::DeviceMemory imageMemory = nullptr;
    
public:
    vk::raii::Image image = nullptr;
    vk::raii::ImageView imageView = nullptr;
    vk::raii::Sampler textureSampler = nullptr;
    void create(CommandPool& pool){
        //load image
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load("assets/texture.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        vk::DeviceSize imageSize = texWidth * texHeight * 4;

        if (!pixels) {
            throw std::runtime_error("failed to load texture image!");
        }

        // load into stating Buffer
        Buffer stagingBuffer;
        stagingBuffer.createBuffer(pool.device,imageSize,vk::BufferUsageFlagBits::eTransferSrc,vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

        void* data = stagingBuffer.bufferMemory.mapMemory(0, imageSize);
        memcpy(data, pixels, imageSize);
        stagingBuffer.bufferMemory.unmapMemory();

        stbi_image_free(pixels);

        createImage(pool,texWidth, texHeight, vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal);
        
        transitionImageLayout(pool, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
        copyBufferToImage(pool,stagingBuffer.buffer, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
        transitionImageLayout(pool, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

        createTextureImageView(pool.device);
        createTextureSampler(  pool.device);
    }
    void createImage(CommandPool& pool,uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties) {
        vk::ImageCreateInfo imageInfo{ .imageType = vk::ImageType::e2D, .format = format,
            .extent = {width, height, 1}, .mipLevels = 1, .arrayLayers = 1,
            .samples = vk::SampleCountFlagBits::e1, .tiling = tiling,
            .usage = usage, .sharingMode = vk::SharingMode::eExclusive };

        image = vk::raii::Image(pool.device.device, imageInfo);

        vk::MemoryRequirements memRequirements = image.getMemoryRequirements();
        vk::MemoryAllocateInfo allocInfo{ .allocationSize = memRequirements.size,
                                            .memoryTypeIndex = Buffer::findMemoryType(pool.device,memRequirements.memoryTypeBits, properties) };
        imageMemory = vk::raii::DeviceMemory(pool.device.device, allocInfo);
        image.bindMemory(*imageMemory, 0);

        CommandBuffer commandBuffer(pool); 
        commandBuffer.beginSingleTimeCommands();
        commandBuffer.endSingleTimeCommands(pool);
    }


    //TODO: store the current layout to be used for "oldLayout"
    void transitionImageLayout(CommandPool& pool,vk::ImageLayout oldLayout, vk::ImageLayout newLayout) {
        CommandBuffer commandBuffer(pool); 
        commandBuffer.beginSingleTimeCommands();
        vk::ImageMemoryBarrier barrier{ 
            .oldLayout = oldLayout,
            .newLayout = newLayout,
            .image = image,
            .subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 } 
        };

        vk::PipelineStageFlags sourceStage;
        vk::PipelineStageFlags destinationStage;

        if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
            barrier.srcAccessMask = {};
            barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

            sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
            destinationStage = vk::PipelineStageFlagBits::eTransfer;
        } else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
            barrier.srcAccessMask =  vk::AccessFlagBits::eTransferWrite;
            barrier.dstAccessMask =  vk::AccessFlagBits::eShaderRead;

            sourceStage = vk::PipelineStageFlagBits::eTransfer;
            destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
        } else {
            throw std::invalid_argument("unsupported layout transition!");
        }


        commandBuffer.commandBuffer.pipelineBarrier(sourceStage, destinationStage, {}, {}, nullptr, barrier);
        commandBuffer.endSingleTimeCommands(pool);
    }
    void copyBufferToImage(CommandPool& pool,const vk::raii::Buffer& buffer, uint32_t width, uint32_t height) {
        
        CommandBuffer commandBuffer(pool);
        commandBuffer.beginSingleTimeCommands();
        vk::BufferImageCopy region{ 
            .bufferOffset = 0,
            .bufferRowLength = 0,
            .bufferImageHeight = 0,
            .imageSubresource = { vk::ImageAspectFlagBits::eColor, 0, 0, 1 }, 
            .imageOffset = {0, 0, 0}, 
            .imageExtent = {width, height, 1} 
        };
        commandBuffer.commandBuffer.copyBufferToImage(buffer,image,vk::ImageLayout::eTransferDstOptimal,{region});
        commandBuffer.endSingleTimeCommands(pool);
    }

    void createTextureImageView(Device& device) {
        imageView = createImageView(device, vk::Format::eR8G8B8A8Srgb,vk::ImageAspectFlagBits::eColor);

    }
    vk::raii::ImageView createImageView(Device& device, vk::Format format, vk::ImageAspectFlags aspectFlags) {
        vk::ImageViewCreateInfo viewInfo{ .image = image, .viewType = vk::ImageViewType::e2D,
            .format = format, .subresourceRange = { aspectFlags, 0, 1, 0, 1 } };
        return vk::raii::ImageView( device.device, viewInfo );
    }

    void createTextureSampler(Device& device) {
        vk::PhysicalDeviceProperties properties = device.physicalDevice.getProperties();
        vk::SamplerCreateInfo samplerInfo{
            .magFilter = vk::Filter::eLinear, 
            .minFilter = vk::Filter::eLinear,  
            .mipmapMode = vk::SamplerMipmapMode::eLinear,
            .addressModeU = vk::SamplerAddressMode::eRepeat,
            .addressModeV = vk::SamplerAddressMode::eRepeat,
            .addressModeW = vk::SamplerAddressMode::eRepeat,
            .anisotropyEnable = vk::True, // TODO: deactive when .samplerAnisotropy isnt available and make maxAnisotropy = 1.0f
            .maxAnisotropy = properties.limits.maxSamplerAnisotropy, 
            .compareEnable = vk::False,
            .compareOp = vk::CompareOp::eAlways
        };
        textureSampler = vk::raii::Sampler(device.device, samplerInfo);
    }
};

