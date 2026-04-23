#include "Game.hpp"

Game::Game():
    window(&instanceSettings),
    validationLayer(&instanceSettings),
    swapchain(deviceSettings)
{
    instance.create(instanceSettings);
    window.create(instance,640, 720, "YAVoG");
    queue.create(window,deviceSettings);

    server.create(gf.directory/"saves"/"example");

    // Create a chain of feature structures
    vk::PhysicalDeviceFeatures2 a{.features = {.samplerAnisotropy = true}};// vk::PhysicalDeviceFeatures2 (empty for now)
    vk::PhysicalDeviceVulkan13Features b{.synchronization2 = true,.dynamicRendering = true}; // Enable dynamic rendering from Vulkan 1.3
    vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT c{.extendedDynamicState = true };  // Enable extended dynamic state from the extension
    vk::PhysicalDeviceVulkan11Features d{.shaderDrawParameters = true};
    a.pNext = &b;
    b.pNext = &c;
    c.pNext = &d;

    DeviceFeatures features(&a,[](const vk::raii::PhysicalDevice& physicalDevice)->bool
    {   
        // Check if the physicalDevice supports the required features (dynamic rendering and extended dynamic state)
        auto features = physicalDevice .template getFeatures2<
            vk::PhysicalDeviceFeatures2,
            vk::PhysicalDeviceVulkan13Features,
            vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT,
            vk::PhysicalDeviceVulkan11Features
        >();
        bool supportsRequiredFeatures = 
            features.template get<vk::PhysicalDeviceFeatures2>().features.samplerAnisotropy &&
            features.template get<vk::PhysicalDeviceVulkan13Features>().synchronization2 &&
            features.template get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
            features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState &&
            features.template get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters;
        return supportsRequiredFeatures;
    });

    device.create(instance,deviceSettings, features);
    swapchain.create(window,device);
    commandPool.create(device,queue);


}

Game::~Game()
{
}
