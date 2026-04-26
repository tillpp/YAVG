#include "DeviceFeatures.hpp"

DeviceFeatures::DeviceFeatures(void* logicalDeviceFeatures, std::function<bool(const vk::raii::PhysicalDevice& physicalDevice)> physicalDeviceSuitable):physicalDeviceSuitable(physicalDeviceSuitable),logicalDeviceFeatures(logicalDeviceFeatures){
    
}