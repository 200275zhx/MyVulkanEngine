#pragma once
#include <vulkan/vulkan.h>
#include <vector>

namespace mve {

    class DescriptorSetLayout {
    public:
        DescriptorSetLayout(VkDevice device, const std::vector<VkDescriptorSetLayoutBinding>& bindings);
        ~DescriptorSetLayout();

        DescriptorSetLayout(DescriptorSetLayout&& other) noexcept;
        DescriptorSetLayout& operator=(DescriptorSetLayout&& other) noexcept;

        DescriptorSetLayout(const DescriptorSetLayout&) = delete;
        DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;

        VkDescriptorSetLayout getHandle() const noexcept { return layout_; }

    private:
        void destroy();

        VkDevice device_ = VK_NULL_HANDLE;
        VkDescriptorSetLayout layout_ = VK_NULL_HANDLE;
    };

} // namespace mve
