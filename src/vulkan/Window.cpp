#include "Window.hpp"

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
void Window::create(Instance& instance){
    // window 
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(640, 720, "YAVoG", nullptr, nullptr);
    glfwSetWindowUserPointer(window,(void*)this);
    
    // surface
    VkSurfaceKHR       _surface;
    if (glfwCreateWindowSurface(*instance.instance, window, nullptr, &_surface) != 0) {
        throw std::runtime_error("failed to create window surface!");
    }
    surface = vk::raii::SurfaceKHR(instance.instance, _surface);

    // callback 
    glfwSetFramebufferSizeCallback(window,[](GLFWwindow* window, int , int height){
        auto self = (Window*)glfwGetWindowUserPointer(window);
        self->framebufferResized = true;
    });
    glfwSetCursorPosCallback(window,[](GLFWwindow* window,double xpos, double ypos){
        auto self = (Window*)glfwGetWindowUserPointer(window);
        self->onCursorPos( xpos,ypos);
    });

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
void Window::onCursorPos(double xpos, double ypos){
}
