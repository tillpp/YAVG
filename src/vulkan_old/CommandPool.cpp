#include "CommandPool.hpp"

CommandPool::CommandPool(Device& device, Queue& queue):queue(queue),device(device){
    vk::CommandPoolCreateInfo poolInfo{
        .flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = queue.queueFamilyIndex
    };
    commandPool = vk::raii::CommandPool(device.device, poolInfo);
}