#include "CommandBuffer.hpp"
#include "vulkan/DepthBuffer.hpp"


CommandBuffer::CommandBuffer(CommandPool& pool){
    vk::CommandBufferAllocateInfo allocInfo{ .commandPool = *pool.commandPool, .level = vk::CommandBufferLevel::ePrimary, .commandBufferCount = 1 };
    commandBuffer = std::move(vk::raii::CommandBuffers(pool.getDevice().device, allocInfo).front());
}
void CommandBuffer::beginSingleTimeCommands(){
    vk::CommandBufferBeginInfo beginInfo{ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit };
    commandBuffer.begin(beginInfo);
}
void CommandBuffer::endSingleTimeCommands(CommandPool& pool) {
    commandBuffer.end();

    vk::SubmitInfo submitInfo{ .commandBufferCount = 1, .pCommandBuffers = &*commandBuffer };
    pool.getQueue().submit(submitInfo, nullptr);
    pool.getQueue().waitIdle(); //TODO: use fence  instead to copy multiple buffers at once.
}

void CommandBuffer::begin(){
    commandBuffer.begin({});
    
}
void CommandBuffer::end(){
    commandBuffer.end();
}

void CommandBuffer::transition_image_layout(
        const vk::Image& image,
	    vk::ImageLayout         old_layout,
	    vk::ImageLayout         new_layout,
	    vk::AccessFlags2        src_access_mask,
	    vk::AccessFlags2        dst_access_mask,
	    vk::PipelineStageFlags2 src_stage_mask,
	    vk::PipelineStageFlags2 dst_stage_mask,
        vk::ImageAspectFlags    image_aspect_flags)
{
        vk::ImageMemoryBarrier2 barrier = {
            .srcStageMask        = src_stage_mask,
            .srcAccessMask       = src_access_mask,
            .dstStageMask        = dst_stage_mask,
            .dstAccessMask       = dst_access_mask,
            .oldLayout           = old_layout,
            .newLayout           = new_layout,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image               = image,
            .subresourceRange    = {
                .aspectMask     = image_aspect_flags,
                .baseMipLevel   = 0,
                .levelCount     = 1,
                .baseArrayLayer = 0,
                .layerCount     = 1}};
        vk::DependencyInfo dependency_info = {
            .dependencyFlags         = {},
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers    = &barrier};
    commandBuffer.pipelineBarrier2(dependency_info);
}
