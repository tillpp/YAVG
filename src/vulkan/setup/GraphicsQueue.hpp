#pragma once 
#include "vulkan/Header.hpp"
#include "vulkan/setup/Queue.hpp"
#include "vulkan/setup/Window.hpp"

class GraphicsQueue:public Queue
{
    vk::raii::SurfaceKHR* surface;
    using Queue::create;
public:
    void create(Window& window,DeviceSettings& deviceSettings);
    virtual bool isQueueFamilySuitable(vk::QueueFamilyProperties const & qfp, size_t queueFamilyIndex, vk::raii::PhysicalDevice& physicalDevice) override;   
};//game.