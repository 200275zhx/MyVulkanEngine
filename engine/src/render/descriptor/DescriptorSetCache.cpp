#include "render/descriptor/DescriptorSetCache.hpp"

namespace mve {

    DescriptorSetCache::DescriptorSetCache(VkDevice device)
        : device_(device) {
    }

    DescriptorSetCache::~DescriptorSetCache() {
        // NOTE: we don't own the VkDescriptorSets or pools
        cache_.clear();
    }

    void DescriptorSetCache::store(const DescriptorKey& key, VkDescriptorSet set) {
        cache_[key] = set;
    }

    VkDescriptorSet DescriptorSetCache::retrieve(const DescriptorKey& key) const {
        auto it = cache_.find(key);
        if (it != cache_.end())
            return it->second;
        return VK_NULL_HANDLE;
    }

    bool DescriptorSetCache::DescriptorKey::operator==(const DescriptorKey& other) const {
        if (buffers.size() != other.buffers.size() || images.size() != other.images.size())
            return false;

        for (size_t i = 0; i < buffers.size(); ++i) {
            const auto& a = buffers[i];
            const auto& b = other.buffers[i];
            if (a.buffer != b.buffer || a.offset != b.offset || a.range != b.range)
                return false;
        }

        for (size_t i = 0; i < images.size(); ++i) {
            const auto& a = images[i];
            const auto& b = other.images[i];
            if (a.imageView != b.imageView || a.imageLayout != b.imageLayout || a.sampler != b.sampler)
                return false;
        }

        return true;
    }

    size_t DescriptorSetCache::DescriptorKey::hash() const {
        size_t h = 0;

        for (const auto& b : buffers) {
            h ^= std::hash<VkBuffer>()(b.buffer);
            h ^= std::hash<VkDeviceSize>()(b.offset) << 1;
            h ^= std::hash<VkDeviceSize>()(b.range) << 2;
        }

        for (const auto& i : images) {
            h ^= std::hash<VkImageView>()(i.imageView);
            h ^= std::hash<VkImageLayout>()(i.imageLayout) << 1;
            h ^= std::hash<VkSampler>()(i.sampler) << 2;
        }

        return h;
    }

} // namespace mve
