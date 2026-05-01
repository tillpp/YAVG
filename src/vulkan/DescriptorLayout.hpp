#pragma once
#include "vulkan/setup/Device.hpp"


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

class DescriptorSetLayout
{
public:
    vk::raii::DescriptorSetLayout descriptorSetLayout = nullptr;
    void create(Device& device,std::vector<DescriptorLayout> dsArray);
};
