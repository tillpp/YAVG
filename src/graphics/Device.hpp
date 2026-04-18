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
#include "Queue.hpp"


struct DeviceSettings{
    std::vector<const char*> extensions;
};

class Device
{
public:
    vk::raii::PhysicalDevice physicalDevice = nullptr;
    vk::raii::Device device = nullptr;

    void create(Instance& instance,DeviceSettings settings,std::vector<Queue*> queues);
private:
    std::optional<int> isDeviceSuitable( vk::raii::PhysicalDevice const & physicalDevice ,DeviceSettings settings);
    vk::raii::PhysicalDevice pickPhysicalDevice(Instance& instance,const DeviceSettings& settings);
    
};
