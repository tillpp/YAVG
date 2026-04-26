#pragma once
#include "Descriptor.hpp"
#include "DescriptorSetLayout.hpp"


class DescriptorSet{
public:
    vk::raii::DescriptorPool descriptorPool = nullptr;
    std::vector<vk::raii::DescriptorSet> descriptorSets;


    void create(Device& device,RenderSync& render,DescriptorSetLayout& dsl,std::vector<Descriptor> dsArray);
    void use(vk::raii::CommandBuffer& commandBuffer,RenderSync& render, class Pipeline& pipeline,uint32_t firstSet = 0);
};