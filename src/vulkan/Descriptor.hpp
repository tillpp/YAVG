#pragma once
#include "vulkan/Header.hpp"
#include "vulkan/RenderSync.hpp"
#include "DescriptorLayout.hpp"

class Descriptor:public DescriptorLayout{
public:
    class UBO*   ubo   = nullptr;
    class Image* image = nullptr;

    Descriptor(
        uint32_t             binding,
        vk::ShaderStageFlags stageFlags,
        class UBO&           ubo
    );
    Descriptor(
        uint32_t             binding,
        vk::ShaderStageFlags stageFlags,
        class Image&         image
    );
    
    vk::DescriptorBufferInfo bufferInfo;
    vk::DescriptorImageInfo imageInfo;
    void writeDescriptorSet(vk::WriteDescriptorSet& wds,size_t frameInFlight);   

};