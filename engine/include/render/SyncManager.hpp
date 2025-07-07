#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <mutex>
#include <cstdint>
#include <string>
#include <stdexcept>
#include <cassert>
#include <array>

#include "tool/HelpersVulkan.hpp"   // QueryAllFeatures, AllFeatures
#include "render/Device.hpp"        // VK_CHECK, Device::setObjectName

namespace mve {
    // Defines synchronization domains
    enum class Domain : uint32_t {
        GRAPHICS,
        COMPUTE,
        TRANSFER,

        // sentinel for array sizing:
        COUNT
    };

    // Encapsulates a timeline semaphore
    struct TimelineInfo {
        VkSemaphore semaphore = VK_NULL_HANDLE;
    };

    class SyncManager {
    public:
        SyncManager(Device& device, const std::vector<Domain>& enabledDomain);
        ~SyncManager();

        // Wait host until a given domain semaphore reaches 'value'
        // This is a low-frequency operation and thread-safe
        void hostWait(Domain domain, uint64_t value, uint64_t timeout = UINT64_MAX);

        // Query the current counter value of a domain semaphore
        // Lock-free and safe to call from any thread
        uint64_t completedValue(Domain domain) const;

        // Retrieve raw VkSemaphore for use in GPU submissions
        // Lock-free and safe to call from worker threads
        VkSemaphore getSemaphore(Domain domain) const noexcept;

    private:
        Device&                   device_wrapper_;
        VkDevice                  device_;
        VkPhysicalDevice          physical_device_;
        std::array<TimelineInfo, static_cast<size_t>(Domain::COUNT)> semaphores_;
        mutable std::mutex        mutex_;  // Guards hostWait only

        // Create and name a timeline semaphore for 'domain'
        void createTimelineSemaphore(Domain domain);
    };

} // namespace mve