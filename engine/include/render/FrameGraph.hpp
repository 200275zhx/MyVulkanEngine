#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <array>
#include <functional>
#include <memory>
#include <unordered_map>
#include "render/RenderPass.hpp"
#include "render/Device.hpp"

namespace mve {

    /// Small wrapper embedding the last layout state with each image handle
    struct FrameGraphImage {
        VkImage           handle;
        VkImageLayout     lastLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        FrameGraphImage() = default;
        FrameGraphImage(VkImage img, VkImageLayout init)
            : handle(img), lastLayout(init) {
        }
    };

    /// A single node in the frame-graph, holding its VkRenderPass, clear-values, record callback, and per-attachment image state.
    struct FrameGraphPass {
        RenderPassCreateInfo                              info;
        std::array<uint32_t, size_t(AttachmentRole::Count)> indexMap;
        std::vector<VkClearValue>                         clears;
        std::function<void(VkCommandBuffer)>              executeCb;
        std::unique_ptr<RenderPass>                       rp;
        std::vector<FrameGraphImage>                      images;  // embedded image wrappers
    };

    /// FrameGraph: manages registration, compilation, and execution of passes with embedded resource-state tracking.
    class FrameGraph {
    public:
        explicit FrameGraph(Device& device);
        ~FrameGraph();

        /// Register one pass (marking graph dirty)
        uint32_t addPass(
            const RenderPassCreateInfo& info,
            const std::array<uint32_t, size_t(AttachmentRole::Count)>& indexMap,
            std::vector<VkClearValue> clears,
            std::function<void(VkCommandBuffer)> executeCb);

        /// Begin the frame: set up extent, command buffer, and image index
        void beginFrame(VkExtent2D extent, VkCommandBuffer cmd, uint32_t imageIndex);

        /// Compile graph (build all RenderPass + Framebuffers, initialize image wrappers) if dirty
        void compile(
            const std::vector<std::vector<VkImageView>>& perFrameAttachmentViews,
            const std::vector<VkFence>& inFlightFences,
            uint32_t currentFrame);

        /// Record all passes: insert per-image barriers and execute
        void endFrame();

    private:
        Device&                                         device_;
        VkExtent2D                                      extent_{};
        VkCommandBuffer                                 cmd_ = VK_NULL_HANDLE;
        uint32_t                                        imageIndex_ = 0;
        bool                                            dirty_{ true };

        std::vector<FrameGraphPass>                     passes_;

        // Helpers
        void compilePasses(
            const std::vector<std::vector<VkImageView>>& perFrameAttachmentViews,
            const std::vector<VkFence>& inFlightFences,
            uint32_t currentFrame);
        void recordPasses();
    };

} // namespace mve