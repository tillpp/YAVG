#pragma once
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <atomic>
#include "Instance.hpp"

class Window
{
public:
    int xpos,ypos;
    GLFWwindow *window = nullptr;
    vk::raii::SurfaceKHR surface = nullptr;

    
    Window(InstanceSettings* settings);
    Window(const Window&)=delete;
    ~Window();

    void create(Instance& instance);
    void close();
    bool update();

    //callbacks
    void onCursorPos(double xpos, double ypos);

    bool framebufferResized = false;
};
