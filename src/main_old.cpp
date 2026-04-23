

#include "client/GameFolder.hpp"
#include "vulkan/Window.hpp"
#include "vulkan/ValidationLayer.hpp"
#include "vulkan/GraphicsQueue.hpp"
#include "vulkan/Device.hpp"
#include "vulkan_old/Pipeline.hpp"
#include "vulkan_old/CommandBuffer.hpp"
#include "vulkan_old/DescriptorSetLayout.hpp"
#include "vulkan_old/Camera.hpp"

#include "client/Game.hpp"
#include <array>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "client/MeshWeaver.hpp"
#include "FastNoiseLite.h"


struct Chunk{
    Buffer vertexBuffer,indexBuffer;
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;

    void create(Device& device,CommandPool& pool,FastNoiseLite& noise,size_t xOffset,size_t yOffset,size_t zOffset){
         auto t1 = std::chrono::high_resolution_clock::now();
        MeshWeaver mw;
        {
            char* data = new char[33*33*33];
            for (size_t x = 0; x < 33; x++){
                for (size_t y = 0; y < 33; y++){
                    for (size_t z = 0; z < 33; z++){
                        int value = (noise.GetNoise((float)x+xOffset, (float)z+zOffset)*32+32)>(float) y+yOffset;
                        // if(x==0||y==0||z==0||x==32||y==32||z==32)
                            // value = 0;
                        data[x*33*33+y*33+z] = value;
                    }
                }
            }

            mw.create(data,xOffset,yOffset,zOffset);
            auto t2 =  std::chrono::high_resolution_clock::now();
            std::cout <<"mesh generation time:"<< std::chrono::duration_cast<std::chrono::microseconds>(t2-t1).count()<<"µs" <<std::endl;
        }
        vertices = *(std::vector<Vertex>*)&mw.vertices; // me being a bad boy. Because i am lazy.
        indices = mw.index;

        if(indices.size()){
            vertexBuffer.createVertexBuffer(pool,vertices);
            indexBuffer.createIndexBuffer(pool,indices);
        }
    }
    void draw(CommandBuffer& buffer){
        if(indices.size()){
            auto& commandBuffer = buffer.commandBuffer;
            commandBuffer.bindVertexBuffers(0, *vertexBuffer.buffer, {0});
            commandBuffer.bindIndexBuffer(*indexBuffer.buffer, 0, vk::IndexType::eUint16);
            commandBuffer.drawIndexed(static_cast<uint32_t>(indices.size()), 1, 0, 0,0);
        }
    }
};
void game(Game& _game) {
    time_t t;
    time(&t);
    FastNoiseLite noise(t);
    noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    noise.SetFractalType(FastNoiseLite::FractalType_FBm);
    noise.SetFrequency(0.005);
    //noise.SetFractalGain(0.75);
    noise.SetFractalOctaves(5);
    //noise.SetFractalLacunarity(2);    

    constexpr int MAX_FRAMES_IN_FLIGHT = 2;
    auto& swapchain = _game.swapchain;
    Image image;
    image.create(_game.commandPool);
    UBO ubo;
    ubo.create(_game.device,MAX_FRAMES_IN_FLIGHT);
    DescriptorSetLayout dsLayout;
    dsLayout.create(_game.device,
        {
            vk::DescriptorSetLayoutBinding( 0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex, nullptr),
            vk::DescriptorSetLayoutBinding( 1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment, nullptr)
        },
        MAX_FRAMES_IN_FLIGHT,
        ubo,image
    );
    
    
    DepthBuffer dephBuffer;
    dephBuffer.create(_game.commandPool,swapchain);
    Pipeline pipeline;
    pipeline.create(_game.device,
        std::filesystem::path("bin")/"slang.spv",
        "vertMain","fragMain",
        swapchain, dsLayout,dephBuffer);
    
    const size_t range = 5;
    Chunk chunk[range][range][range];
    for (size_t x = 0; x < range; x++)
    {
        for (size_t y = 0; y < range; y++)
        {
            for (size_t z = 0; z < range; z++)
            {
                chunk[x][y][z].create(_game.device,_game.commandPool,noise,x*32,-32+y*32,z*32);
            }
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
        dephBuffer.create(_game.commandPool,swapchain);
    };

    Camera camera;
    // TODO refactor the following in the future:
    auto recordCommandBuffer = [&](uint32_t imageIndex)
    {
        float aspectRatio = static_cast<float>(swapchain.swapChainExtent.width) / static_cast<float>(swapchain.swapChainExtent.height);

        bool zoom = glfwGetKey(_game.window,GLFW_KEY_C) == GLFW_PRESS;
        ubo.updateUniformBuffer(frameIndex,aspectRatio, zoom,camera.pos,camera.forward);
        commandBuffers[frameIndex].begin(swapchain,imageIndex,dephBuffer);
        auto& commandBuffer = commandBuffers[frameIndex].commandBuffer;
        {

            commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline.graphicsPipeline);
            commandBuffer.setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(swapchain.swapChainExtent.width), static_cast<float>(swapchain.swapChainExtent.height), 0.0f, 1.0f));
            commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swapchain.swapChainExtent));
            
            //TODO: learn more about dynamic descriptors
            commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline.pipelineLayout, 0, *dsLayout.descriptorSets[frameIndex], nullptr);
            
            for (size_t x = 0; x < range; x++)
            {
                for (size_t y = 0; y < range; y++)
                {
                    for (size_t z = 0; z < range; z++)
                    {
                        chunk[x][y][z].draw(commandBuffers[frameIndex]);
                    }
                }
            }
        }
        commandBuffers[frameIndex].end(swapchain,imageIndex);
    };


    

    assert(presentCompleteSemaphores.empty() && renderFinishedSemaphores.empty() && inFlightFences.empty());
    for (size_t i = 0; i < swapchain.images.size(); i++)
	{
		renderFinishedSemaphores.emplace_back(_game.device.device, vk::SemaphoreCreateInfo());
	}
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        presentCompleteSemaphores.emplace_back(_game.device.device, vk::SemaphoreCreateInfo());
        inFlightFences.emplace_back(_game.device.device, vk::FenceCreateInfo{.flags = vk::FenceCreateFlagBits::eSignaled});
        commandBuffers.emplace_back(_game.commandPool);
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
        if ((result == vk::Result::eSuboptimalKHR) || (result == vk::Result::eErrorOutOfDateKHR))
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
    bool grabMouse = true;
    if(grabMouse)
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
        //camera 
        if(grabMouse)
        {
            camera.update(_game.window,delta);   
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
