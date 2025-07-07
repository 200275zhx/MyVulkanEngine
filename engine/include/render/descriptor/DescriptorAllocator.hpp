#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>

namespace mve {

    class DescriptorAllocator {
    public:
        explicit DescriptorAllocator(VkDevice device);
        ~DescriptorAllocator();

        DescriptorAllocator(const DescriptorAllocator&) = delete;
        DescriptorAllocator& operator=(const DescriptorAllocator&) = delete;

        void setPoolSizes(const std::vector<VkDescriptorPoolSize>& sizes);
        void reset();

        // Allocates a set from the current pool; grows if full
        VkResult allocate(VkDescriptorSetLayout layout, VkDescriptorSet& outSet);

    private:
        VkDevice device_;
        std::vector<VkDescriptorPoolSize> poolSizes_;
        std::vector<VkDescriptorPool> usedPools_;
        std::vector<VkDescriptorPool> freePools_;
        VkDescriptorPool currentPool_ = VK_NULL_HANDLE;

        VkDescriptorPool createPool(uint32_t maxSets) const;
        VkDescriptorPool grabPool();
        void destroyAll();
    };

} // namespace mve
