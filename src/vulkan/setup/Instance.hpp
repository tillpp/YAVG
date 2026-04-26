#pragma once
#include "vulkan/Header.hpp"

struct InstanceSettings{
    std::vector<const char*> extensions;
    std::vector<const char*> layers;
    vk::InstanceCreateFlags flags{};
};
//TODO: vulkan give objects name
class Instance
{
    vk::raii::Context  context;
    vk::raii::Instance instance = nullptr;
public:
    operator vk::raii::Instance&();

    void create(InstanceSettings extensions);

};