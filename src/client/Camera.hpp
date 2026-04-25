#pragma once
#include "vulkan/Header.hpp"
#include "vulkan/Window.hpp"

class Camera
{
public:
    glm::vec3 pos = glm::vec3(1.0f, 5.0f, 5.0f);
    glm::vec3 forward = glm::vec3(0.0f, -1.0f, -1.0f);
    glm::vec3 right = glm::vec3(-1.0f, -1.0f, 0.0f);
    
    void update(Window& window, float delta);
};

