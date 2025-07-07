#pragma once

#include "render/RenderPass.hpp"
#include "render/pipeline/GraphicsPipeline.hpp"
#include "render/CommandBuffer.hpp"
#include <vector>

namespace mve {

    class GBufferPass {
    public:
        GBufferPass(Device& device,
            VkExtent2D extent,
            const std::vector<std::vector<VkImageView>>& perFrameAttachmentViews,
            const std::vector<VkFence>& inFlightFences,
            uint32_t currentFrame,
            VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT);

        ~GBufferPass();

        /// Recreate the underlying RenderPass + pipeline (e.g. on resize)
        void recreate(VkExtent2D extent,
            const std::vector<std::vector<VkImageView>>& perFrameAttachmentViews);

        /// Record the deferred-G-Buffer subpass
        void begin(VkCommandBuffer cmd, uint32_t imageIndex);
        void end(VkCommandBuffer cmd);

        // Expose the RenderPass layout info for higher-level systems
        const RenderPassCreateInfo& getCreateInfo()  const { return renderPass_.getCreateInfo(); }
        const std::array<uint32_t, static_cast<size_t>(AttachmentRole::Count)>&
            getIndexMap()    const { return renderPass_.getIndexMap(); }
        const std::vector<VkClearValue>& getClearValues() const { return clearValues_; }

    private:
        Device* device_;
        RenderPass          renderPass_;
        std::vector<VkClearValue>         clearValues_;

        uint32_t            posAtt_, normAtt_, albedoAtt_, depthAtt_;
        VkSampleCountFlagBits              msaaSamples_;
        VkPipelineLayout                   pipelineLayout_;
        GraphicsPipeline                   pipeline_;

        void buildPipeline();
    };

} // namespace mve
