#pragma once
#include "Device.hpp"
#include <glm/glm.hpp>
#include <array>
#include "CommandBuffer.hpp"

struct Vertex
{
    glm::vec2 pos;
    glm::vec3 color;

    static vk::VertexInputBindingDescription getBindingDescription()
    {
        return {.binding = 0, .stride = sizeof(Vertex), .inputRate = vk::VertexInputRate::eVertex};
    }
    static std::array<vk::VertexInputAttributeDescription, 2> getAttributeDescriptions()
    {
      return {{{.location = 0, .binding = 0, .format = vk::Format::eR32G32Sfloat, .offset = offsetof(Vertex, pos)},
               {.location = 1, .binding = 0, .format = vk::Format::eR32G32B32Sfloat, .offset = offsetof(Vertex, color)}}};
    }
};

const std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f}, {1.0f, 0.0f, 1.0f}}
};
const std::vector<uint16_t> indices = {
    0, 1, 2, 2, 3, 0
};

//NOTE: there are only 4096 max memory allocations, split bigger buffers into smaller ones with offset. (TODO: custom allocator)
//NOTE: IndexBuffer and VertexBuffer in one, is more cache friendly.
//TODO: learn more about "aliasing" in Vulkan
//TODO move this to Buffer.hpp
class Buffer{
public:
    vk::raii::DeviceMemory bufferMemory = nullptr;
    vk::raii::Buffer buffer = nullptr;

    static uint32_t findMemoryType(Device& device,uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
        vk::PhysicalDeviceMemoryProperties memProperties = device.physicalDevice.getMemoryProperties();
        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
        {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type!");
    }
    void createBuffer(Device& device,vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties) {
        vk::BufferCreateInfo bufferInfo{ .size = size, .usage = usage, .sharingMode = vk::SharingMode::eExclusive };
        buffer = vk::raii::Buffer(device.device, bufferInfo);
        vk::MemoryRequirements memRequirements = buffer.getMemoryRequirements();
        vk::MemoryAllocateInfo allocInfo{ .allocationSize = memRequirements.size, .memoryTypeIndex = findMemoryType(device,memRequirements.memoryTypeBits, properties) };
        bufferMemory = vk::raii::DeviceMemory(device.device, allocInfo);
        buffer.bindMemory(*bufferMemory, 0);
    }

    //TODO: use  VK_COMMAND_POOL_CREATE_TRANSIENT_BIT for this commandPool (short lives commandBuffers)
    //TODO: assert that Queue of commandPool needs to support eTransfer
    static void copyBuffer(CommandPool& commandPool,Buffer& srcBuffer, Buffer& dstBuffer, vk::DeviceSize size) {
        CommandBuffer commandBuffer(commandPool);
        commandBuffer.commandBuffer.begin(vk::CommandBufferBeginInfo{
            .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit
        });
        commandBuffer.commandBuffer.copyBuffer(srcBuffer.buffer,dstBuffer.buffer,vk::BufferCopy(0,0,size));
        commandBuffer.commandBuffer.end();

        commandPool.queue.queue.submit(vk::SubmitInfo{ .commandBufferCount = 1, .pCommandBuffers = &*commandBuffer.commandBuffer }, nullptr);
        commandPool.queue.queue.waitIdle(); //TODO: use fence  instead to copy multiple buffers at once.

    }
public:
    //create vertexBuffer
    void createVertexBuffer(Device& device,CommandPool& commandPool){
        //create StagingBuffer
        Buffer stagingBuffer;
        {
            auto size = sizeof(vertices[0]) * vertices.size();
            auto usage = vk::BufferUsageFlagBits::eTransferSrc;
            auto properties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
            stagingBuffer.createBuffer(device,size, usage,properties);

            void* dataStaging = stagingBuffer.bufferMemory.mapMemory(0, size);
            memcpy(dataStaging, vertices.data(), size);
            stagingBuffer.bufferMemory.unmapMemory();
        }

        auto size = sizeof(vertices[0]) * vertices.size();
        auto usage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst;
        auto properties = vk::MemoryPropertyFlagBits::eDeviceLocal;
        createBuffer(device,size, usage,properties);
        
        copyBuffer(commandPool,stagingBuffer, *this, size);
    }

    //TODO: a lot of duplicate code with VertexBuffer create
    // create IndexBuffer
    void createIndexBuffer(Device& device,CommandPool& commandPool){
        //create StagingBuffer
        Buffer stagingBuffer;
        {
            auto size = sizeof(indices[0]) * indices.size();
            auto usage = vk::BufferUsageFlagBits::eTransferSrc;
            auto properties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
            stagingBuffer.createBuffer(device,size, usage,properties);

            void* dataStaging = stagingBuffer.bufferMemory.mapMemory(0, size);
            memcpy(dataStaging, indices.data(), size);
            stagingBuffer.bufferMemory.unmapMemory();
        }

        auto size = sizeof(indices[0]) * indices.size();
        auto usage = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst;
        auto properties = vk::MemoryPropertyFlagBits::eDeviceLocal;
        createBuffer(device,size, usage,properties);
        
        copyBuffer(commandPool,stagingBuffer, *this, size);
    }
};
//TODO add a parameter in the pipeline to add VertexBuffer.
//TODO inherit Buffer privately.
//TODO: have IndexBuffer
class VertexBuffer:public Buffer
{
public:
    
};

