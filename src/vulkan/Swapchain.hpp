#pragma once
#include "vulkan/Window.hpp"
#include "vulkan/Device.hpp"
#include <limits>

class Swapchain
{
public:
    vk::SurfaceFormatKHR   surfaceFormat;
    vk::Extent2D           swapChainExtent;

    vk::raii::SwapchainKHR swapChain = nullptr;
    std::vector<vk::Image> images;
    std::vector<vk::raii::ImageView> imageViews;

    Swapchain(DeviceSettings& deviceSettings);
    ~Swapchain();

    void create(Window& window,Device& device);
    void recreate(Window& window,Device& device);
private:
    // choose surface settings
    static vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
    static  vk::PresentModeKHR chooseSwapPresentMode(std::vector<vk::PresentModeKHR> const &availablePresentModes);
    static  vk::Extent2D chooseSwapExtent(Window& window,vk::SurfaceCapabilitiesKHR const &capabilities);
    static  uint32_t chooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const &surfaceCapabilities);
        
};
