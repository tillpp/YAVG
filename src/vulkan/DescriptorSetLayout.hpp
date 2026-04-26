#pragma once
#include "vulkan/Header.hpp"
#include "vulkan/RenderSync.hpp"
#include "DescriptorLayout.hpp"


class DescriptorSetLayout
{
public:
    vk::raii::DescriptorSetLayout descriptorSetLayout = nullptr;
    void create(Device& device,std::vector<DescriptorLayout> dsArray);
};
