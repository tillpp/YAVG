#pragma once
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <atomic>
#include <map>
#include <optional>
#include "Instance.hpp"
#include "Queue.hpp"

class Device
{
    vk::raii::PhysicalDevice physicalDevice = nullptr;
    vk::raii::Device device = nullptr;
public:
    std::vector<const char*> requiredDeviceExtension = {vk::KHRSwapchainExtensionName};

    void create(Instance& instance,std::vector<Queue*> queues){
        physicalDevice = pickPhysicalDevice(instance);

        
        struct QueueInfo{
            Queue*  queue;
            float priority;
        };
        std::map<unsigned int,std::vector<QueueInfo>>  queueMap;
        {
            std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();
            for (auto &&queue : queues){
                auto queueFamilyProperty = std::ranges::find_if(queueFamilyProperties, [&queue](auto const &qfp) {
                    return queue->isQueueFamilySuitable(qfp);
                });
                auto index = static_cast<uint32_t>(std::distance(queueFamilyProperties.begin(), queueFamilyProperty));
                float queuePriority = 0.5f;
                
                queueMap[index].push_back((QueueInfo){
                    .queue = queue,
                    .priority = queuePriority,
                });
             }
        }
        std::vector<vk::DeviceQueueCreateInfo> deviceQueueCreateInfos;
        for(auto&& pair: queueMap){
            auto familyIndex = pair.first;
            std::vector<float> priorities;
            {
                priorities.reserve(pair.second.size());
                std::ranges::transform(pair.second,std::back_inserter(priorities), &QueueInfo::priority);
            }
                
            deviceQueueCreateInfos.push_back(vk::DeviceQueueCreateInfo{ 
                .queueFamilyIndex = familyIndex,
                .queueCount = (unsigned int)priorities.size(),
                .pQueuePriorities = priorities.data()
            });
        }

        // Create a chain of feature structures
        vk::PhysicalDeviceFeatures2 a{};// vk::PhysicalDeviceFeatures2 (empty for now)
        vk::PhysicalDeviceVulkan13Features b{.dynamicRendering = true}; // Enable dynamic rendering from Vulkan 1.3
        vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT c{.extendedDynamicState = true };  // Enable extended dynamic state from the extension
        a.pNext = &b;
        b.pNext = &c;


        vk::DeviceCreateInfo deviceCreateInfo{
            .pNext = &a,
            .queueCreateInfoCount = deviceQueueCreateInfos.size(),
            .pQueueCreateInfos = deviceQueueCreateInfos.data(),
            .enabledExtensionCount = static_cast<uint32_t>(requiredDeviceExtension.size()),
            .ppEnabledExtensionNames = requiredDeviceExtension.data()
        }; 
        device = vk::raii::Device( physicalDevice, deviceCreateInfo );

        for(auto&& pair: queueMap){
            auto familyIndex = pair.first;
            
            for (size_t i = 0; i < pair.second.size(); i++)
            {
                pair.second[i].queue->queue = vk::raii::Queue( device, familyIndex, i );
            }
            
        }
    }
private:
    std::optional<int> isDeviceSuitable( vk::raii::PhysicalDevice const & physicalDevice )
    {
        auto deviceProperties = physicalDevice.getProperties();
        auto deviceFeatures = physicalDevice.getFeatures();
        uint32_t score = 0;

        // Discrete GPUs have a significant performance advantage
        if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
            score += 1000;
        }

        // Maximum possible size of textures affects graphics quality
        score += deviceProperties.limits.maxImageDimension2D;

        // Application can't function without geometry shaders
        if (!deviceFeatures.geometryShader)
        {
            return {};
        }


        // Check if the physicalDevice supports the Vulkan 1.3 API version
        bool supportsVulkan1_3 = physicalDevice.getProperties().apiVersion >= vk::ApiVersion13;

        // Check if any of the queue families support graphics operations
        auto queueFamilies    = physicalDevice.getQueueFamilyProperties();
        bool supportsGraphics = std::ranges::any_of( queueFamilies, []( auto const & qfp ) { return !!( qfp.queueFlags & vk::QueueFlagBits::eGraphics ); } );

        // Check if all required physicalDevice extensions are available
        auto availableDeviceExtensions = physicalDevice.enumerateDeviceExtensionProperties();
        bool supportsAllRequiredExtensions =
        std::ranges::all_of( requiredDeviceExtension,
            [&availableDeviceExtensions]( auto const & requiredDeviceExtension )
            {
            return std::ranges::any_of( availableDeviceExtensions,
                                        [requiredDeviceExtension]( auto const & availableDeviceExtension )
                                        { return strcmp( availableDeviceExtension.extensionName, requiredDeviceExtension ) == 0; } );
            } );

        // Check if the physicalDevice supports the required features (dynamic rendering and extended dynamic state)
        auto features =
            physicalDevice
            .template getFeatures2<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan13Features, vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
        bool supportsRequiredFeatures = features.template get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
                                        features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState;


        // Return true if the physicalDevice meets all the criteria
        if(supportsVulkan1_3 && supportsGraphics && supportsAllRequiredExtensions && supportsRequiredFeatures)
            return score;
        return {};
    }
    vk::raii::PhysicalDevice pickPhysicalDevice(Instance& instance){
         auto physicalDevices = vk::raii::PhysicalDevices( instance.instance );
        if (physicalDevices.empty())
        {
            throw std::runtime_error( "failed to find GPUs with Vulkan support!" );
        }

        // Use an ordered map to automatically sort candidates by increasing score
        std::multimap<int, vk::raii::PhysicalDevice> candidates;

        for (const auto& pd : physicalDevices)
        {
            auto score = isDeviceSuitable(pd);
            if(score.has_value())   
                candidates.insert(std::make_pair(score.value(), pd));
        }

        // Check if the best candidate is suitable at all
        if (!candidates.empty() && candidates.rbegin()->first > 0)
        {
            return candidates.rbegin()->second;
        }
        else
        {
            throw std::runtime_error("failed to find a suitable GPU!");
        }
    }
    
};
