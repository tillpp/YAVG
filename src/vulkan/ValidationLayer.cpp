#include "ValidationLayer.hpp"
#include <ranges>

ValidationLayer::ValidationLayer(InstanceSettings* settings)
{
#ifdef WIN32 //fck windows
    enableValidationLayers = false;
#endif
    if (enableValidationLayers)
    {
        settings->layers.insert(settings->layers.end(),validationLayers.begin(), validationLayers.end());
        
        // check support
        vk::raii::Context  context;
        auto layerProperties = context.enumerateInstanceLayerProperties();
        auto unsupportedLayerIt = std::ranges::find_if(validationLayers,
            [&layerProperties](auto const &requiredLayer) {
            return std::ranges::none_of(layerProperties,
            [requiredLayer](auto const &layerProperty) { return strcmp(layerProperty.layerName, requiredLayer) == 0; });
        });
        if (unsupportedLayerIt != validationLayers.end())
        {
            throw std::runtime_error("Required layer not supported: " + std::string(*unsupportedLayerIt));
        }
    }

}

ValidationLayer::~ValidationLayer()
{
}
