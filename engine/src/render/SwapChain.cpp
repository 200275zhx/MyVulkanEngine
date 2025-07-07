// Render/SwapChain.cpp
#include "render/SwapChain.hpp"
#include <stdexcept>
#include <algorithm>

namespace mve {

    SwapChain::SwapChain(Device& device,
        VkExtent2D windowExtent,
        bool vsync,
        bool allowTearing)
        : device_(device)
        , surface_(device.getSurface())
        , windowExtent_(windowExtent)
        , vsync_(vsync)
        , allowTearing_(allowTearing)
        , graphicsQueue_(device.getGraphicsQueue())
        , presentQueue_(device.getPresentQueue())
    {
        // Query support
        support_ = QuerySwapChainSupport(device_.getPhysicalDevice(), surface_);
        createSwapChain(VK_NULL_HANDLE);
        createImageViews();
        createSyncObjects();
    }

    SwapChain::~SwapChain() {
        waitForAllInFlightFences();
        cleanupImageViews();
        cleanupSyncObjects();
        destroySwapChainHandle(swapChain_);
    }

    void SwapChain::createSwapChain(VkSwapchainKHR oldSwapchain) {
        auto& caps = support_.capabilities;
        auto& formats = support_.formats;
        auto& modes = support_.presentModes;

        VkSurfaceFormatKHR surfaceFmt = chooseFormat(formats);
        VkPresentModeKHR   presentMd = choosePresentMode(modes);
        VkExtent2D         ext = chooseExtent(caps);

        const uint32_t DEFAULT_FRAMES_IN_FLIGHT = 2;
        const uint32_t DESIRED_IMAGE_COUNT = 3;

        // pick buffer count
        uint32_t imageCount = DESIRED_IMAGE_COUNT;                                      // triple buffer
        imageCount = std::max(imageCount, caps.minImageCount);
        if (caps.maxImageCount > 0) imageCount = std::min(imageCount, caps.maxImageCount);

        VkSwapchainCreateInfoKHR info{};
        info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        info.surface = surface_;
        info.minImageCount = imageCount;
        info.imageFormat = surfaceFmt.format;
        info.imageColorSpace = surfaceFmt.colorSpace;
        info.imageExtent = ext;
        info.imageArrayLayers = 1;
        info.imageUsage =   VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                            VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                            VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

        // Always exclusive mode
        info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        info.queueFamilyIndexCount = 0;
        info.pQueueFamilyIndices = nullptr;

        info.preTransform = caps.currentTransform;
        info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        info.presentMode = presentMd;
        info.clipped = VK_TRUE;
        info.oldSwapchain = oldSwapchain;

        VK_CHECK(vkCreateSwapchainKHR(device_.getDevice(), &info, nullptr, &swapChain_));

        // Retrieve images
        vkGetSwapchainImagesKHR(device_.getDevice(), swapChain_, &imageCount, nullptr);
        maxFramesInFlight_ = DEFAULT_FRAMES_IN_FLIGHT;
        images_.resize(imageCount);
        vkGetSwapchainImagesKHR(device_.getDevice(), swapChain_, &imageCount, images_.data());
        imageFormat_ = surfaceFmt.format;
        imageColorSpace_ = surfaceFmt.colorSpace;
        extent_ = ext;
    }

    void SwapChain::waitForAllInFlightFences() {
        for (auto fence : inFlightFences_) vkWaitForFences(device_.getDevice(), 1, &fence, VK_TRUE, UINT64_MAX);
    }

    void SwapChain::destroySwapChainHandle(VkSwapchainKHR handle) {
        if (handle != VK_NULL_HANDLE) vkDestroySwapchainKHR(device_.getDevice(), handle, nullptr);
    }

    void SwapChain::createImageViews() {
        imageViews_.resize(images_.size());
        for (size_t i = 0; i < images_.size(); i++) {
            VkImageViewCreateInfo vi{};
            vi.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            vi.image = images_[i];
            vi.viewType = VK_IMAGE_VIEW_TYPE_2D;
            vi.format = imageFormat_;
            vi.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                     VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
            vi.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

            VK_CHECK(vkCreateImageView(device_.getDevice(), &vi, nullptr, &imageViews_[i]));
        }
    }

    void SwapChain::cleanupImageViews() {
        for (auto view : imageViews_)
            vkDestroyImageView(device_.getDevice(), view, nullptr);
        imageViews_.clear();
    }

    void SwapChain::createSyncObjects() {
        imageAvailableSemaphores_.resize(maxFramesInFlight_);
        renderFinishedSemaphores_.resize(maxFramesInFlight_);
        inFlightFences_      .resize(maxFramesInFlight_);
    
        VkSemaphoreCreateInfo semInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
        VkFenceCreateInfo     fenceInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // so first wait returns immediately
        
        for (uint32_t i = 0; i < maxFramesInFlight_; i++) {
            if (vkCreateSemaphore(device_.getDevice(), &semInfo, nullptr, &imageAvailableSemaphores_[i]) != VK_SUCCESS ||
                vkCreateSemaphore(device_.getDevice(), &semInfo, nullptr, &renderFinishedSemaphores_[i]) != VK_SUCCESS ||
                vkCreateFence    (device_.getDevice(), &fenceInfo, nullptr, &inFlightFences_[i])       != VK_SUCCESS) {
                throw std::runtime_error("failed to create swapchain sync objects");
            }
        }
    }

