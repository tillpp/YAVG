#pragma once
#include "vulkan/setup/Device.hpp"
#include "vulkan/setup/Queue.hpp"

class CommandPool{
    Device* device = nullptr;
    Queue* queue = nullptr ;
public:
    vk::raii::CommandPool commandPool = nullptr;

    Device& getDevice()const;
    Queue& getQueue()const;

    void create(Device& device, Queue& queue);
};
