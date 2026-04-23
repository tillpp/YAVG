#include "CommandPool.hpp"


Device& CommandPool::getDevice()const{
    return *device;
}
Queue& CommandPool::getQueue()const{
    return *queue;
}

void CommandPool::create(Device& device, Queue& queue){
    this->device = &device;
    this->queue  = &queue;
    
    vk::CommandPoolCreateInfo poolInfo{
        .flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = queue.queueFamilyIndex
    };
    commandPool = vk::raii::CommandPool(device.device, poolInfo);

}
