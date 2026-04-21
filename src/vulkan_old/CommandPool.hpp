#pragma once
#include "Device.hpp"
#include "Swapchain.hpp"
#include "vulkan/Queue.hpp"

class CommandPool{
public:
    Device& device;
    Queue& queue;
    vk::raii::CommandPool commandPool = nullptr;

    
    CommandPool(Device& device, Queue& queue);
};
