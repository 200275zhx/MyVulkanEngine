#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <unordered_map>

namespace mve {

    class DescriptorSetCache {
    public:
        explicit DescriptorSetCache(VkDevice device);
        ~DescriptorSetCache();

        DescriptorSetCache(const DescriptorSetCache&) = delete;
        DescriptorSetCache& operator=(const DescriptorSetCache&) = delete;

        struct DescriptorKey {
            std::vector<VkDescriptorBufferInfo> buffers;
            std::vector<VkDescriptorImageInfo> images;

            bool operator==(const DescriptorKey& other) const;
            size_t hash() const;
        };

        void store(const DescriptorKey& key, VkDescriptorSet set);
        VkDescriptorSet retrieve(const DescriptorKey& key) const;

    private:
        struct DescriptorKeyHasher {
            size_t operator()(const DescriptorKey& key) const {
                return key.hash();
            }
        };

        VkDevice device_;
        std::unordered_map<DescriptorKey, VkDescriptorSet, DescriptorKeyHasher> cache_;
    };

} // namespace mve
