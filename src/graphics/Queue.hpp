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
    
    virtual bool isQueueFamilySuitable(vk::QueueFamilyProperties const & qfp)=0;
    
};
class GraphicsQueue:public Queue
{
public:
    
    virtual bool isQueueFamilySuitable(vk::QueueFamilyProperties const & qfp) override{
        return (qfp.queueFlags & vk::QueueFlagBits::eGraphics) != static_cast<vk::QueueFlags>(0);
    } 
    
};