#pragma once
#include "vulkan/Header.hpp"

class DeviceFeatures
{
public:
    std::function<bool(const vk::raii::PhysicalDevice& physicalDevice)> physicalDeviceSuitable;
    void* logicalDeviceFeatures;
    DeviceFeatures(void* logicalDeviceFeatures, std::function<bool(const vk::raii::PhysicalDevice& physicalDevice)> physicalDeviceSuitable);
};
