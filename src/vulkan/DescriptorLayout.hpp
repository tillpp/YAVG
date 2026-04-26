#pragma once
#include "vulkan/Header.hpp"
#include "vulkan/setup/RenderSync.hpp"

struct DescriptorLayout{
    uint32_t             binding;
    vk::ShaderStageFlags stageFlags;
    vk::DescriptorType   descriptorType;

    
    DescriptorLayout(
        uint32_t             binding,
        vk::ShaderStageFlags stageFlags,
        vk::DescriptorType   descriptorType
    );
    
    vk::DescriptorSetLayoutBinding getBinding()const;
};