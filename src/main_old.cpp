

#include "client/GameFolder.hpp"
#include "vulkan/Window.hpp"
#include "vulkan/ValidationLayer.hpp"
#include "vulkan/GraphicsQueue.hpp"
#include "vulkan_old/Device.hpp"
#include "vulkan_old/Pipeline.hpp"
#include "vulkan_old/CommandBuffer.hpp"
#include "client/Game.hpp"
#include <array>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "client/MeshWeaver.hpp"


void game(Game& _game) {
    
    MeshWeaver mw;
    char* data = new char[33*33*33];
    for (size_t x = 0; x < 33; x++){
        for (size_t y = 0; y < 33; y++){
            for (size_t z = 0; z < 33; z++){
                int value = rand()%2;
                if(x==0||y==0||z==0||x==32||y==32||z==32)
                    value = 0;
                data[x*33*33+y*33+z] = value;
            }
        }
    }
    mw.create(data);
    std::vector<Vertex> vertices = *(std::vector<Vertex>*)&mw.vertices; // me being a bad boy. Because i am lazy.
    std::vector<uint16_t> indices = mw.index;

    GameFolder gf;

    constexpr int MAX_FRAMES_IN_FLIGHT = 2;
    Swapchain swapchain(_game.deviceSettings);
    _game.device.create(_game.instance,_game.deviceSettings);
    swapchain.create(_game.window,_game.device);
    UBO ubo;
    ubo.create(_game.device,MAX_FRAMES_IN_FLIGHT);
    vk::raii::DescriptorSetLayout descriptorSetLayout = nullptr;
    {
        std::array bindings = {
            vk::DescriptorSetLayoutBinding( 0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex, nullptr),
            vk::DescriptorSetLayoutBinding( 1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment, nullptr)
        };

        vk::DescriptorSetLayoutCreateInfo layoutInfo{.bindingCount = bindings.size(), .pBindings = bindings.data()};
        descriptorSetLayout = vk::raii::DescriptorSetLayout(_game.device.device, layoutInfo);

    }
    vk::raii::DescriptorPool descriptorPool = nullptr;
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
        descriptorPool = vk::raii::DescriptorPool(_game.device.device, poolInfo);
    }
    CommandPool commandPool(_game.device,_game.queue);
    DepthBuffer dephBuffer;
    dephBuffer.create(commandPool,swapchain);
    
    Pipeline pipeline;
    pipeline.create(_game.device,swapchain, descriptorSetLayout,dephBuffer);
    Buffer vertexBuffer,indexBuffer;
    vertexBuffer.createVertexBuffer(_game.device,commandPool,vertices);
    indexBuffer.createIndexBuffer(_game.device,commandPool,indices);
    Image image;
    image.create(commandPool);

    std::vector<vk::raii::DescriptorSet> descriptorSets;
    {
        std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, *descriptorSetLayout);
        vk::DescriptorSetAllocateInfo allocInfo{ .descriptorPool = descriptorPool, .descriptorSetCount = static_cast<uint32_t>(layouts.size()), .pSetLayouts = layouts.data() };

        descriptorSets.clear();
        descriptorSets = _game.device.device.allocateDescriptorSets(allocInfo);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vk::DescriptorBufferInfo bufferInfo{ .buffer = ubo.uniformBuffers[i].buffer, .offset = 0, .range = sizeof(UniformBufferObject) };
            vk::DescriptorImageInfo imageInfo{ .sampler = image.textureSampler, .imageView = image.imageView, .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal};
            
            std::array descriptorWrites{
                vk::WriteDescriptorSet{ .dstSet = descriptorSets[i], .dstBinding = 0, .dstArrayElement = 0, .descriptorCount = 1,
                    .descriptorType = vk::DescriptorType::eUniformBuffer, .pBufferInfo = &bufferInfo },
                vk::WriteDescriptorSet{ .dstSet = descriptorSets[i], .dstBinding = 1, .dstArrayElement = 0, .descriptorCount = 1,
                    .descriptorType = vk::DescriptorType::eCombinedImageSampler, .pImageInfo = &imageInfo }
            };
            _game.device.device.updateDescriptorSets(descriptorWrites, {});
        }
    }


    uint32_t frameIndex = 0;
    
    std::vector<CommandBuffer> commandBuffers;
    std::vector<vk::raii::Semaphore> presentCompleteSemaphores;
    std::vector<vk::raii::Semaphore> renderFinishedSemaphores;
    std::vector<vk::raii::Fence> inFlightFences;
    
    auto recreateSwapChain = [&](){
        int width = 0, height = 0;
        glfwGetFramebufferSize(_game.window, &width, &height);
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(_game.window, &width, &height);
            glfwWaitEvents();
        }
        _game.device.device.waitIdle();

        swapchain.recreate(_game.window,_game.device);
        dephBuffer.create(commandPool,swapchain);
    };

    glm::vec3 cameraPos = glm::vec3(1.0f, 5.0f, 5.0f);
    glm::vec3 cameraForward = glm::vec3(0.0f, -1.0f, -1.0f);
    glm::vec3 cameraRight;
    
    // TODO refactor the following in the future:
    auto recordCommandBuffer = [&](uint32_t imageIndex)
    {
        float aspectRatio = static_cast<float>(swapchain.swapChainExtent.width) / static_cast<float>(swapchain.swapChainExtent.height);

        bool zoom = glfwGetKey(_game.window,GLFW_KEY_C) == GLFW_PRESS;
        ubo.updateUniformBuffer(frameIndex,aspectRatio, zoom,cameraPos,cameraForward);
        commandBuffers[frameIndex].begin(swapchain,imageIndex,dephBuffer);
        auto& commandBuffer = commandBuffers[frameIndex].commandBuffer;
        {

            commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline.graphicsPipeline);
            commandBuffer.setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(swapchain.swapChainExtent.width), static_cast<float>(swapchain.swapChainExtent.height), 0.0f, 1.0f));
            commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swapchain.swapChainExtent));
            
            commandBuffer.bindVertexBuffers(0, *vertexBuffer.buffer, {0});
            commandBuffer.bindIndexBuffer(*indexBuffer.buffer, 0, vk::IndexType::eUint16);
            //TODO: learn more about dynamic descriptors
            commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline.pipelineLayout, 0, *descriptorSets[frameIndex], nullptr);
            commandBuffer.drawIndexed(static_cast<uint32_t>(indices.size()), 1, 0, 0,0);
        }
        commandBuffers[frameIndex].end(swapchain,imageIndex);
    };


    

    assert(presentCompleteSemaphores.empty() && renderFinishedSemaphores.empty() && inFlightFences.empty());
    for (size_t i = 0; i < swapchain.swapChainImages.size(); i++)
	{
		renderFinishedSemaphores.emplace_back(_game.device.device, vk::SemaphoreCreateInfo());
	}
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        presentCompleteSemaphores.emplace_back(_game.device.device, vk::SemaphoreCreateInfo());
        inFlightFences.emplace_back(_game.device.device, vk::FenceCreateInfo{.flags = vk::FenceCreateFlagBits::eSignaled});
        commandBuffers.emplace_back(commandPool);
    }
    
    
    auto drawFrame = [&](){
        auto fenceResult = _game.device.device.waitForFences(*inFlightFences[frameIndex], vk::True, UINT64_MAX);
		if (fenceResult != vk::Result::eSuccess)
		{
			throw std::runtime_error("failed to wait for fence!");
		}
        auto [result, imageIndex] = swapchain.swapChain.acquireNextImage(UINT64_MAX, *presentCompleteSemaphores[frameIndex], nullptr);
        if(result == vk::Result::eErrorOutOfDateKHR){
            recreateSwapChain();
            return;
        }
        else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
        {
            assert(result == vk::Result::eTimeout || result == vk::Result::eNotReady);
            throw std::runtime_error("failed to acquire swap chain image!");
        }
        
        _game.device.device.resetFences(*inFlightFences[frameIndex]);
		commandBuffers[frameIndex].commandBuffer.reset();
        recordCommandBuffer(imageIndex);
        vk::PipelineStageFlags waitDestinationStageMask( vk::PipelineStageFlagBits::eColorAttachmentOutput );
        const vk::SubmitInfo   submitInfo{
            .waitSemaphoreCount   = 1,
            .pWaitSemaphores      = &*presentCompleteSemaphores[frameIndex],
            .pWaitDstStageMask    = &waitDestinationStageMask,
            .commandBufferCount   = 1,
            .pCommandBuffers      = &*commandBuffers[frameIndex].commandBuffer,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores    = &*renderFinishedSemaphores[imageIndex]
        };
        (_game.queue).submit(submitInfo, *inFlightFences[frameIndex]);


        const vk::PresentInfoKHR presentInfoKHR{
            .waitSemaphoreCount = 1,
            .pWaitSemaphores    = &*renderFinishedSemaphores[imageIndex],
            .swapchainCount     = 1,
            .pSwapchains        = &*swapchain.swapChain,
            .pImageIndices      = &imageIndex};
        result = _game.queue.presentKHR(presentInfoKHR);
        if ((result == vk::Result::eSuboptimalKHR) || (result == vk::Result::eErrorOutOfDateKHR) || _game.window.framebufferResized)
        {
            recreateSwapChain();
        }
        else
        {
            // There are no other success codes than eSuccess; on any error code, presentKHR already threw an exception.
            assert(result == vk::Result::eSuccess);
        }
        
        frameIndex = (frameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
    };

    //FPS counter
    auto lastSecond = std::chrono::steady_clock::now();
    size_t frames = 0;
    glfwSetInputMode(_game.window,GLFW_CURSOR,GLFW_CURSOR_DISABLED);
    auto lastFrame = std::chrono::high_resolution_clock::now();
    while(_game.window.update()){
        glfwPollEvents();
        drawFrame();   
        auto now = std::chrono::steady_clock::now();
        if(std::chrono::duration_cast<std::chrono::milliseconds>(now-lastSecond).count()>=1000){
            std::cout << "[FPS]" << frames << std::endl;
            frames = 0;
            lastSecond = now;
        }
        frames++;

        float delta;
        {
            auto currentTime = std::chrono::high_resolution_clock::now();
            delta = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastFrame).count();
            lastFrame = currentTime;
        }
        //camera rotation
        {
            double xpos, ypos;
            glfwGetCursorPos(_game.window, &xpos, &ypos);
            //glfwSetCursorPos(window.window,0,0);
            float sensitivity = 0.05;
            
            auto rotation = glm::mat4(1.f);
            rotation = glm::rotate(rotation, glm::radians(-(float)xpos*sensitivity) , glm::vec3(0.0f, 1.0f, 0.0f));
            rotation = glm::rotate(rotation, glm::radians(-(float)ypos*sensitivity) , glm::vec3(1.0f, 0.0f, 0.0f));
            cameraForward = rotation*glm::vec4(0.0f, 0, -1.0f,1.0f);
            cameraRight   = rotation*glm::vec4(1.0f, 0, 0.0f,1.0f);
        }
        //camera position
        float speed = 4.f;
        if(glfwGetKey(_game.window,GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS){
            speed = 20.f;
        }
        {
            if(glfwGetKey(_game.window,GLFW_KEY_W) == GLFW_PRESS){
                cameraPos += cameraForward*delta*speed;
            }
            if(glfwGetKey(_game.window,GLFW_KEY_S) == GLFW_PRESS){
                cameraPos -= cameraForward*delta*speed;
            }
            if(glfwGetKey(_game.window,GLFW_KEY_D) == GLFW_PRESS){
                cameraPos += cameraRight*delta*speed;
            }
            if(glfwGetKey(_game.window,GLFW_KEY_A) == GLFW_PRESS){
                cameraPos -= cameraRight*delta*speed;
            }
        }
        if(glfwGetKey(_game.window,GLFW_KEY_SPACE) == GLFW_PRESS){
            cameraPos += glm::vec3(0.f,1.f,0.f)*delta*speed;
        }
        if(glfwGetKey(_game.window,GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS){
            cameraPos -= glm::vec3(0.f,1.f,0.f)*delta*speed;
        }

    }
    _game.device.device.waitIdle();
}

int main(int argc, char const *argv[]){
    time_t t;
    time(&t);
    srand(t);

    try{
        Game _game;
        game(_game);
    } catch (const vk::SystemError& err){
        std::cerr << "Vulkan error: " << err.what() << std::endl;
        return 1;
    }catch (const std::exception& err){
        std::cerr << "Error: " << err.what() << std::endl;
        return 1;
    }
    return 0;
}
