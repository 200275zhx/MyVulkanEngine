#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include <vector>
#include <string>
#include <stdexcept>
#include <iostream>
#include <set>

namespace mve {

    static constexpr char const* ENGINE_NAME = "MyVulkanEngine";
    static constexpr uint32_t    ENGINE_VERSION = VK_MAKE_VERSION(1, 0, 0);

    class VulkanInstance {
    public:
        VulkanInstance(const std::string& appName = "VulkanApp", uint32_t appVersion = VK_MAKE_VERSION(1, 0, 0), bool enableValidation = true, const std::vector<const char*>& extensions = {});
        ~VulkanInstance();

        // No copy
        VulkanInstance(const VulkanInstance&) = delete;
        VulkanInstance& operator=(const VulkanInstance&) = delete;

        VkInstance get() const { return instance_; }

    private:
        void createInstance(const std::string& appName, uint32_t appVersion);
        bool checkValidationLayerSupport();
        std::vector<const char*> getRequiredExtensions(); // Get glfw extensions and debug utils extension
        void setupDebugMessenger();

        static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
        static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData);
        static VkResult CreateDebugUtilsMessengerEXT(
            VkInstance instance,
            const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
            const VkAllocationCallbacks* pAllocator,
            VkDebugUtilsMessengerEXT* pDebugMessenger);
        static void DestroyDebugUtilsMessengerEXT(
            VkInstance instance,
            VkDebugUtilsMessengerEXT debugMessenger,
            const VkAllocationCallbacks* pAllocator);

        VkInstance instance_{ VK_NULL_HANDLE };
        VkDebugUtilsMessengerEXT debugMessenger_{ VK_NULL_HANDLE };
        bool validationEnabled_;
        std::vector<const char*> validationLayers_;
        std::vector<const char*> extensions_{ "VK_KHR_surface", "VK_EXT_swapchain_colorspace" };
    };
}