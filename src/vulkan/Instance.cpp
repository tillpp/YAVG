#include "Instance.hpp"

Instance::operator vk::raii::Instance&(){
    return instance;
}

void Instance::create(InstanceSettings settings){
    constexpr vk::ApplicationInfo appInfo{
        .pApplicationName   = "Yet Another Voxel Game",
        .applicationVersion = VK_MAKE_VERSION( 1, 0, 0 ),
        .pEngineName        = "No Engine",
        .engineVersion      = VK_MAKE_VERSION( 1, 0, 0 ),
        .apiVersion         = vk::ApiVersion14,
    };

    // for macOS
    {
        settings.flags |= vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;
        settings.extensions.push_back(vk::KHRPortabilityEnumerationExtensionName);
    }


    vk::InstanceCreateInfo createInfo{
        .flags = settings.flags,
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = (uint32_t)settings.layers.size(),
        .ppEnabledLayerNames = settings.layers.data(),
        .enabledExtensionCount = (uint32_t)settings.extensions.size(),
        .ppEnabledExtensionNames = settings.extensions.data(),
    };

    {
        auto extensions = context.enumerateInstanceExtensionProperties();
        std::cout << "available extensions:\n";
        for (const auto& extension : extensions) {
            std::cout << '\t' << extension.extensionName << '\n';
        }
    }
    {
        auto extensions = context.enumerateInstanceLayerProperties();
        std::cout << "available Layer:\n";
        for (const auto& extension : extensions) {
            std::cout << '\t' << extension.layerName << '\n';
        }
    }

    instance = vk::raii::Instance(context, createInfo);
}