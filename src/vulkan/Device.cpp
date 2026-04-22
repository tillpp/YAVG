#include "Device.hpp"
#include "vulkan/Queue.hpp"

void Device::create(Instance& instance,DeviceSettings settings,const DeviceFeatures& features){
    physicalDevice = pickPhysicalDevice(instance,settings,features);
    initLogicalDevice(instance,settings,features);
}
void Device::initLogicalDevice(Instance& instance,const DeviceSettings& settings,const DeviceFeatures& features){
    // LOGICAL DEVICE:
    struct QueueInfo{
        Queue*  queue;
        float priority;
    };
    // turn Queues into vk::DeviceQueueCreateInfo  
    // queueFamilyIndex -> QueueInfo[]
    std::vector<vk::DeviceQueueCreateInfo> deviceQueueCreateInfos;
    std::map<unsigned int,std::vector<QueueInfo>>  queueMap; 
    auto createQueueMap = [&](){
        std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();
        for (auto &&queue : settings.queues){
            uint32_t queueIndex = ~0;
            for (uint32_t qfpIndex = 0; qfpIndex < queueFamilyProperties.size(); qfpIndex++)
            {
                if (queue->isQueueFamilySuitable(queueFamilyProperties[qfpIndex],qfpIndex,physicalDevice));
                {
                    // found a queue family that supports both graphics and present
                    queueIndex = qfpIndex;
                    queue->queueFamilyIndex = qfpIndex;
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
    };
    auto useQueueMap = [&](){
        for(auto&& pair: queueMap){
            auto familyIndex = pair.first;
            
            for (size_t i = 0; i < pair.second.size(); i++)
            {
                (vk::raii::Queue&)(*pair.second[i].queue) = vk::raii::Queue( device, familyIndex, i );
            }
            
        }
    };
    

    createQueueMap();

    vk::DeviceCreateInfo deviceCreateInfo{
        .pNext = features.logicalDeviceFeatures,
        .queueCreateInfoCount = static_cast<uint32_t>(deviceQueueCreateInfos.size()),
        .pQueueCreateInfos = deviceQueueCreateInfos.data(),
        .enabledExtensionCount = static_cast<uint32_t>(settings.extensions.size()),
        .ppEnabledExtensionNames = settings.extensions.data()
    }; 
    device = vk::raii::Device( physicalDevice, deviceCreateInfo );

    useQueueMap();
}


std::optional<int> Device::isDeviceSuitable( vk::raii::PhysicalDevice const & physicalDevice ,DeviceSettings settings,const DeviceFeatures& features){
    auto deviceProperties = physicalDevice.getProperties();
    auto deviceFeatures = physicalDevice.getFeatures();
    uint32_t score = 0;

    // Discrete GPUs have a significant performance advantage
    if (deviceProperties.deviceType == vk::PhysicalDeviceType::eCpu) {
        return 0; // worst possible alternative
    }else if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
        score += 1e9;
    }else if (deviceProperties.deviceType == vk::PhysicalDeviceType::eIntegratedGpu) {
        score += 1e3;
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

    bool supportsRequiredFeatures = features.physicalDeviceSuitable(physicalDevice);

    // Return true if the physicalDevice meets all the criteria
    if(supportsVulkan1_3 && supportsGraphics && supportsAllRequiredExtensions && supportsRequiredFeatures)
        return score;
    return {};
}
vk::raii::PhysicalDevice Device::pickPhysicalDevice(Instance& instance,const DeviceSettings& settings,const DeviceFeatures& features){
     auto physicalDevices = vk::raii::PhysicalDevices( instance );
    if (physicalDevices.empty())
    {
        throw std::runtime_error( "failed to find GPUs with Vulkan support!" );
    }

    // Use an ordered map to automatically sort candidates by increasing score
    std::multimap<int, vk::raii::PhysicalDevice> candidates;
    for (const auto& pd : physicalDevices)
    {
        auto score = isDeviceSuitable(pd,settings, features);
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