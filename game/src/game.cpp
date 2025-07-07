#define VULKAN_PHYSICAL_DEVICE_INFO_CHECK

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include "render/VulkanInstance.hpp"
#include "render/Device.hpp"
#include "render/SyncManager.hpp"
#include "render/SwapChain.hpp"

using namespace mve;

int main() {
    try {
        // --- Init GLFW & create window ---
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        GLFWwindow* window = glfwCreateWindow(800, 600, "GPU Info App", nullptr, nullptr);

        // --- Create Vulkan instance & surface ---
        VulkanInstance instance("GPU Info App", VK_MAKE_VERSION(1, 0, 0), true, {});
        VkSurfaceKHR surface;
        if (glfwCreateWindowSurface(instance.get(), window, nullptr, &surface) != VK_SUCCESS)
            throw std::runtime_error("Failed to create window surface");

        // --- Pick physical device, create logical device ---
        Device device(
            instance.get(),
            surface,
            { VK_KHR_SWAPCHAIN_EXTENSION_NAME },
            { "timelineSemaphore", "bufferDeviceAddress", "memoryPriority" }
        );

        // --- Timeline-sync for graphics-queue work only ---
        SyncManager sync(device, { Domain::GRAPHICS }); // Domain enum is now GRAPHICS, COMPUTE, TRANSFER :contentReference[oaicite:14]{index=14}

        // --- Build a 3-image swapchain + 2-in-flight (vsync on, no tearing) ---
        VkExtent2D windowSize{ 800, 600 };
        SwapChain* swapchain = new SwapChain(device, windowSize, /*vsync=*/true, /*allowTearing=*/false); // ctor signature change :contentReference[oaicite:15]{index=15}

        // --- Single acquire/present (render nothing) ---
        uint32_t imageIndex;
        VkResult result = swapchain->acquireNextImage(&imageIndex); // acquireNextImage :contentReference[oaicite:16]{index=16}
        if (result == VK_SUCCESS) {
            // Wait until our graphics timeline semaphore has reached its current value
            uint64_t graphicsPoint = sync.completedValue(Domain::GRAPHICS);
            swapchain->presentImage(imageIndex, graphicsPoint);      // presentImage :contentReference[oaicite:17]{index=17}
        }
        else if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
            swapchain->recreate(windowSize);
        }
        else {
            throw std::runtime_error("Failed to acquire swapchain image");
        }

        // --- Main loop (no further frames) ---
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
        }

        // --- Clean up ---
        vkDeviceWaitIdle(device.getDevice());
        delete swapchain;
        vkDestroySurfaceKHR(instance.get(), surface, nullptr);
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_SUCCESS;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
