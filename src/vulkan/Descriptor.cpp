#include "Descriptor.hpp"
#include "vulkan_old/UBO.hpp"




#include "vulkan/Pipeline.hpp"
#include <cassert>
#include <vector>

void DescriptorSet::create(Device& device,RenderSync& render,DescriptorSetLayout& dsl,std::vector<DescriptorLayout> dsArray){
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
    }
    for (auto& dsLayout : dsArray) {
        mappingID2Index[dsLayout.binding] = bindings.size();
        bindings.emplace_back(render,dsLayout);
    }
}
void DescriptorSet::bind(Device& device,vk::raii::CommandBuffer& commandBuffer,RenderSync& render, Pipeline& pipeline,uint32_t firstSet ){
    //Updating reincarnations:

    auto fi = render.getFrameIndex();

    std::vector<vk::WriteDescriptorSet> descriptorWrites;
    DescriptorInfo descriptorInfos[bindings.size()];

    for(int bindIndex = 0;bindIndex < bindings.size(); bindIndex++){
        auto& bind          = bindings[bindIndex];
        auto& reincarnation = bind.frames[fi].reincarnation;
        auto& resource      = bind.resource;
        
        assert(resource);

        // missmatch?
        if(reincarnation != resource->getResource(fi)){
            reincarnation = resource->getResource(fi);
            vk::WriteDescriptorSet wds{ 
                .dstSet = descriptorSets[fi], 
                .dstBinding = bind.binding, 
                .dstArrayElement = 0, 
                .descriptorCount = 1,
                .descriptorType = bind.descriptorType
            };
            auto& descriptorInfo =descriptorInfos[bindIndex];
            descriptorInfo = reincarnation->getDescriptorInfo();

            if(descriptorInfo.type == DescriptorInfo::BUFFER){
                wds.pBufferInfo = &descriptorInfo.bufferInfo;
            }else if(descriptorInfo.type == DescriptorInfo::IMAGE){
                wds.pImageInfo = &descriptorInfo.imageInfo;
            }
            descriptorWrites.push_back(wds);
        }
    }
    if(descriptorWrites.size())
        device.device.updateDescriptorSets(descriptorWrites, {});

    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline.pipelineLayout, firstSet, *descriptorSets[fi], nullptr);
}
void DescriptorSet::setResource(size_t binding,std::shared_ptr<Resource> resource){
    bindings[mappingID2Index[binding]].resource = resource;
}
