
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
    CommandBuffer commandBuffer(commandPool);
    
    auto recordCommandBuffer = [&](uint32_t imageIndex)
    {
        commandBuffer.begin(swapchain,imageIndex);

        {
            commandBuffer.commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline.graphicsPipeline);
            commandBuffer.commandBuffer.setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(swapchain.swapChainExtent.width), static_cast<float>(swapchain.swapChainExtent.height), 0.0f, 1.0f));
            commandBuffer.commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swapchain.swapChainExtent));

            commandBuffer.commandBuffer.draw(3, 1, 0, 0);

        }
        commandBuffer.end(swapchain,imageIndex);
    };



    vk::raii::Semaphore presentCompleteSemaphore = nullptr;
    vk::raii::Semaphore renderFinishedSemaphore  = nullptr;
    vk::raii::Fence     drawFence                = nullptr;

    auto drawFrame = [&](){
        auto fenceResult = device.device.waitForFences(*drawFence, vk::True, UINT64_MAX);
		if (fenceResult != vk::Result::eSuccess)
		{
			throw std::runtime_error("failed to wait for fence!");
		}
		device.device.resetFences(*drawFence);
        auto [result, imageIndex] = swapchain.swapChain.acquireNextImage(UINT64_MAX, *presentCompleteSemaphore, nullptr);
        recordCommandBuffer(imageIndex);
        queue.queue.waitIdle();
        vk::PipelineStageFlags waitDestinationStageMask( vk::PipelineStageFlagBits::eColorAttachmentOutput );
        const vk::SubmitInfo   submitInfo{
            .waitSemaphoreCount   = 1,
            .pWaitSemaphores      = &*presentCompleteSemaphore,
            .pWaitDstStageMask    = &waitDestinationStageMask,
            .commandBufferCount   = 1,
            .pCommandBuffers      = &*commandBuffer.commandBuffer,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores    = &*renderFinishedSemaphore
        };
        queue.queue.submit(submitInfo, *drawFence);

        const vk::PresentInfoKHR presentInfoKHR{
            .waitSemaphoreCount = 1,
            .pWaitSemaphores    = &*renderFinishedSemaphore,
            .swapchainCount     = 1,
            .pSwapchains        = &*swapchain.swapChain,
            .pImageIndices      = &imageIndex};
        result = queue.queue.presentKHR(presentInfoKHR);
    };

    presentCompleteSemaphore = vk::raii::Semaphore(device.device, vk::SemaphoreCreateInfo());
    renderFinishedSemaphore  = vk::raii::Semaphore(device.device, vk::SemaphoreCreateInfo());
    drawFence                = vk::raii::Fence(device.device, {.flags = vk::FenceCreateFlagBits::eSignaled});

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
