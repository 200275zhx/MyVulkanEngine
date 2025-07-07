#include "render/FrameGraph.hpp"
#include "tool/HelpersVulkan.hpp"
#include <stdexcept>

namespace mve {

    FrameGraph::FrameGraph(Device& device)
        : device_(device)
    {
    }

    FrameGraph::~FrameGraph() = default;

    uint32_t FrameGraph::addPass(
        const RenderPassCreateInfo& info,
        const std::array<uint32_t, size_t(AttachmentRole::Count)>& indexMap,
        std::vector<VkClearValue> clears,
        std::function<void(VkCommandBuffer)> executeCb)
    {
        dirty_ = true;
        passes_.push_back({ info, indexMap, std::move(clears), std::move(executeCb), nullptr, {} });
        return static_cast<uint32_t>(passes_.size() - 1);
    }

    void FrameGraph::beginFrame(VkExtent2D extent, VkCommandBuffer cmd, uint32_t imageIndex)
    {
        extent_ = extent;
        cmd_ = cmd;
        imageIndex_ = imageIndex;
    }

    void FrameGraph::compile(
        const std::vector<std::vector<VkImageView>>& perFrameAttachmentViews,
        const std::vector<VkFence>& inFlightFences,
        uint32_t currentFrame)
    {
        if (!dirty_) return;
        compilePasses(perFrameAttachmentViews, inFlightFences, currentFrame);
        dirty_ = false;
    }

    void FrameGraph::compilePasses(
        const std::vector<std::vector<VkImageView>>& perFrameAttachmentViews,
        const std::vector<VkFence>& inFlightFences,
        uint32_t currentFrame)
    {
        for (auto& P : passes_) {
            // Build Vulkan RenderPass + Framebuffers
            P.rp = std::make_unique<RenderPass>(
                device_,
                P.info,
                P.indexMap,
                extent_,
                perFrameAttachmentViews,
                inFlightFences,
                currentFrame);

            // Initialize image wrappers for each attachment
            P.images.clear();
            for (size_t a = 0; a < P.info.attachments.size(); ++a) {
                VkImage img = P.rp->getFramebuffer(imageIndex_)->getImageHandle(a);
                P.images.emplace_back(img, P.info.attachments[a].desc.initialLayout);
            }
        }
    }

    void FrameGraph::endFrame()
    {
        recordPasses();
    }

    void FrameGraph::recordPasses()
    {
        if (passes_.empty() || cmd_ == VK_NULL_HANDLE)
            return;

        // First pass: just begin
        auto& first = passes_[0];
        first.rp->begin(cmd_, imageIndex_, first.clears, VK_SUBPASS_CONTENTS_INLINE);
        first.executeCb(cmd_);
        first.rp->end(cmd_);
        // Update each image's lastLayout
        for (size_t a = 0; a < first.images.size(); ++a) {
            first.images[a].lastLayout = first.info.attachments[a].desc.finalLayout;
        }

        // Subsequent passes
        for (size_t i = 1; i < passes_.size(); ++i) {
            auto& prev = passes_[i - 1];
            auto& next = passes_[i];

            // Insert barriers only where needed per image
            for (size_t a = 0; a < next.images.size(); ++a) {
                FrameGraphImage& imgRes = next.images[a];
                VkImageLayout required = next.info.attachments[a].desc.initialLayout;
                if (imgRes.lastLayout != required) {
                    VkImageMemoryBarrier barrier{};
                    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                    barrier.srcAccessMask = HelpersVulkan::accessMaskForLayout(imgRes.lastLayout);
                    barrier.dstAccessMask = HelpersVulkan::accessMaskForLayout(required);
                    barrier.oldLayout = imgRes.lastLayout;
                    barrier.newLayout = required;
                    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                    barrier.image = imgRes.handle;
                    barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

                    vkCmdPipelineBarrier(
                        cmd_,
                        HelpersVulkan::pipelineStageForLayout(barrier.oldLayout),
                        HelpersVulkan::pipelineStageForLayout(barrier.newLayout),
                        0, 0, nullptr,
                        0, nullptr,
                        1, &barrier);
                }
            }

            // Record pass
            next.rp->begin(cmd_, imageIndex_, next.clears, VK_SUBPASS_CONTENTS_INLINE);
            next.executeCb(cmd_);
            next.rp->end(cmd_);
            // Update layouts
            for (size_t a = 0; a < next.images.size(); ++a) {
                next.images[a].lastLayout = next.info.attachments[a].desc.finalLayout;
            }
        }
    }

} // namespace mve
