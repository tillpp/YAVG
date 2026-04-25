#pragma once
#include "vulkan/Device.hpp"
#include "vulkan/GraphicsQueue.hpp"
#include "vulkan/CommandPool.hpp"
#include "vulkan_old/CommandBuffer.hpp"
#include "vulkan_old/DepthBuffer.hpp"


class Render
{
    uint32_t frameIndex = 0;
    
    std::vector<CommandBuffer> commandBuffers;
    std::vector<vk::raii::Semaphore> presentCompleteSemaphores;
    std::vector<vk::raii::Semaphore> renderFinishedSemaphores;
    std::vector<vk::raii::Fence> inFlightFences;
    
public:
    uint32_t getFrameIndex()const;
    const int MAX_FRAMES_IN_FLIGHT = 2;
    
    void create(CommandPool& pool,Swapchain& swapchain);
    void close(Device& device);
    void recreateSwapChain(Window& window,CommandPool& pool,Swapchain& swapchain,DepthBuffer* depthBuffer);
    void draw(
        Window& window,
        Swapchain& swapchain,
        CommandPool& pool,
        DepthBuffer* depthBuffer, 
        std::function<void(CommandBuffer& commandBuffer,uint32_t frameIndex,uint32_t imageIndex)> recordCommandBuffer);
};