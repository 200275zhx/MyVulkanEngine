#include "render/RenderPass.hpp"
#include "tool/HelpersVulkan.hpp"
#include <stdexcept>
#include <algorithm>
#include <cassert>

namespace mve {

    RenderPass::RenderPass(Device& device,
        const RenderPassCreateInfo& info,
        const std::array<uint32_t, static_cast<size_t>(AttachmentRole::Count)>& indexOf,
        VkExtent2D extent,
        const std::vector<std::vector<VkImageView>>& perFrameAttachmentViews,
        const std::vector<VkFence>& inFlightFences,
        uint32_t currentFrame)
        : device_(&device)
        , info_(info)
        , indexOf_(indexOf)
        , extent_(extent)
        , inFlightFences_(inFlightFences)
        , currentFrame_(currentFrame)
        , attachmentViews_(perFrameAttachmentViews)
    {
        createRenderPass();
        createResources(perFrameAttachmentViews);
    }

    RenderPass::~RenderPass() {
        cleanupResources();
        cleanupRenderPass();
    }

    void RenderPass::recreate(const std::vector<std::vector<VkImageView>>& perFrameAttachmentViews, VkExtent2D extent) {
        vkWaitForFences(device_->getDevice(), 1, &inFlightFences_[currentFrame_], VK_TRUE, UINT64_MAX);
        vkResetFences(device_->getDevice(), 1, &inFlightFences_[currentFrame_]);
        cleanupResources();
        cleanupRenderPass();

        attachmentViews_ = perFrameAttachmentViews;
        extent_ = extent;

        createRenderPass();
        createResources(perFrameAttachmentViews);
    }

