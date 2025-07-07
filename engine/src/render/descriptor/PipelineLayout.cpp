#include "render/descriptor/PipelineLayout.hpp"
#include "tool/HelpersVulkan.hpp" // for VK_CHECK

namespace mve {

    PipelineLayout::PipelineLayout(
        VkDevice device,
        const std::vector<VkDescriptorSetLayout>& setLayouts,
        const std::vector<VkPushConstantRange>& pushConstants
    )
        : device_(device)
    {
        VkPipelineLayoutCreateInfo info{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
        info.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
        info.pSetLayouts = setLayouts.data();
        info.pushConstantRangeCount = static_cast<uint32_t>(pushConstants.size());
        info.pPushConstantRanges = pushConstants.data();

        VK_CHECK(vkCreatePipelineLayout(device_, &info, nullptr, &layout_));
    }

    PipelineLayout::~PipelineLayout() {
        destroy();
    }

    PipelineLayout::PipelineLayout(PipelineLayout&& other) noexcept
        : device_(other.device_), layout_(other.layout_)
    {
        other.device_ = VK_NULL_HANDLE;
        other.layout_ = VK_NULL_HANDLE;
    }

    PipelineLayout& PipelineLayout::operator=(PipelineLayout&& other) noexcept {
        if (this != &other) {
            destroy();
            device_ = other.device_;
            layout_ = other.layout_;
            other.device_ = VK_NULL_HANDLE;
            other.layout_ = VK_NULL_HANDLE;
        }
        return *this;
    }

    void PipelineLayout::destroy() {
        if (layout_ != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(device_, layout_, nullptr);
            layout_ = VK_NULL_HANDLE;
        }
    }

} // namespace mve
