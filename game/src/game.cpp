#include <iostream>
#include "core/Application.hpp"
#include "tool/HelpersGLFW.hpp"

int main() {
	mve::App::print("Starting engine ...");

	constexpr uint32_t WIDTH = 1280;
	constexpr uint32_t HEIGHT = 800;

	GLFWwindow* window = InitWindow("GLFW window test", WIDTH, HEIGHT);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
	}
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}