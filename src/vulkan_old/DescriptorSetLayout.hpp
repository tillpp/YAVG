#pragma once
#include "vulkan/Header.hpp"

class DescriptorSetLayout
{
public:
    void create(Device& device,std::vector<vk::DescriptorSetLayoutBinding> bindings,uint32_t MAX_FRAMES_IN_FLIGHT, UBO& ubo, Image& image){
        vk::DescriptorSetLayoutCreateInfo layoutInfo{
            .bindingCount = (uint32_t)bindings.size(), 
            .pBindings = bindings.data()
        };
        descriptorSetLayout = vk::raii::DescriptorSetLayout(device.device, layoutInfo);

        {
            std::array poolSize {
                vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, MAX_FRAMES_IN_FLIGHT),
                vk::DescriptorPoolSize(  vk::DescriptorType::eCombinedImageSampler, MAX_FRAMES_IN_FLIGHT)
            };
            vk::DescriptorPoolCreateInfo poolInfo{ 
                .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
                .maxSets = MAX_FRAMES_IN_FLIGHT,
                .poolSizeCount = poolSize.size(),
                .pPoolSizes = poolSize.data()
            };
            descriptorPool = vk::raii::DescriptorPool(device.device, poolInfo);
        }
        {
            std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, *descriptorSetLayout);
            vk::DescriptorSetAllocateInfo allocInfo{ .descriptorPool = descriptorPool, .descriptorSetCount = static_cast<uint32_t>(layouts.size()), .pSetLayouts = layouts.data() };

            descriptorSets.clear();
            descriptorSets = device.device.allocateDescriptorSets(allocInfo);

            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                vk::DescriptorBufferInfo bufferInfo{ .buffer = ubo.uniformBuffers[i].buffer, .offset = 0, .range = sizeof(UniformBufferObject) };
                vk::DescriptorImageInfo imageInfo{ .sampler = image.textureSampler, .imageView = image.imageView, .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal};
                
                std::array descriptorWrites{
                    vk::WriteDescriptorSet{ .dstSet = descriptorSets[i], .dstBinding = 0, .dstArrayElement = 0, .descriptorCount = 1,
                        .descriptorType = vk::DescriptorType::eUniformBuffer, .pBufferInfo = &bufferInfo },
                    vk::WriteDescriptorSet{ .dstSet = descriptorSets[i], .dstBinding = 1, .dstArrayElement = 0, .descriptorCount = 1,
                        .descriptorType = vk::DescriptorType::eCombinedImageSampler, .pImageInfo = &imageInfo }
                };
                device.device.updateDescriptorSets(descriptorWrites, {});
            }
        }
    }

    vk::raii::DescriptorSetLayout descriptorSetLayout = nullptr;
    vk::raii::DescriptorPool descriptorPool = nullptr;
    std::vector<vk::raii::DescriptorSet> descriptorSets;
    
};
