#pragma once
#include "Device.hpp"
#include <glm/glm.hpp>
#include <array>

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
    {{0.0f, -0.5f}, {1.0f, 1.0f, 1.0f}},
    {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
};

//TODO add a parameter in the pipeline to add VertexBuffer.
class VertexBuffer
{
    vk::raii::DeviceMemory vertexBufferMemory = nullptr;
public:
    vk::raii::Buffer vertexBuffer = nullptr;

    uint32_t findMemoryType(Device& device,uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
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
    void create(Device& device){
        vk::BufferCreateInfo bufferInfo{
            .size        = sizeof(vertices[0]) * vertices.size(),
		    .usage       = vk::BufferUsageFlagBits::eVertexBuffer,
		    .sharingMode = vk::SharingMode::eExclusive};
        vertexBuffer = vk::raii::Buffer(device.device, bufferInfo);

        vk::MemoryRequirements memRequirements = vertexBuffer.getMemoryRequirements();

        vk::MemoryAllocateInfo memoryAllocateInfo{
           .allocationSize  = memRequirements.size,
            .memoryTypeIndex = findMemoryType(device,memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)
        };
        vertexBufferMemory = vk::raii::DeviceMemory(device.device, memoryAllocateInfo);
        vertexBuffer.bindMemory( *vertexBufferMemory, 0 );

        void* data = vertexBufferMemory.mapMemory(0, bufferInfo.size);
        memcpy(data, vertices.data(), bufferInfo.size);
        vertexBufferMemory.unmapMemory();
    }
};

