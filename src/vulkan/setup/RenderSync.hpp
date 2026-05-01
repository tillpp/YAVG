#pragma once
#include "vulkan/setup/CommandPool.hpp"
#include "vulkan/setup/CommandBuffer.hpp"

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
    
    void create(class CommandPool& pool,class Swapchain& swapchain);
    void recreateSwapChain(class Window& window,class CommandPool& pool,class Swapchain& swapchain,class DepthBuffer* depthBuffer);

    bool begin(
        class Window& window,
        class Swapchain& swapchain,
        class CommandPool& pool,
        class DepthBuffer* depthBuffer,
        ImageIndex& imageIndex);
    void end(
        class Window& window,
        class Swapchain& swapchain,
        class CommandPool& pool,
        class DepthBuffer* depthBuffer,
        ImageIndex imageIndex);
};