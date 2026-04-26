#include "DescriptorLayout.hpp"

vk::DescriptorSetLayoutBinding DescriptorLayout::getBinding()const{
    return vk::DescriptorSetLayoutBinding{
        .binding = binding,
        .descriptorType = descriptorType,
        .descriptorCount = 1,
        .stageFlags = stageFlags,
        .pImmutableSamplers = nullptr,
    };
}
DescriptorLayout::DescriptorLayout(
    uint32_t             binding,
    vk::ShaderStageFlags stageFlags,
    vk::DescriptorType   descriptorType
){
    this->binding = binding;
    this->stageFlags = stageFlags;
    this->descriptorType = descriptorType;
}
