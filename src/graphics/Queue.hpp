#pragma once
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <atomic>
#include <map>
#include <optional>
#include "Instance.hpp"

class Queue
{
public:
    vk::raii::Queue queue = nullptr;
    
    virtual bool isQueueFamilySuitable(vk::QueueFamilyProperties const & qfp,size_t queueFamilyIndex, vk::raii::PhysicalDevice& physicalDevice)=0;
    
};
class GraphicsQueue:public Queue
{
    vk::raii::SurfaceKHR& surface;
public:

    GraphicsQueue(Window& window ):surface(window.surface){
    }
    virtual bool isQueueFamilySuitable(vk::QueueFamilyProperties const & qfp, size_t queueFamilyIndex, vk::raii::PhysicalDevice& physicalDevice) override{
        return (qfp.queueFlags & vk::QueueFlagBits::eGraphics) && physicalDevice.getSurfaceSupportKHR(queueFamilyIndex, *surface);
    } 
    
};