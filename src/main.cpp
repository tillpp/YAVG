#include <vulkan/vulkan_raii.hpp>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

#include "GameFolder.hpp"

const uint32_t WIDTH  = 640;
const uint32_t HEIGHT = 720;


int main() {

    GameFolder gf;

    GLFWwindow *window = nullptr;

    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window = glfwCreateWindow(WIDTH, HEIGHT, "YAVG", nullptr, nullptr);

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
	}
    glfwDestroyWindow(window);

	glfwTerminate();
    return 0;
}