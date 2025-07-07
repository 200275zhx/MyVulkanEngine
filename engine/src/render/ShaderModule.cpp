#include "render/ShaderModule.hpp"
#include "tool/HelpersVulkan.hpp"
#include <cassert>

namespace mve {
	ShaderModule::ShaderModule(VkDevice dev, const uint32_t* spirv, size_t size) : device_{ dev } {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = size;
        createInfo.pCode = spirv;

        VK_CHECK(vkCreateShaderModule(device_, &createInfo, nullptr, &module_));
	}

	ShaderModule::~ShaderModule() {
        if (module_ != VK_NULL_HANDLE) {
            vkDestroyShaderModule(device_, module_, nullptr);
        }
	}

    ShaderModule::ShaderModule(ShaderModule&& other) noexcept
        : device_{ other.device_ }
        , module_{ other.module_ }
    {
        other.module_ = VK_NULL_HANDLE;
    }

    // Move assignment
    ShaderModule& ShaderModule::operator=(ShaderModule&& other) noexcept {
        if (this != &other) {
            // Clean up any existing module
            if (module_ != VK_NULL_HANDLE) {
                vkDestroyShaderModule(device_, module_, nullptr);
            }
            // Steal state
            device_ = other.device_;
            module_ = other.module_;
            // Leave other in null state
            other.module_ = VK_NULL_HANDLE;
        }
        return *this;
    }
} // namespace mve