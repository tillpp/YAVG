#pragma once
#include "vulkan/Device.hpp"
#include "Swapchain.hpp"
#include "CommandPool.hpp"
#include "vulkan/Queue.hpp"


class CommandBuffer
{
public:
    vk::raii::CommandBuffer commandBuffer = nullptr;
    CommandBuffer(CommandPool& pool);

    void beginSingleTimeCommands(){
        vk::CommandBufferBeginInfo beginInfo{ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit };
        commandBuffer.begin(beginInfo);
    }
    void endSingleTimeCommands(CommandPool& pool) {
        commandBuffer.end();

        vk::SubmitInfo submitInfo{ .commandBufferCount = 1, .pCommandBuffers = &*commandBuffer };
        pool.queue.submit(submitInfo, nullptr);
        pool.queue.waitIdle(); //TODO: use fence  instead to copy multiple buffers at once.
    }

    //TODO: begin, end and transition_image_layout need a better place to live.
    void begin(Swapchain& swapchain,uint32_t imageIndex,class DepthBuffer& depthBuffer);
    void end(Swapchain& swapchain,uint32_t imageIndex);
private: //helper
    void transition_image_layout(
        const vk::Image& image,
	    vk::ImageLayout         old_layout,
	    vk::ImageLayout         new_layout,
	    vk::AccessFlags2        src_access_mask,
	    vk::AccessFlags2        dst_access_mask,
	    vk::PipelineStageFlags2 src_stage_mask,
	    vk::PipelineStageFlags2 dst_stage_mask,
        vk::ImageAspectFlags    image_aspect_flags);
};
