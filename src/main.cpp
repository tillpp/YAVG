
#include "GameFolder.hpp"
#include "graphics/Window.hpp"
#include "graphics/Instance.hpp"
#include "graphics/ValidationLayer.hpp"
#include "graphics/Device.hpp"

void game() {
    GameFolder gf;

    InstanceSettings instanceSettings;
    Window window(instanceSettings);
    ValidationLayer validationLayer(instanceSettings);
    Instance instance;
    instance.create(instanceSettings);
    Device device;

    GraphicsQueue queue;
    device.create(instance,{&queue});

    window.create();
    
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
