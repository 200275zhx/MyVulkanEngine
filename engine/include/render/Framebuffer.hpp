#pragma once

#include <vulkan/vulkan.h>
#include <vector>

namespace mve {

    class Framebuffer {
    public:
        Framebuffer(VkDevice device,
            VkRenderPass renderPass,
            VkExtent2D extent,
            const std::vector<VkImageView>& attachments);
        ~Framebuffer();

        // Disable copy, enable moving
        Framebuffer(const Framebuffer&) = delete;
        Framebuffer& operator=(const Framebuffer&) = delete;
        Framebuffer(Framebuffer&& other) noexcept;
        Framebuffer& operator=(Framebuffer&& other) noexcept;

        // Accessors
        VkFramebuffer getHandle() const noexcept { return framebuffer_; }
        VkExtent2D    getExtent() const noexcept { return extent_; }
        size_t        getAttachmentCount() const noexcept { return attachments_.size(); }
        const std::vector<VkImageView>& getAttachments() const noexcept { return attachments_; }

    private:
        VkDevice              device_{ VK_NULL_HANDLE };
        VkRenderPass          renderPass_{ VK_NULL_HANDLE };
        VkExtent2D            extent_{};
        VkFramebuffer         framebuffer_{ VK_NULL_HANDLE };
        std::vector<VkImageView> attachments_;
    };

} // namespace mve
