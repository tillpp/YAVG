#include "Descriptor.hpp"
#include "vulkan_old/UBO.hpp"

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

#include "vulkan/Pipeline.hpp"

void DescriptorSet::create(Device& device,RenderSync& render,DescriptorSetLayout& dsl,std::vector<Descriptor> dsArray){
    //pool creation
    {
        std::map<vk::DescriptorType,uint32_t> poolsizes;
        for(auto&ds:dsArray){
            if(poolsizes.find(ds.descriptorType) == poolsizes.end())
                poolsizes[ds.descriptorType] = 0;
            poolsizes[ds.descriptorType]++;
        }

        std::vector<vk::DescriptorPoolSize> poolSize;
        for(auto& pair:poolsizes){
            poolSize.push_back(
                vk::DescriptorPoolSize{
                    .type = pair.first,
                    .descriptorCount = render.MAX_FRAMES_IN_FLIGHT*pair.second,
                }
            );
        }
        vk::DescriptorPoolCreateInfo poolInfo{ 
            .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
            .maxSets = render.MAX_FRAMES_IN_FLIGHT,
            .poolSizeCount = (uint32_t)poolSize.size(),
            .pPoolSizes = poolSize.data(),
        };
        descriptorPool = vk::raii::DescriptorPool(device.device, poolInfo);
    }
    {
        std::vector<vk::DescriptorSetLayout> layouts(render.MAX_FRAMES_IN_FLIGHT, *dsl.descriptorSetLayout);
        vk::DescriptorSetAllocateInfo allocInfo{ 
            .descriptorPool = descriptorPool, 
            .descriptorSetCount = static_cast<uint32_t>(layouts.size()), 
            .pSetLayouts = layouts.data() 
        };

        descriptorSets.clear();
        descriptorSets = device.device.allocateDescriptorSets(allocInfo);

        for (size_t i = 0; i < render.MAX_FRAMES_IN_FLIGHT; i++) {
            std::vector<vk::WriteDescriptorSet> descriptorWrites;
            for(auto& ds:dsArray){
                vk::WriteDescriptorSet wds{ 
                    .dstSet = descriptorSets[i], 
                    .dstBinding = ds.binding, 
                    .dstArrayElement = 0, 
                    .descriptorCount = 1,
                    .descriptorType = ds.descriptorType
                };
                ds.writeDescriptorSet(wds,i);
                descriptorWrites.push_back(wds);
            }
            device.device.updateDescriptorSets(descriptorWrites, {});
        }
    }
}
void DescriptorSet::use(vk::raii::CommandBuffer& commandBuffer,RenderSync& render, Pipeline& pipeline,uint32_t firstSet ){
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline.pipelineLayout, firstSet, *descriptorSets[render.getFrameIndex()], nullptr);
}