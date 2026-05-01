#pragma once
#include "vulkan/Descriptor.hpp"
#include "vulkan/Header.hpp"
#include "vulkan/setup/CommandPool.hpp"
#include "vulkan_old/Image2.hpp"
#include <filesystem>
#include <memory>
#include <vulkan/vulkan.hpp>

class Image:public Resource{
    // when the image gets resized, it "reincarnates" (wink). 
    // DS can still point at old Reincarnations before their are updated.
    struct Reincarnation:public ResourceReincarnation{
        vk::raii::DeviceMemory imageMemory    = nullptr;
        vk::raii::Image        image          = nullptr;
        vk::raii::ImageView    imageView      = nullptr;
        vk::raii::Sampler      textureSampler = nullptr;

        void initImage(Device& device,uint32_t width, uint32_t height, 
            vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, 
            vk::MemoryPropertyFlags properties) {

            vk::ImageCreateInfo imageInfo{ 
                .imageType = vk::ImageType::e2D, 
                .format = format,
                .extent = {width, height, 1}, 
                .mipLevels = 1, .arrayLayers = 1,
                .samples = vk::SampleCountFlagBits::e1,
                .tiling = tiling,
                .usage = usage,
                .sharingMode = vk::SharingMode::eExclusive 
            };
                
            image = vk::raii::Image(device.device, imageInfo);
                
            vk::MemoryRequirements memRequirements = image.getMemoryRequirements();
            vk::MemoryAllocateInfo allocInfo{ 
                .allocationSize = memRequirements.size,
                .memoryTypeIndex = Buffer::findMemoryType(device,memRequirements.memoryTypeBits, properties) 
            };
            imageMemory = vk::raii::DeviceMemory(device.device, allocInfo);
            image.bindMemory(*imageMemory, 0);   
        }
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
        void createTextureImageView(Device& device, vk::Format format = vk::Format::eR8G8B8A8Srgb) {
            imageView = createImageView(device, format,vk::ImageAspectFlagBits::eColor);
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

        virtual DescriptorInfo getDescriptorInfo()const{
            DescriptorInfo di;
            di.type = DescriptorInfo::IMAGE;
            di.imageInfo = { 
                .sampler     = textureSampler, 
                .imageView   = imageView, 
                .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
            };
            return di;
        }

    };
    std::shared_ptr<Reincarnation> current;
public:

    Image(){}
    ~Image(){}

    void create(CommandPool& pool,std::filesystem::path path){        
        //load image
        int texWidth = 0, texHeight = 0, texChannels = 0;
        auto pathAsString = path.string();
        stbi_uc* pixels = stbi_load(pathAsString.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

        if (!pixels) {
            throw std::runtime_error("failed to load texture image!");
        }
        create(pool,texWidth, texHeight,pixels);
        stbi_image_free(pixels);
    }
    void create(CommandPool& pool,int texWidth, int texHeight,stbi_uc* pixels){
        auto& device = pool.getDevice();
        vk::DeviceSize imageSize = texWidth * texHeight * 4;
        // load into stating Buffer
        Buffer stagingBuffer;
        {
            stagingBuffer.createBuffer(
                device,
                imageSize,
                vk::BufferUsageFlagBits::eTransferSrc,
                vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
            );
            
            void* data = stagingBuffer.bufferMemory.mapMemory(
                0, imageSize);
            memcpy(data, pixels, imageSize);
            stagingBuffer.bufferMemory.unmapMemory();
        }
        current = std::make_shared<Reincarnation>();
        current->initImage(device, texWidth, texHeight, vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal);
        current->transitionImageLayout(pool, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
        current->copyBufferToImage(pool,stagingBuffer.buffer, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
        current->transitionImageLayout(pool, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

        current->createTextureImageView(device);
        current->createTextureSampler(  device);
    }

    virtual std::shared_ptr<ResourceReincarnation> getResource(size_t frameIndex)const{
        return current;
    }
private:
};