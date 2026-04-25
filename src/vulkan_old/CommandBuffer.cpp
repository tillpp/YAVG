#include "CommandBuffer.hpp"
#include "DepthBuffer.hpp"


CommandBuffer::CommandBuffer(CommandPool& pool){
vk::CommandBufferAllocateInfo allocInfo{ .commandPool = *pool.commandPool, .level = vk::CommandBufferLevel::ePrimary, .commandBufferCount = 1 };

commandBuffer = std::move(vk::raii::CommandBuffers(pool.getDevice().device, allocInfo).front());
}
void CommandBuffer::begin(Swapchain& swapchain,uint32_t imageIndex){
    commandBuffer.begin({});
    
}
void CommandBuffer::end(Swapchain& swapchain,uint32_t imageIndex){
    commandBuffer.end();
}

void CommandBuffer::beginRendering(Swapchain& swapchain,uint32_t imageIndex,DepthBuffer* depthBuffer){
    
    // Before starting rendering, transition the swapchain image to vk::ImageLayout::eColorAttachmentOptimal
    transition_image_layout(
        swapchain.images[imageIndex],
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal,
        {},                                                        // srcAccessMask (no need to wait for previous operations)
        vk::AccessFlagBits2::eColorAttachmentWrite,                // dstAccessMask
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,        // srcStage
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,         // dstStage
        vk::ImageAspectFlagBits::eColor
    );

    if(depthBuffer){
        transition_image_layout(
            *(*depthBuffer).image.image,
            vk::ImageLayout::eUndefined,
            vk::ImageLayout::eDepthAttachmentOptimal,
            vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
            vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
            vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
            vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
            vk::ImageAspectFlagBits::eDepth
        );
    }

    vk::ClearValue clearColor = vk::ClearColorValue(0.01f, 0.0f, 0.01f, 1.0f);
    vk::ClearValue clearDepth = vk::ClearDepthStencilValue(1.0f, 0);
    

    vk::RenderingAttachmentInfo attachmentInfo = {
        .imageView   = swapchain.imageViews[imageIndex],
        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .loadOp      = vk::AttachmentLoadOp::eClear,
        .storeOp     = vk::AttachmentStoreOp::eStore,
        .clearValue  = clearColor};

    vk::RenderingInfo renderingInfo = {
        .renderArea           = {.offset = {0, 0}, .extent = swapchain.swapChainExtent},
        .layerCount           = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments    = &attachmentInfo,
    };
    vk::RenderingAttachmentInfo depthAttachmentInfo;
    if(depthBuffer){
        depthAttachmentInfo = {
            .imageView   = (*depthBuffer).image.imageView,
            .imageLayout = vk::ImageLayout::eDepthAttachmentOptimal,
            .loadOp      = vk::AttachmentLoadOp::eClear,
            .storeOp     = vk::AttachmentStoreOp::eDontCare,
            .clearValue  = clearDepth
        };
        renderingInfo.pDepthAttachment     = &depthAttachmentInfo;

    }
    commandBuffer.beginRendering(renderingInfo);
}
void CommandBuffer::endRendering(Swapchain& swapchain,uint32_t imageIndex){
    commandBuffer.endRendering();

    // After rendering, transition the swapchain image to vk::ImageLayout::ePresentSrcKHR
    transition_image_layout(
        swapchain.images[imageIndex],
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::ImageLayout::ePresentSrcKHR,
        vk::AccessFlagBits2::eColorAttachmentWrite,             // srcAccessMask
        {},                                                     // dstAccessMask
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,     // srcStage
        vk::PipelineStageFlagBits2::eBottomOfPipe,               // dstStage
        vk::ImageAspectFlagBits::eColor
    );

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
