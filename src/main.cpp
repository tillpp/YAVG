
#include "GameFolder.hpp"
#include "graphics/Window.hpp"
#include "graphics/Instance.hpp"
#include "graphics/ValidationLayer.hpp"
#include "graphics/Device.hpp"
#include "graphics/Swapchain.hpp"

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