    void SwapChain::cleanupSyncObjects() {
        for (auto& sem : imageAvailableSemaphores_)
            vkDestroySemaphore(device_.getDevice(), sem, nullptr);
        for (auto& sem : renderFinishedSemaphores_)
            vkDestroySemaphore(device_.getDevice(), sem, nullptr);
        for (auto& f   : inFlightFences_)
            vkDestroyFence    (device_.getDevice(), f,   nullptr);
    
        imageAvailableSemaphores_.clear();
        renderFinishedSemaphores_.clear();
        inFlightFences_.clear();
    }

    // private method
    VkResult SwapChain::acquireNextImage(uint32_t& imageIndex) {
        // wait on our per-frame fence, then reset it
        vkWaitForFences (device_.getDevice(), 1, &inFlightFences_[currentFrame_], VK_TRUE, UINT64_MAX);
        vkResetFences   (device_.getDevice(), 1, &inFlightFences_[currentFrame_]);
    
        return vkAcquireNextImageKHR(
            device_.getDevice(),
            swapChain_,
            UINT64_MAX,
            imageAvailableSemaphores_[currentFrame_], // signal image available binary semaphore from this frame
            VK_NULL_HANDLE,
            &imageIndex
        );
    }

    // public wrapper
    VkResult SwapChain::acquireNextImage(uint32_t* pOutImageIndex) {
        assert(pOutImageIndex);
        return acquireNextImage(*pOutImageIndex);
    }

    VkResult SwapChain::presentImage(uint32_t imageIndex, uint64_t waitValue) {
        // Supply a pNext chain for timeline-semaphores
        VkTimelineSemaphoreSubmitInfo timelineInfo{};
        timelineInfo.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
        timelineInfo.waitSemaphoreValueCount = 1;
        timelineInfo.pWaitSemaphoreValues = &waitValue;

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        // tell the queue to wait on our render-finished semaphore
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &renderFinishedSemaphores_[currentFrame_];
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &swapChain_;
        presentInfo.pImageIndices = &imageIndex;

        VkResult result = vkQueuePresentKHR(presentQueue_, &presentInfo);
        currentFrame_ = (currentFrame_ + 1) % maxFramesInFlight_;

        return result;
    }

    void SwapChain::recreate(VkExtent2D newExtent) {
        windowExtent_ = newExtent;

        // 1) Host-wait for prev present works:
        waitForAllInFlightFences();

        // 2) Tear down old views
        cleanupImageViews();
        cleanupSyncObjects();

        // 3) Rebuild swapchain & views
        VkSwapchainKHR old = swapChain_;
        support_ = QuerySwapChainSupport(device_.getPhysicalDevice(), surface_);
        createSwapChain(old);
        createImageViews();
        createSyncObjects();
        currentFrame_ = 0;

        // 4) Destroy the old swapchain handle
        destroySwapChainHandle(old);
    }

    VkSurfaceFormatKHR SwapChain::chooseFormat(const std::vector<VkSurfaceFormatKHR>& avail) const {
        // 1) PQ (HDR10) first
        for (auto& f : avail) {
            if (f.format == VK_FORMAT_A2B10G10R10_UNORM_PACK32 &&
                f.colorSpace == VK_COLOR_SPACE_HDR10_ST2084_EXT)
                return f;
        }
        // 2) Display-P3 (SDR wide gamut) next
        for (auto& f : avail) {
            if (f.format == VK_FORMAT_A2B10G10R10_UNORM_PACK32 &&
                f.colorSpace == VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT)
                return f;
        }
        // 3) sRGB fallback
        for (auto& f : avail) {
            if (f.format == VK_FORMAT_B8G8R8A8_SRGB &&
                f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                return f;
        }
        // 4) final fallback to whatever’s first
        return avail.front();
    }

    VkPresentModeKHR SwapChain::choosePresentMode(const std::vector<VkPresentModeKHR>& avail) const {
        // Check support
        bool mailboxSupported = std::find(avail.begin(), avail.end(),
            VK_PRESENT_MODE_MAILBOX_KHR) != avail.end();
        bool immediateSupported = std::find(avail.begin(), avail.end(),
            VK_PRESENT_MODE_IMMEDIATE_KHR) != avail.end();

        if (!vsync_) {
            // 1) No V-Sync -> prefer MAILBOX if available
            if (mailboxSupported) {
                std::cout << "PresentMode: Mailbox\n";
                return VK_PRESENT_MODE_MAILBOX_KHR;
            }
            // 2) If tearing is explicitly allowed, fall back to IMMEDIATE
            if (allowTearing_ && immediateSupported) {
                std::cout << "PresentMode: Immediate\n";
                return VK_PRESENT_MODE_IMMEDIATE_KHR;
            }
        }

        // 3) V-Sync on, or no better mode -> FIFO (mandatory, tear-free)
        std::cout << "PresentMode: FIFO\n";
        return VK_PRESENT_MODE_FIFO_KHR;
    }


    VkExtent2D SwapChain::chooseExtent(const VkSurfaceCapabilitiesKHR& caps) const {
        if (caps.currentExtent.width != UINT32_MAX) {
            return caps.currentExtent;
        }
        else {
            VkExtent2D actual = windowExtent_;
            actual.width = std::clamp(actual.width, caps.minImageExtent.width, caps.maxImageExtent.width);
            actual.height = std::clamp(actual.height, caps.minImageExtent.height, caps.maxImageExtent.height);
            return actual;
        }
    }

} // namespace mve
