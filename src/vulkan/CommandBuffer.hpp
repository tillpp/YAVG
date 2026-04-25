#pragma once
#include "vulkan/Device.hpp"
#include "vulkan/CommandPool.hpp"
#include "vulkan/Queue.hpp"


class CommandBuffer
{
public:
    vk::raii::CommandBuffer commandBuffer = nullptr;
    CommandBuffer(CommandPool& pool);

    void beginSingleTimeCommands();
    void endSingleTimeCommands(CommandPool& pool);

    void begin();
    void end();
    
    //helper (TODO: maybe into Image.hpp?)
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
