#pragma once
#include "vulkan/Header.hpp"
#include "vulkan/setup/Instance.hpp"

class ValidationLayer
{
    const std::vector<char const*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

#ifdef NDEBUG
    bool enableValidationLayers = false;
#else
    bool enableValidationLayers = true;
#endif
public:
    ValidationLayer(InstanceSettings* settings);
    ~ValidationLayer();
};

// TODO: skipped the Message Callback stuff from https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/02_Validation_layers.html