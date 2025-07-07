#include "render/Framebuffer.hpp"
#include "tool/HelpersVulkan.hpp"   // for VK_CHECK
#include <utility>

namespace mve {

    Framebuffer::Framebuffer(VkDevice device,
        VkRenderPass renderPass,
        VkExtent2D extent,
        const std::vector<VkImageView>& attachments)
        : device_(device),
        renderPass_(renderPass),
        extent_(extent),
        attachments_(attachments)
    {
        VkFramebufferCreateInfo fci{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
        fci.renderPass = renderPass_;
        fci.attachmentCount = static_cast<uint32_t>(attachments_.size());
        fci.pAttachments = attachments_.data();
        fci.width = extent_.width;
        fci.height = extent_.height;
        fci.layers = 1;

        VK_CHECK(vkCreateFramebuffer(device_, &fci, nullptr, &framebuffer_));
    }

    Framebuffer::Framebuffer(Framebuffer&& other) noexcept
        : device_(other.device_),
        renderPass_(other.renderPass_),
        extent_(other.extent_),
        framebuffer_(other.framebuffer_),
        attachments_(std::move(other.attachments_))
    {
        // Leave other in a harmless state
        other.framebuffer_ = VK_NULL_HANDLE;
        other.device_ = VK_NULL_HANDLE;
        other.renderPass_ = VK_NULL_HANDLE;
        other.extent_ = { 0, 0 };
    }

    Framebuffer& Framebuffer::operator=(Framebuffer&& other) noexcept
    {
        if (this != &other) {
            // Destroy our current handle
            if (framebuffer_ != VK_NULL_HANDLE) {
                vkDestroyFramebuffer(device_, framebuffer_, nullptr);
            }

            // Steal resources
            device_ = other.device_;
            renderPass_ = other.renderPass_;
            extent_ = other.extent_;
            framebuffer_ = other.framebuffer_;
            attachments_ = std::move(other.attachments_);

            // Leave other in a harmless state
            other.framebuffer_ = VK_NULL_HANDLE;
            other.device_ = VK_NULL_HANDLE;
            other.renderPass_ = VK_NULL_HANDLE;
            other.extent_ = { 0, 0 };
        }
        return *this;
    }

    Framebuffer::~Framebuffer()
    {
        if (framebuffer_ != VK_NULL_HANDLE) {
            vkDestroyFramebuffer(device_, framebuffer_, nullptr);
        }
    }

} // namespace mve