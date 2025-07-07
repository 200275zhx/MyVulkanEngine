#include "render/descriptor/DescriptorSetLayout.hpp"
#include "tool/HelpersVulkan.hpp"
#include <utility>

namespace mve {

    DescriptorSetLayout::DescriptorSetLayout(VkDevice device, const std::vector<VkDescriptorSetLayoutBinding>& bindings)
        : device_(device)
    {
        VkDescriptorSetLayoutCreateInfo info{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
        info.bindingCount = static_cast<uint32_t>(bindings.size());
        info.pBindings = bindings.data();

        VK_CHECK(vkCreateDescriptorSetLayout(device_, &info, nullptr, &layout_));
    }

    DescriptorSetLayout::~DescriptorSetLayout() {
        destroy();
    }

    DescriptorSetLayout::DescriptorSetLayout(DescriptorSetLayout&& other) noexcept
        : device_(other.device_), layout_(other.layout_)
    {
        other.layout_ = VK_NULL_HANDLE;
        other.device_ = VK_NULL_HANDLE;
    }

    DescriptorSetLayout& DescriptorSetLayout::operator=(DescriptorSetLayout&& other) noexcept {
        if (this != &other) {
            destroy();
            device_ = other.device_;
            layout_ = other.layout_;
            other.layout_ = VK_NULL_HANDLE;
            other.device_ = VK_NULL_HANDLE;
        }
        return *this;
    }

    void DescriptorSetLayout::destroy() {
        if (layout_ != VK_NULL_HANDLE) {
            vkDestroyDescriptorSetLayout(device_, layout_, nullptr);
            layout_ = VK_NULL_HANDLE;
        }
    }

} // namespace mve
