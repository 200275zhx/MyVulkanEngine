#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <array>
#include <memory>
#include "render/RenderPassBuilder.hpp"
#include "render/Framebuffer.hpp"
#include "render/Device.hpp"

namespace mve {

    enum class AttachmentRole {
        Color,
        DepthStencil,
        Resolve,
        Input,
        Preserve,
        GBufferPosition,
        GBufferNormal,
        GBufferAlbedo,
        GBufferMaterial,
        GBufferEmissive,
        MotionVector,
        SSAO,
        ShadowDepth,
        Bloom,
        PostProcess,
        UI,
        Invalid,
        Count
    };

    struct AttachmentInfo {
        AttachmentRole role;
        VkAttachmentDescription desc;
    };

    struct SubpassInfo {
        VkPipelineBindPoint bindPoint{ VK_PIPELINE_BIND_POINT_GRAPHICS };
        std::vector<AttachmentRole> colorAttachments;
        std::vector<AttachmentRole> inputAttachments;
        std::vector<AttachmentRole> resolveAttachments;
        std::vector<AttachmentRole> preserveAttachments;
        AttachmentRole depthStencilAttachment = AttachmentRole::Invalid;
    };

    struct RenderPassCreateInfo {
        std::vector<AttachmentInfo> attachments;
        std::vector<SubpassInfo> subpasses;
        std::vector<VkSubpassDependency> dependencies;
    };

    class RenderPass {
    public:
        class Builder;

        RenderPass(Device& device,
            const RenderPassCreateInfo& info,
            const std::array<uint32_t, static_cast<size_t>(AttachmentRole::Count)>& indexOf,
            VkExtent2D extent,
            const std::vector<std::vector<VkImageView>>& perFrameAttachmentViews,
            const std::vector<VkFence>& inFlightFences,
            uint32_t currentFrame);
        ~RenderPass();

        void recreate(const std::vector<std::vector<VkImageView>>& perFrameAttachmentViews, VkExtent2D extent);
        void begin(VkCommandBuffer cmd,
            uint32_t imageIndex,
            const std::vector<VkClearValue>& clears,
            VkSubpassContents contents) const;
        void end(VkCommandBuffer cmd) const;

        // getters
        VkRenderPass    getHandle() const noexcept { return renderPass_; }
        VkExtent2D      getExtent() const { return extent_; }

        const std::vector<std::unique_ptr<Framebuffer>>&            getFramebuffers()   const { return frameBuffers_; }
        const RenderPassCreateInfo&                                 getCreateInfo()     const { return info_; }
        const std::array<uint32_t, (size_t)AttachmentRole::Count>&  getIndexMap()       const { return indexOf_; }

    private:
        Device* device_;
        RenderPassCreateInfo info_;
        std::array<uint32_t, static_cast<size_t>(AttachmentRole::Count)> indexOf_;

        VkRenderPass renderPass_{ VK_NULL_HANDLE };
        VkExtent2D extent_;

        std::vector<std::vector<VkImageView>> attachmentViews_;
        std::vector<std::unique_ptr<Framebuffer>> frameBuffers_;

        std::vector<VkFence> inFlightFences_;
        uint32_t currentFrame_;

        void createRenderPass();
        void cleanupRenderPass();
        void createResources();
        void cleanupResources();
    };

    class RenderPass::Builder {
        friend class RenderPass;

    public:
        Builder();

        Builder& addAttachment(AttachmentRole role, const VkAttachmentDescription& desc);
        Builder& addSubpass(const SubpassInfo& sub);
        Builder& addDependency(const VkSubpassDependency& dep);

        uint32_t getAttachmentIndex(AttachmentRole role) const;

        const RenderPassCreateInfo& getCreateInfo() const { return info_; }
        const std::array<uint32_t, static_cast<size_t>(AttachmentRole::Count)>& getIndexMap() const { return indexOf_; }

        RenderPass build(Device& device,
            VkExtent2D extent,
            const std::vector<std::vector<VkImageView>>& perFrameAttachmentViews,
            const std::vector<VkFence>& inFlightFences,
            uint32_t currentFrame) const;

    private:
        RenderPassCreateInfo info_;
        std::array<uint32_t, static_cast<size_t>(AttachmentRole::Count)> indexOf_;
    };

} // namespace mve


