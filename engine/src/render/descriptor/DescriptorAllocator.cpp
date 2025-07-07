#include "render/descriptor/DescriptorAllocator.hpp"
#include "tool/HelpersVulkan.hpp"

namespace mve {

    DescriptorAllocator::DescriptorAllocator(VkDevice device)
        : device_(device) {
    }

    DescriptorAllocator::~DescriptorAllocator() {
        destroyAll();
    }

    void DescriptorAllocator::setPoolSizes(const std::vector<VkDescriptorPoolSize>& sizes) {
        poolSizes_ = sizes;
    }

    VkDescriptorPool DescriptorAllocator::createPool(uint32_t maxSets) const {
        VkDescriptorPoolCreateInfo info{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
        info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        info.maxSets = maxSets;
        info.poolSizeCount = static_cast<uint32_t>(poolSizes_.size());
        info.pPoolSizes = poolSizes_.data();

        VkDescriptorPool pool;
        VK_CHECK(vkCreateDescriptorPool(device_, &info, nullptr, &pool));
        return pool;
    }

    VkDescriptorPool DescriptorAllocator::grabPool() {
        if (!freePools_.empty()) {
            VkDescriptorPool pool = freePools_.back();
            freePools_.pop_back();
            return pool;
        }

        // Default allocation size
        constexpr uint32_t DEFAULT_SET_COUNT = 128;
        return createPool(DEFAULT_SET_COUNT);
    }

    VkResult DescriptorAllocator::allocate(VkDescriptorSetLayout layout, VkDescriptorSet& outSet) {
        if (currentPool_ == VK_NULL_HANDLE) {
            currentPool_ = grabPool();
            usedPools_.push_back(currentPool_);
        }

        VkDescriptorSetAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
        allocInfo.descriptorPool = currentPool_;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &layout;

        VkResult result = vkAllocateDescriptorSets(device_, &allocInfo, &outSet);
        if (result == VK_SUCCESS) return VK_SUCCESS;

        if (result == VK_ERROR_FRAGMENTED_POOL || result == VK_ERROR_OUT_OF_POOL_MEMORY) {
            // Try again with a new pool
            currentPool_ = grabPool();
            usedPools_.push_back(currentPool_);

            allocInfo.descriptorPool = currentPool_;
            return vkAllocateDescriptorSets(device_, &allocInfo, &outSet);
        }

        return result; // actual failure
    }

    void DescriptorAllocator::reset() {
        for (VkDescriptorPool pool : usedPools_) {
            vkResetDescriptorPool(device_, pool, 0);
            freePools_.push_back(pool);
        }
        usedPools_.clear();
        currentPool_ = VK_NULL_HANDLE;
    }

    void DescriptorAllocator::destroyAll() {
        for (VkDescriptorPool pool : usedPools_) {
            vkDestroyDescriptorPool(device_, pool, nullptr);
        }
        for (VkDescriptorPool pool : freePools_) {
            vkDestroyDescriptorPool(device_, pool, nullptr);
        }

        usedPools_.clear();
        freePools_.clear();
        currentPool_ = VK_NULL_HANDLE;
    }

} // namespace mve
