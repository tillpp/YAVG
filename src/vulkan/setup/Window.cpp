#include "Window.hpp"
#include "GLFW/glfw3.h"

int64_t glfwCount = 0;


Window::Window(InstanceSettings* settings)
{
    if(glfwCount == 0){
        glfwInit();
    }
    glfwCount++;

    // add extensions to wishlist:
    uint32_t glfwExtensionCount = 0;
    auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    settings->extensions.insert(settings->extensions.end(),glfwExtensions,glfwExtensions+glfwExtensionCount);

    // check if the extensions are available
    vk::raii::Context context;
    auto extensionProperties = context.enumerateInstanceExtensionProperties();
    for (uint32_t i = 0; i < glfwExtensionCount; ++i)
    {
        if (std::ranges::none_of(extensionProperties,
            [glfwExtension = glfwExtensions[i]](auto const& extensionProperty)
            { return strcmp(extensionProperty.extensionName, glfwExtension) == 0; }))
        {
            throw std::runtime_error("Required GLFW extension not supported: " + std::string(glfwExtensions[i]));
        }
    }
}

Window::~Window()
{
    close();
    glfwCount--;
    if(glfwCount == 0){
        glfwTerminate();
    }
}
Window::operator GLFWwindow*(){
    return window;
}
void Window::create(Instance& instance,int width, int height, const char *title){
    // window 
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(width,height,title, nullptr, nullptr);
    glfwSetWindowUserPointer(window,(void*)this);
    
    // surface
    VkSurfaceKHR       _surface;
    if (glfwCreateWindowSurface(*(vk::raii::Instance&)instance, window, nullptr, &_surface) != 0) {
        throw std::runtime_error("failed to create window surface!");
    }
    surface = vk::raii::SurfaceKHR(instance, _surface);

    // callback 
    glfwSetFramebufferSizeCallback(window,[](GLFWwindow* window, int , int height){
        auto self = (Window*)glfwGetWindowUserPointer(window);
        self->framebufferResized = true;
    });
    glfwSetKeyCallback(window,[](GLFWwindow* window, int key, int scancode, int action, int mods){
        auto self = (Window*)glfwGetWindowUserPointer(window);
        if(key == GLFW_KEY_F11 && action == GLFW_PRESS){
            self->toggleFullscreen();
        }else if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
            self->toggleMouseGrab();
        }


    });
    glfwSetCharCallback(window, [](GLFWwindow* window,unsigned int codepoint){
        auto self = (Window*)glfwGetWindowUserPointer(window);
        self->textInput.push_back(codepoint);
    });
    toggleMouseGrab();
}
void Window::close(){
    glfwDestroyWindow(window);
    window = nullptr;
}
bool Window::update(){
    assert(window);
    // TODO: maybe have an EventQueue that resets all changed variables. If we got a lot.
    framebufferResized = false;
    return !glfwWindowShouldClose(window);
}
void Window::toggleFullscreen(){
    int count;
    auto monitors =  glfwGetMonitors(&count);
    auto monitor = monitors[0];
    if(glfwGetWindowMonitor(window) == nullptr){
        const GLFWvidmode * mode = glfwGetVideoMode(monitor);
        glfwGetWindowPos(window,&beforeFullscreen.xpos,&beforeFullscreen.xpos);
        glfwGetWindowSize(window,&beforeFullscreen.sizex,&beforeFullscreen.sizey);
        glfwSetWindowMonitor(window,monitor,0,0,mode->width,mode->height,0);
        return;
    }else{
        glfwSetWindowMonitor(window,nullptr,beforeFullscreen.xpos,beforeFullscreen.ypos,beforeFullscreen.sizex,beforeFullscreen.sizey,0);
    }
}
void Window::toggleMouseGrab(){
    grabMouse =! grabMouse;
    if(grabMouse)  {
        glfwSetInputMode(window,GLFW_CURSOR,GLFW_CURSOR_DISABLED);
        if (glfwRawMouseMotionSupported())
            glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    }
    else    
        glfwSetInputMode(window,GLFW_CURSOR,GLFW_CURSOR_NORMAL);
}