#pragma once
#include "vulkan/Header.hpp"
#include "vulkan/setup/Device.hpp"

class Queue:public vk::raii::Queue
{
public:
    Queue();
    
    uint32_t queueFamilyIndex;
    
    void create(DeviceSettings& queueList);
    virtual bool isQueueFamilySuitable(vk::QueueFamilyProperties const & qfp,size_t queueFamilyIndex, vk::raii::PhysicalDevice& physicalDevice)=0;
};