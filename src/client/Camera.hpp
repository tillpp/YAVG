#pragma once
#include "vulkan/Header.hpp"
#include "vulkan/setup/RenderSync.hpp"
#include "vulkan/setup/Window.hpp"
#include "vulkan_old/UBO.hpp"
#include <chrono>
#include <memory>

class Camera
{
    struct UniformBufferObject {
        alignas(16) glm::mat4 model;
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 proj;
    };
public:
    std::shared_ptr<UBO> ubo = std::make_shared<UBO>();

    void create(Device& device,RenderSync& render){
        ubo->create(device, render.MAX_FRAMES_IN_FLIGHT,sizeof(UniformBufferObject));
    }

    glm::vec3 pos = glm::vec3(1.0f, 5.0f, 5.0f);
    glm::vec3 forward = glm::vec3(0.0f, -1.0f, -1.0f);
    glm::vec3 right = glm::vec3(-1.0f, -1.0f, 0.0f);
    
    void update(Window& window, float delta);
    
    void updateUniformBuffer(uint32_t frameIndex, float aspectRatio, bool zoom,glm::vec3 cameraPos,glm::vec3 cameraForward) {
        static auto startTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        UniformBufferObject data{};
        // ubo.model = rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        data.model = rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        data.view = lookAt(cameraPos, cameraPos+cameraForward, glm::vec3(0.0f, 1.0f, 0.0f));
        if(!zoom){
            data.proj = glm::perspective(glm::radians(90.0f), aspectRatio, 0.1f, 1000.0f);
        }else{
            data.proj = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 1000.0f);
        }
        
        // cause glm wasnt designed for Vulkan: (invert y aches)
        data.proj[1][1] *= -1;

        memcpy(ubo->frames[frameIndex]->buffersMapped, &data, sizeof(data));
    }
};

