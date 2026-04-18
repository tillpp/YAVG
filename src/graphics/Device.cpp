#include "Device.hpp"


void Device::create(Instance& instance,DeviceSettings settings,std::vector<Queue*> queues){
    physicalDevice = pickPhysicalDevice(instance,settings);
    
    // LOGICLA DEVICE:
    struct QueueInfo{
        Queue*  queue;
        float priority;
    };
    std::map<unsigned int,std::vector<QueueInfo>>  queueMap;
    {
        std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();
        for (auto &&queue : queues){
            uint32_t queueIndex = ~0;
            for (uint32_t qfpIndex = 0; qfpIndex < queueFamilyProperties.size(); qfpIndex++)
            {
                if (queue->isQueueFamilySuitable(queueFamilyProperties[qfpIndex],qfpIndex,physicalDevice));
                {
                    // found a queue family that supports both graphics and present
                    queueIndex = qfpIndex;
                    break;
                }
            }
            if (queueIndex == ~0)
            {
                throw std::runtime_error("Could not find a queue for graphics and present -> terminating");
            }
            
            float queuePriority = 0.5f;
            queueMap[queueIndex].push_back((QueueInfo){
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

    // TODO: refactor features into DeviceSettings
    // Create a chain of feature structures
    vk::PhysicalDeviceFeatures2 a{};// vk::PhysicalDeviceFeatures2 (empty for now)
    vk::PhysicalDeviceVulkan13Features b{.dynamicRendering = true}; // Enable dynamic rendering from Vulkan 1.3
    vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT c{.extendedDynamicState = true };  // Enable extended dynamic state from the extension
    vk::PhysicalDeviceVulkan11Features d{.shaderDrawParameters = true};
    a.pNext = &b;
    b.pNext = &c;
    c.pNext = &d;

    vk::DeviceCreateInfo deviceCreateInfo{
        .pNext = &a,
        .queueCreateInfoCount = static_cast<uint32_t>(deviceQueueCreateInfos.size()),
        .pQueueCreateInfos = deviceQueueCreateInfos.data(),
        .enabledExtensionCount = static_cast<uint32_t>(settings.extensions.size()),
        .ppEnabledExtensionNames = settings.extensions.data()
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
std::optional<int> Device::isDeviceSuitable( vk::raii::PhysicalDevice const & physicalDevice ,DeviceSettings settings){
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
    std::ranges::all_of( settings.extensions,
        [&availableDeviceExtensions]( auto const & requiredDeviceExtension )
        {
        return std::ranges::any_of( availableDeviceExtensions,
                                    [requiredDeviceExtension]( auto const & availableDeviceExtension )
                                    { return strcmp( availableDeviceExtension.extensionName, requiredDeviceExtension ) == 0; } );
        } );

    // Check if the physicalDevice supports the required features (dynamic rendering and extended dynamic state)
    auto features =
        physicalDevice
        .template getFeatures2<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan13Features, vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT,vk::PhysicalDeviceVulkan11Features>();
    bool supportsRequiredFeatures = features.template get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
                                    features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState &&
                                    features.template get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters;

    // Return true if the physicalDevice meets all the criteria
    if(supportsVulkan1_3 && supportsGraphics && supportsAllRequiredExtensions && supportsRequiredFeatures)
        return score;
    return {};
}
vk::raii::PhysicalDevice Device::pickPhysicalDevice(Instance& instance,const DeviceSettings& settings){
     auto physicalDevices = vk::raii::PhysicalDevices( instance.instance );
    if (physicalDevices.empty())
    {
        throw std::runtime_error( "failed to find GPUs with Vulkan support!" );
    }

    // Use an ordered map to automatically sort candidates by increasing score
    std::multimap<int, vk::raii::PhysicalDevice> candidates;
    for (const auto& pd : physicalDevices)
    {
        auto score = isDeviceSuitable(pd,settings);
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