    void RenderPass::begin(VkCommandBuffer cmd, uint32_t imageIndex,
        const std::vector<VkClearValue>& clears,
        VkSubpassContents contents) const {
        VkRenderPassBeginInfo bi{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
        bi.renderPass = renderPass_;
        bi.framebuffer = frameBuffers_[imageIndex]->getHandle();
        bi.renderArea.extent = extent_;
        bi.clearValueCount = static_cast<uint32_t>(clears.size());
        bi.pClearValues = clears.data();
        vkCmdBeginRenderPass(cmd, &bi, contents);
    }

    void RenderPass::end(VkCommandBuffer cmd) const {
        vkCmdEndRenderPass(cmd);
    }

    void RenderPass::createRenderPass() {
        std::vector<std::vector<uint32_t>> colorIdx_;
        std::vector<std::vector<uint32_t>> inputIdx_;
        std::vector<std::vector<uint32_t>> resolveIdx_;
        std::vector<std::vector<uint32_t>> preserveIdx_;
        std::vector<VkAttachmentReference> depthRefs_;
        const auto& info = info_;

        std::vector<VkAttachmentDescription> descs;
        descs.reserve(info.attachments.size());
        for (const auto& att : info.attachments)
            descs.push_back(att.desc);

        size_t subCount = info.subpasses.size();
        colorIdx_.assign(subCount, {});
        inputIdx_.assign(subCount, {});
        resolveIdx_.assign(subCount, {});
        preserveIdx_.assign(subCount, {});
        depthRefs_.assign(subCount, VkAttachmentReference{});

        std::vector<VkSubpassDescription> subs;
        subs.reserve(subCount);
        for (size_t i = 0; i < subCount; ++i) {
            const auto& sp = info.subpasses[i];
            VkSubpassDescription sd{};
            sd.pipelineBindPoint = sp.bindPoint;

            for (auto role : sp.colorAttachments) colorIdx_[i].push_back(indexOf_[static_cast<size_t>(role)]);
            sd.colorAttachmentCount = static_cast<uint32_t>(colorIdx_[i].size());
            sd.pColorAttachments = colorIdx_[i].data();

            for (auto role : sp.inputAttachments) inputIdx_[i].push_back(indexOf_[static_cast<size_t>(role)]);
            sd.inputAttachmentCount = static_cast<uint32_t>(inputIdx_[i].size());
            sd.pInputAttachments = inputIdx_[i].data();

            if (!sp.resolveAttachments.empty()) {
                for (auto role : sp.resolveAttachments) resolveIdx_[i].push_back(indexOf_[static_cast<size_t>(role)]);
                sd.pResolveAttachments = resolveIdx_[i].data();
            }

            for (auto role : sp.preserveAttachments) preserveIdx_[i].push_back(indexOf_[static_cast<size_t>(role)]);
            sd.preserveAttachmentCount = static_cast<uint32_t>(preserveIdx_[i].size());
            sd.pPreserveAttachments = preserveIdx_[i].empty() ? nullptr : preserveIdx_[i].data();

            if (sp.depthStencilAttachment != AttachmentRole::Invalid) {
                depthRefs_[i].attachment = indexOf_[static_cast<size_t>(sp.depthStencilAttachment)];
                depthRefs_[i].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                sd.pDepthStencilAttachment = &depthRefs_[i];
            }

            subs.push_back(sd);
        }

        VkRenderPassCreateInfo rpci{ VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
        rpci.attachmentCount = static_cast<uint32_t>(descs.size());
        rpci.pAttachments = descs.data();
        rpci.subpassCount = static_cast<uint32_t>(subs.size());
        rpci.pSubpasses = subs.data();
        rpci.dependencyCount = static_cast<uint32_t>(info.dependencies.size());
        rpci.pDependencies = info.dependencies.data();

        VK_CHECK(vkCreateRenderPass(device_->getDevice(), &rpci, nullptr, &renderPass_));

        device_->setObjectName(VK_OBJECT_TYPE_RENDER_PASS,
            reinterpret_cast<uint64_t>(renderPass_),
            std::string("RenderPass_main"));
    }

    void RenderPass::cleanupRenderPass() {
        if (renderPass_ != VK_NULL_HANDLE) {
            vkDestroyRenderPass(device_->getDevice(), renderPass_, nullptr);
            renderPass_ = VK_NULL_HANDLE;
        }
    }

    void RenderPass::createResources() {
        cleanupResources();
        frameBuffers_.reserve(attachmentViews_.size());

        for (size_t i = 0; i < attachmentViews_.size(); ++i) {
            const auto& views = attachmentViews_[i];
            assert(views.size() == info_.attachments.size() && "Image view count must match attachment count");

            for (VkImageView view : views) {
                assert(view != VK_NULL_HANDLE && "Invalid VkImageView detected"); // null check
            }

            auto fb = std::make_unique<Framebuffer>(device_->getDevice(), renderPass_, extent_, views);
            device_->setObjectName(VK_OBJECT_TYPE_FRAMEBUFFER,
                reinterpret_cast<uint64_t>(fb->getHandle()),
                std::string("Framebuffer_") + std::to_string(i));
            frameBuffers_.push_back(std::move(fb));
        }
    }

    void RenderPass::cleanupResources() {
        frameBuffers_.clear();
    }

    RenderPass::Builder::Builder() {
        indexOf_.fill(static_cast<uint32_t>(-1));
    }

    RenderPass::Builder& RenderPass::Builder::addAttachment(AttachmentRole role, const VkAttachmentDescription& desc) {
        size_t key = static_cast<size_t>(role);
        assert(key < indexOf_.size());
        assert(indexOf_[key] == static_cast<uint32_t>(-1));
        indexOf_[key] = static_cast<uint32_t>(info_.attachments.size());
        info_.attachments.push_back({ role, desc });
        return *this;
    }

    RenderPass::Builder& RenderPass::Builder::addSubpass(const SubpassInfo& sub) {
        info_.subpasses.push_back(sub);
        return *this;
    }

    RenderPass::Builder& RenderPass::Builder::addDependency(const VkSubpassDependency& dep) {
        info_.dependencies.push_back(dep);
        return *this;
    }

    uint32_t RenderPass::Builder::getAttachmentIndex(AttachmentRole role) const {
        return indexOf_[static_cast<size_t>(role)];
    }

    RenderPass RenderPass::Builder::build(Device& device,
        VkExtent2D extent,
        const std::vector<std::vector<VkImageView>>& perFrameAttachmentViews,
        const std::vector<VkFence>& inFlightFences,
        uint32_t currentFrame) const {
        return RenderPass(device, info_, indexOf_, extent, perFrameAttachmentViews, inFlightFences, currentFrame);
    }

} // namespace mve
