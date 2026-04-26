#pragma once
#include "vulkan/Header.hpp"
#include "vulkan/setup/Instance.hpp"
#include "vulkan/setup/DeviceFeatures.hpp"


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
    void initLogicalDevice(const DeviceSettings& settings,const DeviceFeatures& features);
    
};
