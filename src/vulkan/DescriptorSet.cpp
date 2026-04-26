#include "DescriptorSet.hpp"
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