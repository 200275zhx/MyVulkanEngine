// Render/SwapChain.h
#pragma once

#include <vulkan/vulkan.h>
#include <vector>

#include "render/Device.hpp"
#include "tool/HelpersVulkan.hpp"   // for QuerySwapChainSupport
#include "render/SyncManager.hpp"

namespace mve {

    class SwapChain {
    public:
        SwapChain(Device& device,
            VkExtent2D windowExtent,
            bool vsync = false,
            bool allowTearing = false);
        ~SwapChain();

        // No copying
        SwapChain(const SwapChain&) = delete;
        SwapChain& operator=(const SwapChain&) = delete;

        // Acquire next image for rendering; waits on fences internally.
        // Returns VK_SUCCESS, VK_SUBOPTIMAL_KHR, or VK_ERROR_OUT_OF_DATE_KHR.
        VkResult acquireNextImage(uint32_t* pOutImageIndex);

        // Submit present; if swapchain is out-of-date/suboptimal, you should
        // call recreate() from your frame loop.
        VkResult presentImage(uint32_t imageIndex, uint64_t waitValue);

        // Recreate swapchain (e.g. on window resize or out-of-date)
        void recreate(VkExtent2D newExtent);

        void waitForAllInFlightFences();

        // Accessors
        VkSwapchainKHR                       getHandle()            const { return swapChain_; }
        VkFormat                             getImageFormat()       const { return imageFormat_; }
        VkExtent2D                           getExtent()            const { return extent_; }
        const std::vector<VkImageView>&      getImageViews()        const { return imageViews_; }
        uint32_t                             getImageCount()        const { return static_cast<uint32_t>(images_.size()); }
        uint32_t                             getMaxFramesInFlight() const { return maxFramesInFlight_; }

    private:
        void createSwapChain(VkSwapchainKHR oldSwapchain);
        void destroySwapChainHandle(VkSwapchainKHR handle);

        void createImageViews();
        void cleanupImageViews();

        void createSyncObjects();
        void cleanupSyncObjects();

        VkResult acquireNextImage(uint32_t& imageIndex);

        VkSurfaceFormatKHR chooseFormat(const std::vector<VkSurfaceFormatKHR>& avail) const;
        VkPresentModeKHR   choosePresentMode(const std::vector<VkPresentModeKHR>& avail) const;
        VkExtent2D         chooseExtent(const VkSurfaceCapabilitiesKHR& caps) const;

        Device&                 device_;
        VkSurfaceKHR            surface_;
        VkExtent2D              windowExtent_;
        bool                    vsync_;
        bool                    allowTearing_;
        uint32_t                maxFramesInFlight_{0};

        VkSwapchainKHR              swapChain_{ VK_NULL_HANDLE };
        std::vector<VkImage>        images_;
        std::vector<VkImageView>    imageViews_;
        VkFormat                    imageFormat_{};
        VkColorSpaceKHR             imageColorSpace_{};
        VkExtent2D                  extent_{};
        SwapChainSupportDetails     support_;  // from HelpersVulkan.hpp
        VkSurfaceFormatKHR          surfaceFormat_;

        // per-frame sync objects
        size_t                   currentFrame_ = 0;      // frame-index ring
        std::vector<VkSemaphore> imageAvailableSemaphores_;   // binary semaphores for vkAcquireNextImageKHR
        // these two semaphore and fence need to be signaled out side the class when command submission
        std::vector<VkSemaphore> renderFinishedSemaphores_;   // binary semaphores for vkQueuePresentKHR
        std::vector<VkFence>     inFlightFences_;             // host-wait fences

        // Queues
        VkQueue                graphicsQueue_;
        VkQueue                presentQueue_;
    };

} // namespace mve
