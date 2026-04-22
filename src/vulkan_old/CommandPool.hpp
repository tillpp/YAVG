#pragma once
#include "vulkan/Device.hpp"
#include "vulkan/Swapchain.hpp"
#include "vulkan/Queue.hpp"

class CommandPool{
public:
    Device& device;
    Queue& queue;
    vk::raii::CommandPool commandPool = nullptr;

    
    CommandPool(Device& device, Queue& queue);
};
