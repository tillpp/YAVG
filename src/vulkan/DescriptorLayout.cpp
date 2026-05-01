#include "vulkan/DescriptorLayout.hpp"

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
void DescriptorSetLayout::create(Device& device,std::vector<DescriptorLayout> dsArray){
    {
        std::vector<vk::DescriptorSetLayoutBinding> bindings;
        for(auto& ds:dsArray){
            bindings.push_back(ds.getBinding());
        }
        vk::DescriptorSetLayoutCreateInfo layoutInfo{
            .bindingCount = (uint32_t)bindings.size(), 
            .pBindings = bindings.data(),
        };
        descriptorSetLayout = vk::raii::DescriptorSetLayout(device.device, layoutInfo);
    }        
}
