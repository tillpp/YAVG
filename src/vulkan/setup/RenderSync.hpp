#pragma once
#include "vulkan/setup/Device.hpp"
#include "vulkan/setup/GraphicsQueue.hpp"
#include "vulkan/setup/CommandPool.hpp"
#include "vulkan/setup/CommandBuffer.hpp"
#include "vulkan/DepthBuffer.hpp"

typedef uint32_t ImageIndex;
class RenderSync
{
    uint32_t frameIndex = 0;
    
    std::vector<CommandBuffer> commandBuffers;
    std::vector<vk::raii::Semaphore> presentCompleteSemaphores;
    std::vector<vk::raii::Semaphore> renderFinishedSemaphores;
    std::vector<vk::raii::Fence> inFlightFences;
    
public:
    uint32_t getFrameIndex()const;
    CommandBuffer& getCommandBuffer();
    
    const uint32_t MAX_FRAMES_IN_FLIGHT = 2;
    
    void create(CommandPool& pool,Swapchain& swapchain);
    void recreateSwapChain(Window& window,CommandPool& pool,Swapchain& swapchain,DepthBuffer* depthBuffer);

    bool begin(
        Window& window,
        Swapchain& swapchain,
        CommandPool& pool,
        DepthBuffer* depthBuffer,
        ImageIndex& imageIndex);
    void end(
        Window& window,
        Swapchain& swapchain,
        CommandPool& pool,
        DepthBuffer* depthBuffer,
        ImageIndex imageIndex);
};