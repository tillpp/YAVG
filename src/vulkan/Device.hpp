#pragma once
#include "vulkan/Header.hpp"
#include "vulkan/Instance.hpp"
#include "vulkan/DeviceFeatures.hpp"


struct DeviceSettings{
    std::vector<const char*> extensions;
    std::vector<class Queue*> queues;
};

class Device
{
public:
    vk::raii::PhysicalDevice physicalDevice = nullptr;
    vk::raii::Device device = nullptr;

    void create(Instance& instance,DeviceSettings settings,const DeviceFeatures& features);
private:
    std::optional<int> isDeviceSuitable( vk::raii::PhysicalDevice const & physicalDevice ,DeviceSettings settings,const DeviceFeatures& features);
    vk::raii::PhysicalDevice pickPhysicalDevice(Instance& instance,const DeviceSettings& settings,const DeviceFeatures& features);
    void initLogicalDevice(Instance& instance,const DeviceSettings& settings,const DeviceFeatures& features);
    
};
