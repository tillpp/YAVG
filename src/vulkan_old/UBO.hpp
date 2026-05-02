#pragma once
#include "vulkan/Descriptor.hpp"
#include "vulkan/setup/Device.hpp"
#include "vulkan/setup/RenderSync.hpp"
#include "vulkan_old/Buffer.hpp"
#include <algorithm>
#include <cstddef>
#include <memory>
#include <vulkan/vulkan.hpp>
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>




class UBO:public Resource
{
    struct Reincarnation:public ResourceReincarnation{
        Buffer buffer;
        void* buffersMapped;
        size_t size;
        virtual DescriptorInfo getDescriptorInfo()const{
            DescriptorInfo di;
            di.type = DescriptorInfo::BUFFER;
            di.bufferInfo = { 
                .buffer = buffer.buffer, 
                .offset = 0, 
                .range   = size, 
            };
            return di;     
        }
    };
public:
    size_t size;
    std::vector<std::shared_ptr<Reincarnation>> frames;

    void create(RenderSync* render,Device& device,vk::DeviceSize bufferSize){
        this->size = bufferSize;
        // for memory
        frames.clear();

        for (size_t i = 0; i < render->MAX_FRAMES_IN_FLIGHT; i++) {
            
            auto reincarnation = std::make_shared<Reincarnation>();
            frames.push_back(reincarnation);
            reincarnation->buffer.createBuffer(render,device,bufferSize,vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
            reincarnation->buffersMapped = std::move(reincarnation->buffer.bufferMemory.mapMemory(0, bufferSize));
            reincarnation->size          = bufferSize;
        }
    }

    virtual std::shared_ptr<ResourceReincarnation> getResource(size_t frameIndex)const{
        return frames[frameIndex];
    }

};

