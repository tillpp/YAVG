#pragma once
#include "vulkan/Header.hpp"
#include "vulkan/setup/Instance.hpp"


class Window
{
    GLFWwindow *window = nullptr;
    
    struct{
        int xpos,ypos;
        int sizex,sizey;
    }beforeFullscreen;
    
public:
    vk::raii::SurfaceKHR surface = nullptr;
    Window(InstanceSettings* settings);
    Window(const Window&)=delete;
    ~Window();

    void create(Instance& instance,int width, int height, const char *title);
    void close();
    bool update();
    
    operator GLFWwindow*();
    
    void toggleFullscreen();
    void toggleMouseGrab();

    bool framebufferResized = false;
    bool grabMouse = true;

    std::u32string textInput;
};
            