#include <vulkan/vulkan.hpp>
#include <iostream>
#include <vector>

int main() {
    auto extensions = vk::enumerateInstanceExtensionProperties();
    for (const auto& ext : extensions) {
        std::cout << "\t" << ext.extensionName << " version: " << ext.specVersion << "\n";
    }
    return 0;
}
