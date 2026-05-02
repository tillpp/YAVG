#pragma once
#include "vulkan/setup/CommandPool.hpp"
#include "vulkan/setup/CommandBuffer.hpp"
#include "vulkan/vulkan.hpp"
#include <memory>
#include <vector>
#include <vulkan/vulkan_raii.hpp>

typedef uint32_t ImageIndex;
class RenderSync
{
    uint32_t frameIndex = 0;
    
    std::vector<CommandBuffer> commandBuffers;
    std::vector<vk::raii::Semaphore> presentCompleteSemaphores;
    std::vector<vk::raii::Semaphore> renderFinishedSemaphores;
    std::vector<vk::raii::Fence> inFlightFences;
    
    Device* device = nullptr;

    struct TrashLayer{
        std::vector<vk::raii::Buffer> buffers;
        std::vector<vk::raii::DeviceMemory> deviceMemorys;
        std::vector<vk::raii::Pipeline> pipelines;
        std::vector<vk::raii::PipelineLayout> pipelineLayouts;        
        std::vector<vk::raii::DescriptorSet> descriptorSets;
        std::vector<vk::raii::DescriptorPool> descriptorPools;
        std::vector<std::shared_ptr<class ResourceReincarnation>> reincarnations;
        
        void clear();
    };
public:
    ~RenderSync();
    uint32_t getFrameIndex()const;
    CommandBuffer& getCommandBuffer();
    
    static const uint32_t MAX_FRAMES_IN_FLIGHT = 2;
    
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

    void trash(vk::raii::Buffer buffer,vk::raii::DeviceMemory deviceMemory){
        trashLayer[getFrameIndex()].buffers.push_back(std::move(buffer));
        trashLayer[getFrameIndex()].deviceMemorys.push_back(std::move(deviceMemory));
    }
    void trash(vk::raii::PipelineLayout pipelineLayout,vk::raii::Pipeline pipeline){
        trashLayer[getFrameIndex()].pipelineLayouts.push_back(std::move(pipelineLayout));
        trashLayer[getFrameIndex()].pipelines.push_back(std::move(pipeline));
    }
    void trash(vk::raii::DescriptorPool descriptorPool){
        trashLayer[getFrameIndex()].descriptorPools.push_back(std::move(descriptorPool));
    }
    void trash(vk::raii::DescriptorSet descriptorSet){
        trashLayer[getFrameIndex()].descriptorSets.push_back(std::move(descriptorSet));
    }
    void trash(std::shared_ptr<class ResourceReincarnation> reincarnation){
        trashLayer[getFrameIndex()].reincarnations.push_back(reincarnation);
    }
private:
    TrashLayer trashLayer[MAX_FRAMES_IN_FLIGHT];
    
};