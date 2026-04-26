#pragma once
#include "vulkan/setup/Instance.hpp"
#include "vulkan/setup/Window.hpp"
#include "vulkan/setup/ValidationLayer.hpp"
#include "vulkan/setup/Device.hpp"
#include "vulkan/setup/GraphicsQueue.hpp"
#include "vulkan/setup/Swapchain.hpp"
#include "vulkan/setup/CommandPool.hpp"
#include "vulkan/setup/RenderSync.hpp"
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
    CommandPool commandPool;    
    RenderSync render;

    GameFolder gf;
    Server server;
    
    std::filesystem::path projectBaseDir;
    Game(std::filesystem::path projectBaseDir);
    ~Game();
};
