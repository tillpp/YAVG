
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
    {
        uint32_t imageIndex = 0;
        commandBuffer.begin(swapchain,0);

        {
            commandBuffer.commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline.graphicsPipeline);
            commandBuffer.commandBuffer.setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(swapchain.swapChainExtent.width), static_cast<float>(swapchain.swapChainExtent.height), 0.0f, 1.0f));
            commandBuffer.commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swapchain.swapChainExtent));

            commandBuffer.commandBuffer.draw(3, 1, 0, 0);

        }
        commandBuffer.end(swapchain,0);
    }

    while(window.update()){
        glfwPollEvents();   
    }
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
