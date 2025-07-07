#pragma once
#include <vulkan/vulkan.hpp>

namespace mve {
	class ShaderModule {
	public:
		ShaderModule(VkDevice dev, const uint32_t* code, size_t size);
		~ShaderModule();

		// No copy, allow move
		ShaderModule(const ShaderModule&) = delete;
		ShaderModule& operator=(const ShaderModule&) = delete;
		ShaderModule(ShaderModule&& other) noexcept;
		ShaderModule& operator=(ShaderModule&& other) noexcept;

		VkShaderModule get() const { return module_; }
	private:
		VkDevice       device_{ VK_NULL_HANDLE };
		VkShaderModule module_{ VK_NULL_HANDLE };
	};
}