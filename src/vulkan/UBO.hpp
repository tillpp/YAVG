#pragma once
#include "Device.hpp"
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>


struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

//TODO: learn more about "push constants" as an alternative to this:
class UBO
{
public:
    vk::raii::DescriptorSetLayout descriptorSetLayout = nullptr;

    std::vector<Buffer> uniformBuffers;
    std::vector<void*> uniformBuffersMapped;

    //TODO: pipeline description into a different class.
    void create(Device& device,const size_t MAX_FRAMES_IN_FLIGHT){
        // for pipeline
        vk::DescriptorSetLayoutBinding uboLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex, nullptr);
        vk::DescriptorSetLayoutCreateInfo layoutInfo{.bindingCount = 1, .pBindings = &uboLayoutBinding};
        descriptorSetLayout = vk::raii::DescriptorSetLayout(device.device, layoutInfo);



        // for memory
        uniformBuffers.clear();
        uniformBuffersMapped.clear();

        
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vk::DeviceSize bufferSize = sizeof(UniformBufferObject);
            Buffer buffer;
            buffer.createBuffer(device,bufferSize,vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
            uniformBuffers.emplace_back(std::move(buffer));
            uniformBuffersMapped.emplace_back( uniformBuffers[i].bufferMemory.mapMemory(0, bufferSize));
        }

    }

    //static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height)
    void updateUniformBuffer(uint32_t currentImage, float aspectRatio, bool zoom) {
        static auto startTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        UniformBufferObject ubo{};
        ubo.model = rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.view = lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        if(!zoom){
            ubo.proj = glm::perspective(glm::radians(90.0f), aspectRatio, 0.1f, 10.0f);
        }else{
            ubo.proj = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 10.0f);
        }
        
        // cause glm wasnt designed for Vulkan: (invert y aches)
        ubo.proj[1][1] *= -1;

        memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
    }

    vk::raii::DescriptorPool descriptorPool = nullptr;
    void createDescriptorPool(Device& device,const size_t MAX_FRAMES_IN_FLIGHT){
        vk::DescriptorPoolSize poolSize(vk::DescriptorType::eUniformBuffer, MAX_FRAMES_IN_FLIGHT);
        vk::DescriptorPoolCreateInfo poolInfo{ .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, .maxSets = MAX_FRAMES_IN_FLIGHT, .poolSizeCount = 1, .pPoolSizes = &poolSize };
        descriptorPool = vk::raii::DescriptorPool(device.device, poolInfo);
    }
    std::vector<vk::raii::DescriptorSet> descriptorSets;
    void createDescriptorSets(Device& device,const size_t MAX_FRAMES_IN_FLIGHT) {
        std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, *descriptorSetLayout);
        vk::DescriptorSetAllocateInfo allocInfo{ .descriptorPool = descriptorPool, .descriptorSetCount = static_cast<uint32_t>(layouts.size()), .pSetLayouts = layouts.data() };

        descriptorSets.clear();
        descriptorSets = device.device.allocateDescriptorSets(allocInfo);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vk::DescriptorBufferInfo bufferInfo{ .buffer = uniformBuffers[i].buffer, .offset = 0, .range = sizeof(UniformBufferObject) };
            vk::WriteDescriptorSet descriptorWrite{ .dstSet = descriptorSets[i], .dstBinding = 0, .dstArrayElement = 0, .descriptorCount = 1, .descriptorType = vk::DescriptorType::eUniformBuffer, .pBufferInfo = &bufferInfo };
            device.device.updateDescriptorSets(descriptorWrite, {});
        }
    }
};

