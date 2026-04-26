#pragma once
#include "vulkan/setup/Device.hpp"
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>


struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

class UBO
{
public:
    size_t size = sizeof(UniformBufferObject);
    std::vector<Buffer> uniformBuffers;
    std::vector<void*> uniformBuffersMapped;

    //TODO: pipeline description into a different class.
    void create(Device& device,const size_t MAX_FRAMES_IN_FLIGHT){

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
    void updateUniformBuffer(uint32_t currentFrame, float aspectRatio, bool zoom,glm::vec3 cameraPos,glm::vec3 cameraForward) {
        static auto startTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        UniformBufferObject ubo{};
        // ubo.model = rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.model = rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.view = lookAt(cameraPos, cameraPos+cameraForward, glm::vec3(0.0f, 1.0f, 0.0f));
        if(!zoom){
            ubo.proj = glm::perspective(glm::radians(90.0f), aspectRatio, 0.1f, 1000.0f);
        }else{
            ubo.proj = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 1000.0f);
        }
        
        // cause glm wasnt designed for Vulkan: (invert y aches)
        ubo.proj[1][1] *= -1;

        memcpy(uniformBuffersMapped[currentFrame], &ubo, sizeof(ubo));
    }
};

