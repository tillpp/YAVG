#pragma once
#include "vulkan/setup/Instance.hpp"
#include "vulkan/setup/Window.hpp"
#include "vulkan/setup/ValidationLayer.hpp"
#include "vulkan/setup/Device.hpp"
#include "vulkan/setup/GraphicsQueue.hpp"
#include "vulkan/setup/Swapchain.hpp"
#include "vulkan/setup/CommandPool.hpp"
#include "vulkan/setup/RenderSync.hpp"
#include "server/World.hpp"
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
    World world;
    
    std::filesystem::path projectBaseDir;
    Game(std::filesystem::path projectBaseDir);
    ~Game();
};
