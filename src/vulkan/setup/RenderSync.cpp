
#include "RenderSync.hpp"

void RenderSync::create(CommandPool& pool,Swapchain& swapchain){
    auto& device = pool.getDevice();
    assert(presentCompleteSemaphores.empty() && renderFinishedSemaphores.empty() && inFlightFences.empty());
    for (size_t i = 0; i < swapchain.images.size(); i++)
    {
        renderFinishedSemaphores.emplace_back(device.device, vk::SemaphoreCreateInfo());
    }
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        presentCompleteSemaphores.emplace_back(device.device, vk::SemaphoreCreateInfo());
        inFlightFences.emplace_back(device.device, vk::FenceCreateInfo{.flags = vk::FenceCreateFlagBits::eSignaled});
        commandBuffers.emplace_back(pool);
    }
    
}

uint32_t RenderSync::getFrameIndex()const{
    return frameIndex;
}
CommandBuffer& RenderSync::getCommandBuffer(){
    return commandBuffers[frameIndex];
}

void RenderSync::recreateSwapChain(Window& window,CommandPool& pool,Swapchain& swapchain,DepthBuffer* depthBuffer){
    int width = 0, height = 0;
    auto& device = pool.getDevice();
    glfwGetFramebufferSize(window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }
    device.device.waitIdle();

    swapchain.recreate(window,device);
    if(depthBuffer)
        depthBuffer->create(pool,swapchain);
}

bool RenderSync::begin(
        Window& window,
        Swapchain& swapchain,
        CommandPool& pool,
        DepthBuffer* depthBuffer,
        ImageIndex& imageIndex){

    auto& queue = pool.getQueue();
    auto& device = pool.getDevice();

    auto fenceResult = device.device.waitForFences(*inFlightFences[frameIndex], vk::True, UINT64_MAX);
	if (fenceResult != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to wait for fence!");
	}
    auto [result, _imageIndex] = swapchain.swapChain.acquireNextImage(UINT64_MAX, *presentCompleteSemaphores[frameIndex], nullptr);
    if(result == vk::Result::eErrorOutOfDateKHR){
        
        recreateSwapChain(window,pool,swapchain,depthBuffer);
        return false;
    }
    else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
    {
        assert(result == vk::Result::eTimeout || result == vk::Result::eNotReady);
        throw std::runtime_error("failed to acquire swap chain image!");
    }
    
    device.device.resetFences(*inFlightFences[frameIndex]);
	commandBuffers[frameIndex].commandBuffer.reset();

    imageIndex = _imageIndex;
    return true;
}
void RenderSync::end(
    Window& window,
    Swapchain& swapchain,
    CommandPool& pool,
    DepthBuffer* depthBuffer,
    ImageIndex imageIndex){
    
    auto& queue = pool.getQueue();
    auto& device = pool.getDevice();

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
    queue.submit(submitInfo, *inFlightFences[frameIndex]);


    const vk::PresentInfoKHR presentInfoKHR{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores    = &*renderFinishedSemaphores[imageIndex],
        .swapchainCount     = 1,
        .pSwapchains        = &*swapchain.swapChain,
        .pImageIndices      = &imageIndex};
    auto result = queue.presentKHR(presentInfoKHR);
    if ((result == vk::Result::eSuboptimalKHR) || (result == vk::Result::eErrorOutOfDateKHR)||window.framebufferResized)
    {
        recreateSwapChain(window,pool,swapchain,depthBuffer);
    }
    else
    {
        // There are no other success codes than eSuccess; on any error code, presentKHR already threw an exception.
        assert(result == vk::Result::eSuccess);
    }
    
    frameIndex = (frameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
}