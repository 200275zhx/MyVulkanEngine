#include "tool/HelpersGLFW.hpp"
#include <iostream>

GLFWwindow* InitWindow(const char* windowName, uint32_t width, uint32_t height) {
	// Error Callback
	glfwSetErrorCallback([](int error, const char* description) {
		std::cout << "GLFW Error (" << error << "): " << description << "\n";
		});
	if (!glfwInit()) return nullptr;

	// Set Window Attrib
	const bool IS_FULL_SCREEN = !width || !height;
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, IS_FULL_SCREEN ? GLFW_FALSE : GLFW_TRUE);
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	int x = 0;
	int y = 0;
	int w = mode->width;
	int h = mode->height;
	if (IS_FULL_SCREEN) { glfwGetMonitorWorkarea(monitor, &x, &y, &w, &h); }
	else {
		w = width;
		h = height;
	}

	// Create Window
	GLFWwindow* window = glfwCreateWindow(w, h, windowName, nullptr, nullptr);
	if (!window) {
		glfwTerminate();
		return nullptr;
	}
	if (IS_FULL_SCREEN) glfwSetWindowPos(window, x, y);

	// Escape Key Close Window
	glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int, int action, int) {
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
			glfwSetWindowShouldClose(window, GLFW_TRUE);
		}
		});

	return window;
}