#pragma once
#include "vulkan/Header.hpp"
#include "vulkan/RenderSync.hpp"

class DescriptionSet{
public:
    uint32_t             binding;
    vk::ShaderStageFlags stageFlags;
    
    vk::DescriptorType   descriptorType;
    UBO*   ubo   = nullptr;
    Image* image = nullptr;

    DescriptionSet(
        uint32_t             binding,
        vk::ShaderStageFlags stageFlags,
        UBO&                 ubo
    ):binding(binding),stageFlags(stageFlags),descriptorType(vk::DescriptorType::eUniformBuffer),ubo(&ubo){
        
    }
    DescriptionSet(
        uint32_t             binding,
        vk::ShaderStageFlags stageFlags,
        Image&               image
    ):binding(binding),stageFlags(stageFlags),descriptorType(vk::DescriptorType::eCombinedImageSampler),image(&image){
        
    }
    
    vk::DescriptorBufferInfo bufferInfo;
    vk::DescriptorImageInfo imageInfo;
    void writeDescriptorSet(vk::WriteDescriptorSet& wds,size_t frameInFlight){
        if(descriptorType == vk::DescriptorType::eUniformBuffer){
            bufferInfo = { 
                .buffer = ubo->uniformBuffers[frameInFlight].buffer, 
                .offset = 0, 
                .range   = sizeof(UniformBufferObject) 
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

    vk::DescriptorSetLayoutBinding getBinding()const{
        return vk::DescriptorSetLayoutBinding{
            .binding = binding,
            .descriptorType = descriptorType,
            .descriptorCount = 1,
            .stageFlags = stageFlags,
            .pImmutableSamplers = nullptr,
        };
    }

};
class DescriptorSetLayout
{
    vk::raii::DescriptorPool descriptorPool = nullptr;
public:
    vk::raii::DescriptorSetLayout descriptorSetLayout = nullptr;
    std::vector<vk::raii::DescriptorSet> descriptorSets;

    void create(Device& device,RenderSync& render,std::vector<DescriptionSet> dsArray){
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
            std::vector<vk::DescriptorSetLayout> layouts(render.MAX_FRAMES_IN_FLIGHT, *descriptorSetLayout);
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
    
    void use(vk::raii::CommandBuffer& commandBuffer,RenderSync& render, Pipeline& pipeline,uint32_t firstSet = 0){
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline.pipelineLayout, firstSet, *descriptorSets[render.getFrameIndex()], nullptr);
    }
};
