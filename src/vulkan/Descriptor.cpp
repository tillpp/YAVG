#include "Descriptor.hpp"
#include "vulkan_old/UBO.hpp"

Descriptor::Descriptor(
    uint32_t             binding,
    vk::ShaderStageFlags stageFlags,
    UBO&                 ubo
):DescriptorLayout(binding,stageFlags,vk::DescriptorType::eUniformBuffer),ubo(&ubo){
}
Descriptor::Descriptor(
    uint32_t             binding,
    vk::ShaderStageFlags stageFlags,
    Image&               image
):DescriptorLayout(binding,stageFlags,vk::DescriptorType::eCombinedImageSampler),image(&image){
}

void Descriptor::writeDescriptorSet(vk::WriteDescriptorSet& wds,size_t frameInFlight){
    if(descriptorType == vk::DescriptorType::eUniformBuffer){
        bufferInfo = { 
            .buffer = ubo->uniformBuffers[frameInFlight].buffer, 
            .offset = 0, 
            .range   = ubo->size, 
        };
        wds.pBufferInfo = &bufferInfo;
    }
    if(descriptorType == vk::DescriptorType::eCombinedImageSampler){
        imageInfo = { 
            .sampler     = image->textureSampler, 
            .imageView   = image->imageView, 
            .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
        };
        wds.pImageInfo = &imageInfo;
    }
}
