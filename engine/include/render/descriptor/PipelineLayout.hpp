#pragma once
#include <vulkan/vulkan.h>
#include <vector>

namespace mve {

    class PipelineLayout {
    public:
        PipelineLayout(
            VkDevice device,
            const std::vector<VkDescriptorSetLayout>& setLayouts,
            const std::vector<VkPushConstantRange>& pushConstants = {}
        );

        ~PipelineLayout();

        PipelineLayout(PipelineLayout&& other) noexcept;
        PipelineLayout& operator=(PipelineLayout&& other) noexcept;

        PipelineLayout(const PipelineLayout&) = delete;
        PipelineLayout& operator=(const PipelineLayout&) = delete;

        VkPipelineLayout getHandle() const noexcept { return layout_; }

    private:
        void destroy();

        VkDevice device_ = VK_NULL_HANDLE;
        VkPipelineLayout layout_ = VK_NULL_HANDLE;
    };

} // namespace mve
