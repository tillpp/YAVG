#include "GraphicsQueue.hpp"

void GraphicsQueue::create(Window& window,DeviceSettings& deviceSettings){
    Queue::create(deviceSettings);
    surface = &window.surface;
}

bool GraphicsQueue::isQueueFamilySuitable(vk::QueueFamilyProperties const & qfp, size_t queueFamilyIndex, vk::raii::PhysicalDevice& physicalDevice){
    return (qfp.queueFlags & vk::QueueFlagBits::eGraphics)&& (qfp.queueFlags & vk::QueueFlagBits::eTransfer) && physicalDevice.getSurfaceSupportKHR(queueFamilyIndex, *surface);
} 