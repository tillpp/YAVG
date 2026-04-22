#pragma once
#include "vulkan/Instance.hpp"
#include "vulkan/Window.hpp"
#include "vulkan/ValidationLayer.hpp"
#include "vulkan/Device.hpp"
#include "vulkan/GraphicsQueue.hpp"
#include "vulkan_old/Swapchain.hpp"
#include "server/Server.hpp"
#include "GameFolder.hpp"

class Game
{
public:
    Instance instance;
    InstanceSettings instanceSettings;
    Window window;
    ValidationLayer validationLayer;
    DeviceSettings deviceSettings;
    Device device;
    GraphicsQueue queue;
    Swapchain swapchain;
    
    GameFolder gf;
    Server server;
    Game();
    ~Game();
};
