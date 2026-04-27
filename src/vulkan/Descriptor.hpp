#pragma once
#include "vulkan/Header.hpp"
#include "vulkan/setup/RenderSync.hpp"

struct DescriptorLayout{
    uint32_t             binding;
    vk::ShaderStageFlags stageFlags;
    vk::DescriptorType   descriptorType;

    
    DescriptorLayout(
        uint32_t             binding,
        vk::ShaderStageFlags stageFlags,
        vk::DescriptorType   descriptorType
    );
    
    vk::DescriptorSetLayoutBinding getBinding()const;
};

class Descriptor:public DescriptorLayout{
public:
    class UBO*   ubo   = nullptr;
    class Image* image = nullptr;

    Descriptor(
        uint32_t             binding,
        vk::ShaderStageFlags stageFlags,
        class UBO&           ubo
    );
    Descriptor(
        uint32_t             binding,
        vk::ShaderStageFlags stageFlags,
        class Image&         image
    );
    
    vk::DescriptorBufferInfo bufferInfo;
    vk::DescriptorImageInfo imageInfo;
    void writeDescriptorSet(vk::WriteDescriptorSet& wds,size_t frameInFlight);   

};

class DescriptorSetLayout
{
public:
    vk::raii::DescriptorSetLayout descriptorSetLayout = nullptr;
    void create(Device& device,std::vector<DescriptorLayout> dsArray);
};


class DescriptorSet{
public:
    vk::raii::DescriptorPool descriptorPool = nullptr;
    std::vector<vk::raii::DescriptorSet> descriptorSets;


    void create(Device& device,RenderSync& render,DescriptorSetLayout& dsl,std::vector<Descriptor> dsArray);
    void use(vk::raii::CommandBuffer& commandBuffer,RenderSync& render, class Pipeline& pipeline,uint32_t firstSet = 0);
};

