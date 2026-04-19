
#include "GameFolder.hpp"
#include "graphics/Window.hpp"
#include "graphics/Instance.hpp"
#include "graphics/ValidationLayer.hpp"
#include "graphics/Device.hpp"
#include "graphics/Pipeline.hpp"
#include "graphics/CommandBuffer.hpp"


void game() {
    GameFolder gf;

    Instance instance;
    InstanceSettings instanceSettings;
    Window window(&instanceSettings);
    ValidationLayer validationLayer(&instanceSettings);
    instance.create(instanceSettings);
    window.create(instance);
    Device device;
    GraphicsQueue queue(window);
    DeviceSettings deviceSettings;
    Swapchain swapchain(deviceSettings);
    device.create(instance,deviceSettings,{&queue});
    swapchain.create(window,device);
    Pipeline pipeline;
    pipeline.create(device,swapchain);
    CommandPool commandPool(device,queue);
    

    uint32_t frameIndex = 0;
    constexpr int MAX_FRAMES_IN_FLIGHT = 2;

    std::vector<CommandBuffer> commandBuffers;
    std::vector<vk::raii::Semaphore> presentCompleteSemaphores;
    std::vector<vk::raii::Semaphore> renderFinishedSemaphores;
    std::vector<vk::raii::Fence> inFlightFences;
    
    auto recreateSwapChain = [&](){
        int width = 0, height = 0;
        glfwGetFramebufferSize(window.window, &width, &height);
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(window.window, &width, &height);
            glfwWaitEvents();
        }
        device.device.waitIdle();

        swapchain.recreate(window,device);
    };
    // TODO refactor the following in the future:
    auto recordCommandBuffer = [&](uint32_t imageIndex)
    {
        commandBuffers[frameIndex].begin(swapchain,imageIndex);
        auto& commandBuffer = commandBuffers[frameIndex].commandBuffer;
        {

            commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline.graphicsPipeline);
            commandBuffer.setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(swapchain.swapChainExtent.width), static_cast<float>(swapchain.swapChainExtent.height), 0.0f, 1.0f));
            commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swapchain.swapChainExtent));

            commandBuffer.draw(3, 2, 0, 0);

        }
        commandBuffers[frameIndex].end(swapchain,imageIndex);
    };


    

    assert(presentCompleteSemaphores.empty() && renderFinishedSemaphores.empty() && inFlightFences.empty());
    for (size_t i = 0; i < swapchain.swapChainImages.size(); i++)
	{
		renderFinishedSemaphores.emplace_back(device.device, vk::SemaphoreCreateInfo());
	}
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        presentCompleteSemaphores.emplace_back(device.device, vk::SemaphoreCreateInfo());
        inFlightFences.emplace_back(device.device, vk::FenceCreateInfo{.flags = vk::FenceCreateFlagBits::eSignaled});
        commandBuffers.emplace_back(commandPool);
    }
    
    
    auto drawFrame = [&](){
        auto fenceResult = device.device.waitForFences(*inFlightFences[frameIndex], vk::True, UINT64_MAX);
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
        
        device.device.resetFences(*inFlightFences[frameIndex]);
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
        queue.queue.submit(submitInfo, *inFlightFences[frameIndex]);


        const vk::PresentInfoKHR presentInfoKHR{
            .waitSemaphoreCount = 1,
            .pWaitSemaphores    = &*renderFinishedSemaphores[imageIndex],
            .swapchainCount     = 1,
            .pSwapchains        = &*swapchain.swapChain,
            .pImageIndices      = &imageIndex};
        result = queue.queue.presentKHR(presentInfoKHR);
        if ((result == vk::Result::eSuboptimalKHR) || (result == vk::Result::eErrorOutOfDateKHR) || window.framebufferResized)
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

    while(window.update()){
        glfwPollEvents();
        drawFrame();   
    }
    device.device.waitIdle();
}

int main(int argc, char const *argv[])
{
    try{
        game();
    } catch (const vk::SystemError& err){
        std::cerr << "Vulkan error: " << err.what() << std::endl;
        return 1;
    }catch (const std::exception& err){
        std::cerr << "Error: " << err.what() << std::endl;
        return 1;
    }
    return 0;
}
