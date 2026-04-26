#include "DescriptorSetLayout.hpp"



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
