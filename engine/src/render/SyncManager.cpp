#include "render/SyncManager.hpp"

namespace mve {

    SyncManager::SyncManager(Device& dev, const std::vector<Domain>& enabledDomain)
        : device_wrapper_(dev)
        , device_(dev.getDevice())
        , physical_device_(dev.getPhysicalDevice())
    {
        // Verify timeline semaphore support was enabled at device creation
        AllFeatures avail = QueryAllFeatures(physical_device_);
        if (!avail.v12.timelineSemaphore) throw std::runtime_error("Timeline semaphores not supported by physical device");

        for (auto& info : semaphores_) info.semaphore = VK_NULL_HANDLE;
        for (Domain domain : enabledDomain) createTimelineSemaphore(domain);
    }

    SyncManager::~SyncManager()
    {
        if (vkDeviceWaitIdle(device_) != VK_SUCCESS) {
            std::cerr << "vkDeviceWaitIdle failed during SyncManager destruction\n";
        }

        for (auto& info : semaphores_) {
            if (info.semaphore != VK_NULL_HANDLE) vkDestroySemaphore(device_, info.semaphore, nullptr);
        }
    }

    void SyncManager::createTimelineSemaphore(Domain domain)
    {
        VkSemaphoreTypeCreateInfo timelineCreate{ VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO };
        timelineCreate.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
        timelineCreate.initialValue = 0;

        VkSemaphoreCreateInfo ci{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
        ci.pNext = &timelineCreate;

        TimelineInfo info{};
        VK_CHECK(vkCreateSemaphore(device_, &ci, nullptr, &info.semaphore));

        // Assign debug name for GPU profilers
        device_wrapper_.setObjectName(
            VK_OBJECT_TYPE_SEMAPHORE,
            reinterpret_cast<uint64_t>(info.semaphore),
            std::string("TimelineSemaphore_") + std::to_string(static_cast<uint32_t>(domain))
        );

        semaphores_[static_cast<size_t>(domain)] = info;
    }

    void SyncManager::hostWait(Domain domain, uint64_t value, uint64_t timeout)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        VkSemaphoreWaitInfo wi{ VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO };
        wi.semaphoreCount = 1;
        wi.pSemaphores = &semaphores_[static_cast<size_t>(domain)].semaphore;
        wi.pValues = &value;

        VkResult r = vkWaitSemaphores(device_, &wi, timeout);
        if (r == VK_TIMEOUT) {
            return; // caller can handle timeout
        }
        VK_CHECK(r);
    }

    uint64_t SyncManager::completedValue(Domain domain) const
    {
        uint64_t v = 0;
        VK_CHECK(vkGetSemaphoreCounterValue(
            device_,
            semaphores_[static_cast<size_t>(domain)].semaphore,
            &v
        ));
        return v;
    }

    VkSemaphore SyncManager::getSemaphore(Domain domain) const noexcept
    {
        // No lock needed: semaphores_ is immutable after construction
        return semaphores_[static_cast<size_t>(domain)].semaphore;
    }

} // namespace mve
